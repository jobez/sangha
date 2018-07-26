#pragma once
#include <fftw3.h>

struct shader_t {
  GLenum type;
  std::string filename;
  GLuint gl_id;
  ino_t file_id;
  time_t last_mod = 0;
};

struct shader_manager {
  std::vector<shader_t> shaders;
  GLuint shader_programme;
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
