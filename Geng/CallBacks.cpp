// CallBacks.cpp
#include <glad/glad.h>
#include "CallBacks.h"
#include "Camera.h"

extern Camera camera; // Make sure this camera is declared externally

float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

void CallBacks::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }

    float xoffset = float(xpos) - lastX;
    float yoffset = lastY - float(ypos);
    lastX = float(xpos);
    lastY = float(ypos);

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void CallBacks::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}
