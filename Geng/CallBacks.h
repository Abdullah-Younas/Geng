// CallBacks.h
#pragma once
#include <GLFW/glfw3.h>

class CallBacks {
public:
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};
