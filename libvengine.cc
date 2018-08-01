#include <ctime>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <string>
#include <stdio.h>
#include <vector>
#include "record.h"
/* #include "save_png.h" */
#include <stdlib.h>
#include <glib.h>
#include "utils.h"
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsink.h>
#include "settings.h"
#include "api.h"
#include "libvengine.h"
#include <thread>

/* ----------------------------------------------------------------------- */


struct v_state_t * s = NULL;

GLfloat points[] = {
  -1.0f, -1.0f,
  -1.0f, 1.0f,
  1.0f, -1.0f,
  1.0f, 1.0f
};


void fftToGL(v_state_t* vs, a_state_t* as) {

  as->audio_engine->fft->syncFFTExec();

//   // Create one OpenGL texture

// "Bind" the newly created texture : all future texture functions will modify this texture

  // todo: softcode buffersize (512 arg)
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, vs->fftTexId);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, 512,
               0, GL_RED, GL_FLOAT,
               as->audio_engine->fft->fftBuffer);


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}

// constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int CHANNEL_COUNT = 4;
const int DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT = GL_BGRA;
const int PBO_COUNT = 2;

GLuint pboIds[PBO_COUNT];

// TODO: system wide mechanism for (who) engines to register the 'what' of their signals and have that pattern matched with the 'where' GPU placement logic
uniform_table_t register_uniform_srcs(uniform_table_t uniformTable) {

  uniformTable["fft"]=  fftToGL;
  uniformTable["iTime"]=  [](v_state_t* vs, a_state_t* as){
      glUniform1f(glGetUniformLocation(vs->shader_m.shader_program, "iTime"), glfwGetTime());
  };
  uniformTable["iResolution"]= [](v_state_t* vs, a_state_t* as) {
      glUniform2f(glGetUniformLocation(vs->shader_m.shader_program, "iResolution"), (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

  };

  return uniformTable;

}

maybe_sync_fn uniform_name_to_sync_fn (uniform_table_t uniform_table, std::string uniform_name) {

  if (auto sync_fn = uniform_table[uniform_name]) {
    return maybe_sync_fn{sync_fn};
  } else {
    return std::nullopt;
  }

}

std::vector<uniform_t> reflect_uniforms(GLuint program) {


  std::vector<uniform_t> unis;
  GLint numActiveUniforms = 0;

  glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numActiveUniforms);

  std::vector<GLchar> nameData(256);
  std::vector<GLenum> properties;

  properties.push_back(GL_NAME_LENGTH);
  properties.push_back(GL_TYPE);

  std::vector<GLint> values(properties.size());
  for(int attrib = 0; attrib < numActiveUniforms; ++attrib)
{
  glGetProgramResourceiv(program, GL_UNIFORM, attrib, properties.size(),
                         &properties[0], values.size(), NULL, &values[0]);

  nameData.resize(values[0]); //The length of the name.
  glGetProgramResourceName(program, GL_UNIFORM, attrib, nameData.size(), NULL, &nameData[0]);
  std::string name((char*)&nameData[0], nameData.size() - 1);
  uniform_t uni(name, values[1]);

  unis.push_back(uni);
 }


  return unis;

}

GLFWwindow* init_window() {

  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
  }

  return glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "sangha", NULL, NULL);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

static void * AppInit() {

  void * state = mmap(0, 256L * 1024L * 1024L * 1024L,
                      PROT_READ |
                      PROT_WRITE,
                      MAP_ANONYMOUS |
                      MAP_PRIVATE |
                      MAP_NORESERVE, -1, 0);

  s = (v_state_t*)state;

  // s->gstr = sangha_vsrc(gstr_pipeline_expr);
  std::cout << "hello" << std::endl;
  printf("Init\n");

  return state;
}

void checkErrors(const char *desc) {
	GLenum e = glGetError();
	if (e != GL_NO_ERROR) {
		printf("OpenGL error in \"%s\": %s (%d)\n", desc, gluErrorString(e), e);
		exit(1);
	}
}
// TODO: refactor/rename
void shaderBoilerPlate(struct shader_manager& shader_m) {
  if (shader_m.shader_program) {

    glDeleteProgram(shader_m.shader_program);

  }
  GLuint shader_program = glCreateProgram();
  for (int i = 0; i < shader_m.shaders.size(); i++) {
    struct shader_t& shader = shader_m.shaders[i];
    std::string src_contents = get_file_contents(shader.filename.c_str());

    std::cout << src_contents << std::endl;

    const char *src = src_contents.c_str();
    // compile vertex
    GLuint s = glCreateShader(shader.type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    //Check shader for errors
    GLint shaderCompiled = GL_FALSE;

    glGetShaderiv(s, GL_COMPILE_STATUS, &shaderCompiled );
    if(shaderCompiled == GL_FALSE)
      {

        std::cout << "compilation failed" << std::endl;
        GLint maxLength = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(s, maxLength, &maxLength, &errorLog[0]);
        std::string str(errorLog.begin(),errorLog.end());

        std::cout << str << std::endl;

        /* printf("%s\n",errorLog); */
        // Provide the infolog in whatever manor you deem best.
        // Exit with failure.
        glDeleteShader(s); // Don't leak the shader.

      }

    glAttachShader(shader_program, s);



  }

  shader_m.shader_program = shader_program;
  glLinkProgram(shader_program);
  shader_m.uniforms = reflect_uniforms(shader_m.shader_program);

}


void hydrate_uniforms(shader_manager shader_m, v_state_t* vs, a_state_t* as) {
  for(auto uniform : shader_m.uniforms) {
    if (maybe_sync_fn boxed_fn = uniform_name_to_sync_fn(shader_m.uniformTable, uniform.name)) {
      auto fn = *boxed_fn;
      fn(vs, as);
    } else {

      std::cout << "no sync fn for " << uniform.name << std::endl;

    }
  }
}

static void AppLoad(void * state) {
  s = (v_state_t*)state;
  s->window = init_window();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwSetKeyCallback(s->window, key_callback);

  glfwMakeContextCurrent(s->window);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;



  const GLenum err = glewInit();

  if (GLEW_OK != err)
    {
      std::cout << "GLEW Error: " << glewGetErrorString(err) << std::endl;
    }
  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString(GL_VERSION); // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);



  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), points, GL_STATIC_DRAW);
  s->vbo = vbo;

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  s->vao = vao;

  GLuint fftTexture;

  glBindTexture(GL_TEXTURE_1D, fftTexture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  s->fftTexId = fftTexture;



  // shader marker
  shader_t vert("./thing.vert", GL_VERTEX_SHADER);
  shader_t frag("./jhnn.frag", GL_FRAGMENT_SHADER);


  s->shader_m.shaders.clear();
  s->shader_m.shaders.push_back(frag);
  s->shader_m.shaders.push_back(vert);
  s->shader_m.uniformTable = register_uniform_srcs(s->shader_m.uniformTable);

  glGenBuffersARB(PBO_COUNT, pboIds);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
  glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
  glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
  /* shaderBoilerPlate(s->shader_m); */

  printf("Reload\n");

}

void pollShaderFiles(struct shader_manager& shader_m) {
  /* printf("%i", shader_m.shaders.size()); */
  for (int i = 0; i < shader_m.shaders.size(); i++) {
    struct stat attr;
    struct shader_t& shader = shader_m.shaders[i];
    if (file_is_modified(attr, shader.filename.c_str(), shader.last_mod)) {

      // orient shader wrt time
      shader.file_id = attr.st_ino;
      shader.last_mod = attr.st_mtime;
      shaderBoilerPlate(shader_m);
      //0

    }
  }
}

static int AppStep(void * state) {
  s = (v_state_t*)state;
  pollShaderFiles(s->shader_m);
  static int shift = 0;
  static int index = 0;
  int nextIndex = 0;

  index = (index + 1) % PBO_COUNT;
  nextIndex = (index + 1) % PBO_COUNT;


  if (s->should_record) {
  glReadBuffer(GL_FRONT);

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[index]);
  glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[nextIndex]);
  s->capture = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

  if (s->capture) {
    gstr_step(s->gstr.gloop);
    hydrate_appsrc(s->gstr.vsrc, s->capture);

    /* save_png("./test.png", SCREEN_WIDTH, SCREEN_HEIGHT, 8, PNG_COLOR_TYPE_RGBA, s->capture, 4 * SCREEN_WIDTH, PNG_TRANSFORM_BGR); */
    glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);

  }

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
  }
  glDrawBuffer(GL_BACK);


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(s->shader_m.shader_program);
  glBindVertexArray(s->vao);
  // draw points 0-3 from the currently bound VAO with current in-use shader
  glDrawArrays(GL_TRIANGLES, 0, 3);
  // update other events like input handling
  glfwPollEvents();
  // put the stuff we've been drawing onto the display
  glfwSwapBuffers(s->window);

  return glfwWindowShouldClose(s->window);
}




static int AppStep2(void * state, void * state2) {
  s = (v_state_t*)state;
  pollShaderFiles(s->shader_m);
  static int shift = 0;
  static int index = 0;
  int nextIndex = 0;
  a_state_t* audio_state;


  index = (index + 1) % PBO_COUNT;
  nextIndex = (index + 1) % PBO_COUNT;

  audio_state = (a_state_t*)state2;

  hydrate_uniforms(s->shader_m, s, audio_state);








  // GLint resolutionLoc = glGetUniformLocation(s->shader_m.shader_programme, "resolution");
  // if (resolutionLoc != -1) glUniform2f(resolutionLoc, (float)r->win->width, (float)r->win->height);

  if (s->should_record) {
  glReadBuffer(GL_FRONT);

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[index]);
  glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);



  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[nextIndex]);
  s->capture = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

  if (s->capture) {
    gstr_step(s->gstr.gloop);
    hydrate_appsrc(s->gstr.vsrc, s->capture);

    /* save_png(/"./test.png", SCREEN_WIDTH, SCREEN_HEIGHT, 8, PNG_COLOR_TYPE_RG0BA, s->capture, 4 * SCREEN_WIDTH, PNG_TRANSFORM_BGR); */
    glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);

  }

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
  }
  glDrawBuffer(GL_BACK);


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(s->shader_m.shader_program);
  glBindVertexArray(s->vao);
  // draw points 0-3 from the currently bound VAO with current in-use shader

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


  // update other events like input handling
  glfwPollEvents();
  // put the stuff we've been drawing onto the display
  glfwSwapBuffers(s->window);

  return glfwWindowShouldClose(s->window);
}

static void AppUnload(void * state) {
  glfwTerminate();
  if (s->should_record) {
  sangha_stop_pipeline(s->gstr);
  sangha_close_pipeline(s->gstr);
  }
  s = (v_state_t*)state;

  printf("Unload\n");
}

static void AppDeinit(void * state) {
  s = (v_state_t*)state;

  printf("Finalize\n");
  munmap(state, 256L * 1024L * 1024L * 1024L);
}

struct api_t APP_API = {
  .Init   = AppInit,
  .Load   = AppLoad,
  .Step   = AppStep,
  .Step2 =  AppStep2,
  .Unload = AppUnload,
  .Deinit = AppDeinit
};
