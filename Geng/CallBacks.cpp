// CallBacks.cpp
#include <glad/glad.h>
#include "CallBacks.h"
#include "Camera.h"
#include "Player.h"
extern Player player; // Make sure this player is declared externally
extern Camera camera; // Make sure this camera is declared externally

float lastX = 1920.0f / 2.0f;
float lastY = 1080.0f / 2.0f;
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

    player.camera.ProcessMouseMovement(xoffset, yoffset);
}

void CallBacks::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    player.camera.ProcessMouseScroll((float)yoffset);
}
