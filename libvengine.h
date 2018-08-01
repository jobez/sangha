#pragma once
#include <fftw3.h>

struct shader_t {
  std::string filename;
  GLenum type;
  GLuint gl_id;
  ino_t file_id;
  time_t last_mod = 0;
  shader_t(std::string filename, GLenum type) :
  filename(filename),
    type(type)
  {
  }
};

struct shader_manager {
  std::vector<shader_t> shaders;
  GLuint shader_program;
};

struct v_state_t {
  GLFWwindow* window;
  GLuint vao;
  GLuint vbo;
  GLuint fftTexId;
  struct shader_manager shader_m;
  GLubyte* capture;
  bool should_record = false;
  Vstr_t gstr;
  float fftBuffer[512];
};
