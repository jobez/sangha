#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include "api.h"

struct shader_t {
  GLenum type;
  std::string filename;
  GLuint gl_id;
  ino_t file_id;
  time_t last_mod;
};

struct shader_manager {
  std::vector<shader_t> shaders;
  GLuint shader_programme;
};

struct state_t {
  GLFWwindow* window;
  GLuint vao;
  GLuint vbo;
  struct shader_manager shader_m;
};

struct state_t * s = NULL;

float points[] = {
   0.0f,  0.5f,  0.0f,
   0.5f, -0.5f,  0.0f,
  -0.5f, -0.5f,  0.0f
};

std::string get_file_contents(const char *filename)
{
  std::FILE *fp = std::fopen(filename, "rb");
  if (fp)
  {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return(contents);
  }
  throw(errno);
}

const char* vertex_shader =
  "#version 420\n"
"in vec3 vp;"
"void main() {"
"  gl_Position = vec4(vp, 1.0);"
"}";

/* const char* fragment_shader = get_file_contents("./thing.frag").c_str(); */

const char* fragment_shader =
"#version 420\n"
"out vec4 frag_colour;"
"void main() {"
"  frag_colour = vec4(0.15, 0.75, 0.15, 1.0);"
"}";

GLFWwindow* init_window() {

  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
  }

  return glfwCreateWindow(800, 800, "sangha", NULL, NULL);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void * AppInit() {

  void * state = mmap(0, 256L * 1024L * 1024L * 1024L, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);

  s = (state_t*)state;
  printf("Init\n");

  return state;
}

void shaderBoilerPlate(struct shader_manager& shader_m) {
  if (shader_m.shader_programme) {

    glDeleteProgram(shader_m.shader_programme);

  }
  GLuint shader_programme = glCreateProgram();
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



    /* // compile fragment */
    /* GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); */
    /* glShaderSource(fs, 1, &fragment_shader, NULL); */
    /* glCompileShader(fs); */

    // attatch and link

    glAttachShader(shader_programme, s);



  }
  shader_m.shader_programme = shader_programme;
  glLinkProgram(shader_programme);
}

static void AppLoad(void * state) {
  s = (state_t*)state;
  s->window = init_window();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwSetKeyCallback(s->window, key_callback);

  glfwMakeContextCurrent(s->window);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString(GL_VERSION); // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"


  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);
  s->vbo = vbo;

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  s->vao = vao;

  // shader marker
  struct shader_t vert;
  vert.filename = "./thing.vert";
  vert.type = GL_VERTEX_SHADER;

  struct shader_t frag;
  frag.filename = "./thing.frag";
  frag.type = GL_FRAGMENT_SHADER;
  s->shader_m.shaders.clear();
  s->shader_m.shaders.push_back(frag);
  s->shader_m.shaders.push_back(vert);

  /* shaderBoilerPlate(s->shader_m); */

  printf("Reload\n");

}

int file_is_modified(struct stat& file_stat, const char *path, time_t oldMTime) {
  int err = stat(path, &file_stat);
  if (err != 0) {
    perror(" [file_is_modified] stat");
    exit(errno);
  }
  return file_stat.st_mtime > oldMTime;
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
  s = (state_t*)state;
  pollShaderFiles(s->shader_m);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(s->shader_m.shader_programme);
  glBindVertexArray(s->vao);
  // draw points 0-3 from the currently bound VAO with current in-use shader
  glDrawArrays(GL_TRIANGLES, 0, 3);
  // update other events like input handling
  glfwPollEvents();
  // put the stuff we've been drawing onto the display
  glfwSwapBuffers(s->window);

  return glfwWindowShouldClose(s->window);
}

static void AppUnload(void * state) {
  glfwTerminate();
  s = (state_t*)state;

  printf("Unload\n");
}

static void AppDeinit(void * state) {
  s = (state_t*)state;

  printf("Finalize\n");
  munmap(state, 256L * 1024L * 1024L * 1024L);
}

struct api_t APP_API = {
  .Init   = AppInit,
  .Load   = AppLoad,
  .Step   = AppStep,
  .Unload = AppUnload,
  .Deinit = AppDeinit
};
