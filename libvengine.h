#pragma once

#include "libaengine.h"
#include <fftw3.h>
#include <functional>
#include <optional>
#include <map>

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

struct uniform_t {
  std::string name;
  GLint type;
uniform_t(std::string name, GLint type) :
  name(name),
    type(type)
  {}
};

struct v_state_t;

// TODO: assess performative cost
// https://www.bfilipek.com/2018/05/using-optional.html
typedef std::function<void(v_state_t* vstate,a_state_t* astate)> sync_fn;

typedef std::optional<sync_fn> maybe_sync_fn;

typedef std::map< std::string , sync_fn > uniform_table_t;


struct shader_manager {
  std::vector<shader_t> shaders;
  GLuint shader_program;
  std::vector<uniform_t> uniforms;
  uniform_table_t uniformTable;
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
