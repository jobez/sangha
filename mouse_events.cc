// camera logic https://github.com/patriciogonzalezvivo/glslViewer/blob/master/src/sandbox.cpp
#include "mouse_events.h"

void onMouseClick(float _x, float _y, int _button) {

}

void onScroll(camera_state& cs, double yoffset) {
  auto m_view2d = &cs.m_view2d;
  auto m_eye3d = &cs.m_eye3d;
  auto m_centre3d = &cs.m_centre3d;
  constexpr float zoomfactor = 1.1892;
  if (yoffset != 0) {
    float z = pow(zoomfactor, yoffset);

    // zoom view2d
    glm::vec2 zoom = glm::vec2(z,z);
    glm::vec2 origin = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    *m_view2d = glm::translate(*m_view2d, origin);
    *m_view2d = glm::scale(*m_view2d, zoom);
    *m_view2d = glm::translate(*m_view2d, -origin);

    // zoom view3d
    *m_eye3d = *m_centre3d + (*m_eye3d - *m_centre3d)*z;
  }
}

void onMouseDrag(mouse_state& ms, camera_state& cs, float _x, float _y, int _button) {
  auto m_vel_y = ms.velY;
  auto m_vel_x = ms.velX;
  auto m_vel = glm::vec2(m_vel_x, m_vel_y);
  auto m_cam = cs.m_cam;
  auto m_view2d = &cs.m_view2d;
  auto m_centre3d = &cs.m_centre3d;
  auto m_eye3d = &cs.m_eye3d;
  auto m_up3d = &cs.m_up3d;
  auto m_lat = &cs.m_lat;
  auto m_lon = &cs.m_lon;
  if (_button == 1){
    // Left-button drag is used to rotate geometry.
    float dist = glm::length(m_cam.getPosition());
    *m_lat -= m_vel_y;
    *m_lon -= m_vel_y*0.5;
    m_cam.orbit(*m_lat,*m_lon, dist);
    m_cam.lookAt(glm::vec3(0.0));

    // Left-button drag is used to pan u_view2d.
    *m_view2d = glm::translate(*m_view2d, -m_vel);

    // Left-button drag is used to rotate eye3d around centre3d.
    // One complete drag across the screen width equals 360 degrees.
    constexpr double tau = 6.283185307179586;
    *m_eye3d -= *m_centre3d;
    *m_up3d -= *m_centre3d;

    // Rotate about vertical axis, defined by the 'up' vector.
    float xangle = (m_vel_x / SCREEN_WIDTH) * tau;
    *m_eye3d = glm::rotate(*m_eye3d, -xangle, *m_up3d);
    // Rotate about horizontal axis, which is perpendicular to
    // the (centre3d,eye3d,up3d) plane.
    float yangle = (m_vel_y / SCREEN_HEIGHT) * tau;
    glm::vec3 haxis = glm::cross(*m_eye3d - *m_centre3d, *m_up3d);
    *m_eye3d = glm::rotate(*m_eye3d, -yangle, haxis);
    *m_up3d = glm::rotate(*m_up3d, -yangle, haxis);
    //
    *m_eye3d += *m_centre3d;
    *m_up3d += *m_centre3d;
  }
  else {
    // Right-button drag is used to zoom geometry.
    float dist = glm::length(m_cam.getPosition());
    dist += (-.008f * m_vel_y);
    if(dist > 0.0f){
      m_cam.setPosition( -dist * m_cam.getZAxis() );
      m_cam.lookAt(glm::vec3(0.0));
    }

    // TODO: rotate view2d.

    // pan view3d.
    float dist3d = glm::length(*m_eye3d - *m_centre3d);
    glm::vec3 voff = glm::normalize(*m_up3d)
      * (m_vel_y/SCREEN_HEIGHT) * dist3d;
    *m_centre3d -= voff;
    *m_eye3d -= voff;
    glm::vec3 haxis = glm::cross(*m_eye3d - *m_centre3d, *m_up3d);
    glm::vec3 hoff = glm::normalize(haxis)
      * (m_vel_x/SCREEN_WIDTH) * dist3d;
    *m_centre3d += hoff;
    *m_eye3d += hoff;
  }

}

void onMouseMove(float _x, float _y)  {}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}
