#pragma once

#include "libaengine.h"
#include "record.h"
#include <GLFW/glfw3.h>
#include <fftw3.h>
#include <functional>
#include <optional>
#include <map>
#include "camera.h"
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_impl_glfw.h"
#include "ext/imgui/imgui_impl_opengl3.h"

// constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const glm::ivec4 viewport(0., 0., SCREEN_WIDTH, SCREEN_HEIGHT);
const int CHANNEL_COUNT = 4;
const int DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT = GL_BGRA;
const int PBO_COUNT = 2;



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
typedef void (*sync_fn)(v_state_t* vstate,a_state_t* astate);

typedef std::optional<sync_fn> maybe_sync_fn;

typedef std::map< std::string , sync_fn > uniform_table_t;


typedef struct {
  std::vector<shader_t> shaders;
  GLuint shader_program;
  std::vector<uniform_t> uniforms;
  uniform_table_t uniformTable;
} shader_manager;

// https://github.com/patriciogonzalezvivo/glslViewer/blob/29d4632624e0525f64fda15d95d3577d62aa3596/src/window.cpp#L14
typedef struct {
  bool      left_mouse_button_down;
  float     x,y;
  int       button;
  float     velX,velY;
  glm::vec2 drag;
} mouse_state;


typedef struct {
  float m_lat = 180.0;
  float m_lon = 80.0;
  glm::mat3 m_view2d;
  // These are the 'view3d' uniforms.
  // Note: the up3d vector must be orthogonal to (eye3d - centre3d),
  // or rotation doesn't work correctly.
  glm::vec3 m_centre3d;

  // The following initial value for 'eye3d' is derived by starting with [0,0,6],
  // then rotating 30 degrees around the X and Y axes.
  glm::vec3 m_eye3d;

  // The initial value for up3d is derived by starting with [0,1,0], then
  // applying the same rotations as above, so that up3d is orthogonal to eye3d.
  glm::vec3 m_up3d ;
  Camera m_cam;
} camera_state;

struct v_state_t {
  GLFWwindow* window;
  GLuint vao;
  GLuint vbo;
  GLuint fftTexId;
  shader_manager shader_m;
  GLubyte* capture;
  bool should_record = false;
  Vstr_t gstr;
  float fftBuffer[512];
  float pixelDensity = 1.0;
  glm::vec4 iMouse;
  camera_state cam_s;
  mouse_state mouse_s;
  ImGuiIO& io;
};
