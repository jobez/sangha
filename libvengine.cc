#include <ctime>
#include <GL/glew.h>
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
#include "mouse_events.h"
/* ----------------------------------------------------------------------- */



GLuint pboIds[PBO_COUNT];

v_state_t * s = NULL;

void imgui_view(v_state_t* vs, a_state_t* as) {


    // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  static float f = 0.0f;
  static int counter = 0;


  static double tempo = 120.;
  static double quantum = 4.;
  auto beat_time = as->audio_engine->beatTime();
  ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
  ImGui::InputDouble("beat time", &beat_time, 0.01f, 1.0f, "%.8f");
  ImGui::InputDouble("tempo", &tempo, 0.01f, 1.0f, "%.8f");
  ImGui::InputDouble("quantum", &quantum, 1.0f, 8.0f, "%.8f");
  // ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
  // ImGui::SliderFloat3("Eye3d uniform", (float*)&vs->cam_s.m_eye3d, 0.0f, 10.0f);


  if (ImGui::Button("start playing"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    as->audio_engine->startPlaying();

  if (ImGui::Button("Set Tempo"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    as->audio_engine->setTempo(tempo);

  if (ImGui::Button("Set Quantum"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    as->audio_engine->setQuantum(quantum);

  if (ImGui::Button("Should Close")) {
      glfwSetWindowShouldClose(vs->window, GL_TRUE);
    }

  ImGui::SameLine();
  ImGui::Text("tempo = %d", tempo);

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();


}


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

// TODO: system wide mechanism for (who) engines to register the 'what' of their signals and have that pattern matched with the 'where' GPU placement logic
uniform_table_t register_uniform_srcs() {
  uniform_table_t uniformTable;

  uniformTable["fft"]=  fftToGL;

  uniformTable["iTime"]=  [](v_state_t* vs, a_state_t* as){
    auto beat_time = as->audio_engine->beatTime();

    glUniform1f(glGetUniformLocation(vs->shader_m.shader_program, "iTime"), beat_time);
  };


  uniformTable["beatPerBar"]=  [](v_state_t* vs, a_state_t* as){
    auto beat_time = as->audio_engine->quantum();

    glUniform1i(glGetUniformLocation(vs->shader_m.shader_program, "beatPerBar"), beat_time);
  };

  uniformTable["iResolution"]= [](v_state_t* vs, a_state_t* as) {
    glUniform2f(glGetUniformLocation(vs->shader_m.shader_program, "iResolution"), (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

  };

  // TODO: performance dimensionality of using auto
  uniformTable["iMouse"] = [](v_state_t* vs, a_state_t* as) {
    auto _x = vs->iMouse.x;
    auto _y = vs->iMouse.y;
    auto _z = vs->iMouse.z;
    auto _w = vs->iMouse.w;

    glUniform4f(glGetUniformLocation(vs->shader_m.shader_program, "iMouse"), _x, _y, _z, _w);
  };

  uniformTable["u_eye3d"] = [](v_state_t* vs, a_state_t* as) {
    auto _x = vs->cam_s.m_eye3d.x;
    auto _y = vs->cam_s.m_eye3d.y;
    auto _z = vs->cam_s.m_eye3d.z;


    glUniform3f(glGetUniformLocation(vs->shader_m.shader_program, "u_eye3d"), _x, _y, _z);
  };

  uniformTable["u_centre3d"] = [](v_state_t* vs, a_state_t* as) {
    auto _x = vs->cam_s.m_centre3d.x;
    auto _y = vs->cam_s.m_centre3d.y;
    auto _z = vs->cam_s.m_centre3d.z;


    glUniform3f(glGetUniformLocation(vs->shader_m.shader_program, "u_centre3d"), _x, _y, _z);
  };

 uniformTable["u_up3d"] = [](v_state_t* vs, a_state_t* as) {
    auto _x = vs->cam_s.m_up3d.x;
    auto _y = vs->cam_s.m_up3d.y;
    auto _z = vs->cam_s.m_up3d.z;


    glUniform3f(glGetUniformLocation(vs->shader_m.shader_program, "u_up3d"), _x, _y, _z);
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
void shaderBoilerPlate(shader_manager& shader_m) {
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
  const char* glsl_version = "#version 420";




  glfwSetScrollCallback(s->window, [&s](GLFWwindow* _window, double xoffset, double yoffset) {
      auto fPixelDensity = 1;
      onScroll(s->cam_s, -yoffset * fPixelDensity);
    });

  glfwSetMouseButtonCallback(s->window, [&s](GLFWwindow* window, int button, int action, int mods){
      auto left_mouse_button_down = s->mouse_s.left_mouse_button_down;
      auto iMouse = &s->iMouse;
      auto mouse = &s->mouse_s;

      if (button == GLFW_MOUSE_BUTTON_1) {
        // update iMouse when left mouse button is pressed or released
        if (action == GLFW_PRESS && !left_mouse_button_down) {
          left_mouse_button_down = true;
          iMouse->x = mouse->x;
          iMouse->y = mouse->y;
          iMouse->z = mouse->x;
          iMouse->w = mouse->y;

        } else if (action == GLFW_RELEASE && left_mouse_button_down) {
          left_mouse_button_down = false;
          iMouse->z = -iMouse->z;
          iMouse->w = -iMouse->w;
        }
      }
      if (action == GLFW_PRESS) {
        mouse->drag.x = mouse->x;
        mouse->drag.y = mouse->y;
      }
    });

  glfwSetCursorPosCallback(s->window, [&s](GLFWwindow* _window, double x, double y) {
      auto left_mouse_button_down = &s->mouse_s.left_mouse_button_down;
      auto fPixelDensity = 1;
      auto iMouse = &s->iMouse;
      auto mouse = &s->mouse_s;

      // Convert x,y to pixel coordinates relative to viewport.
      // (0,0) is lower left corner.
      y = viewport.w - y;
      x *= fPixelDensity;
      y *= fPixelDensity;
      // mouse.velX,mouse.velY is the distance the mouse cursor has moved
      // since the last callback, during a drag gesture.
      // mouse.drag is the previous mouse position, during a drag gesture.
      // Note that mouse.drag is *not* constrained to the viewport.
      mouse->velX = x - mouse->drag.x;
      mouse->velY = y - mouse->drag.y;
      mouse->drag.x = x;
      mouse->drag.y = y;

      // mouse.x,mouse.y is the current cursor position, constrained
      // to the viewport.
      mouse->x = x;
      mouse->y = y;


      if (mouse->x < 0) mouse->x = 0;
      if (mouse->y < 0) mouse->y = 0;
      if (mouse->x > viewport.z * fPixelDensity) mouse->x = viewport.z * fPixelDensity;
      if (mouse->y > viewport.w * fPixelDensity) mouse->y = viewport.w * fPixelDensity;

      // update iMouse when cursor moves
      if (left_mouse_button_down) {
        iMouse->x = mouse->x;
        iMouse->y = mouse->y;
      }

      /*
       * TODO: the following code would best be moved into the
       * mouse button callback. If you click the mouse button without
       * moving the mouse, then using this code, the mouse click doesn't
       * register until the cursor is moved. (@doug-moen)
       */
      int action1 = glfwGetMouseButton(s->window, GLFW_MOUSE_BUTTON_1);
      int action2 = glfwGetMouseButton(s->window, GLFW_MOUSE_BUTTON_2);
      int button = 0;

      if (action1 == GLFW_PRESS) button = 1;
      else if (action2 == GLFW_PRESS) button = 2;

      // Lunch events
      if (mouse->button == 0 && button != mouse->button) {
        mouse->button = button;
        onMouseClick(mouse->x,mouse->y,mouse->button);
      }
      else {
        mouse->button = button;
      }

      if (mouse->velX != 0.0 || mouse->velY != 0.0) {
        if (button != 0) onMouseDrag(s->mouse_s, s->cam_s, mouse->x,mouse->y,mouse->button);
        else onMouseMove(mouse->x,mouse->y);
      }
    });

  glfwMakeContextCurrent(s->window);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;

  // Init camera
  //
  s->cam_s.m_cam.setViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
  s->cam_s.m_cam.setPosition(glm::vec3(0.0,0.0,-3.));
  s->cam_s.m_up3d = glm::vec3(-0.25,0.866025,-0.433013);
  s->cam_s.m_eye3d = glm::vec3(2.598076,3.0,4.5);
  s->cam_s.m_centre3d = glm::vec3(0.,0.,0.);
  s->cam_s.m_view2d = glm::mat3(1.);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  // s->io = ImGui::GetIO();
  // (void)s->io;

  ImGui_ImplGlfw_InitForOpenGL(s->window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

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
  shader_t vert("./glsl/default.vert", GL_VERTEX_SHADER);
  shader_t frag("./glsl/lab/one.frag", GL_FRAGMENT_SHADER);


  s->shader_m.shaders.clear();
  s->shader_m.shaders.push_back(frag);
  s->shader_m.shaders.push_back(vert);

  s->shader_m.uniformTable = register_uniform_srcs();

  glGenBuffersARB(PBO_COUNT, pboIds);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[0]);
  glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);
  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pboIds[1]);
  glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, DATA_SIZE, 0, GL_STREAM_READ_ARB);

  glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
  /* shaderBoilerPlate(s->shader_m); */

  printf("Reloaad\n");

}

void pollShaderFiles(shader_manager& shader_m) {
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

static int AppStep(void * state) {}




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

  imgui_view(s, audio_state);

  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
