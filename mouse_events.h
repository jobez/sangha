#pragma once
#include "libvengine.h"

void onScroll(camera_state& cs, double yoffset);

void onMouseClick(float _x, float _y, int _button);

void onMouseDrag(mouse_state& ms, camera_state& cs, float _x, float _y, int _button);

void onMouseMove(float _x, float _y);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
