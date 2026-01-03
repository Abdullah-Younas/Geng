// ================== Includes ==================
//lol
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <Windows.h>
using namespace std;

#include "shader_utils.h"
#include "shaders.h"
#include "Camera.h"
#include "Transformations.h"
#include "CallBacks.h"
#include "model_loader.h"
#include "Skybox.h"
#include "TextureImage.h"

// ================== Globals ==================
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float ScreenColor[4] = { 0.21f, 0.1f, 0.16f, 1.0f };

float DirLightDirection[3] = { 0.67f, -1.0f, 1.0f };
float DirLightSpec[3] = { 1.0f, 1.0f, 1.0f };
float DirLightAmbient[3] = { 1.0f, 1.0f, 1.0f };
float DirLightDiff[3] = { 1.0f, 1.0f, 1.0f };
float DirLightIntensity = 1.5f;

float PointLightSpec[3] = { 0.0f, 0.0f, 0.0f };
float PointLightDiff[3] = { 0.0f, 0.0f, 0.0f };

float SpotLightDiff[3] = { 0.0f, 0.0f, 0.0f };
float SpotLightSpec[3] = { 0.0f, 0.0f, 0.0f };
float SpotlightInnerCutoff = 8.0f;
float SpotlightOuterCutoff = 12.0f;

float FogIntensity = 1.04f;
float FogColor[3] = { 0.21f, 0.1f, 0.16f };

bool Jumping = false;
bool skyBoxOn = true;

float verticalVelocity = 0.0f;
const float gravity = -14.0f;
const float jumpForce = 7.0f;

Camera camera(glm::vec3(0.0f, 0.25f, 1.0f));
Transformations transformer;
Model TestLevel;

// ================== Input Handling ==================
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Gather all movement input
    bool forward = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    bool backward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    bool left = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    bool right = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);

    // Process all movement at once
    camera.ProcessKeyboard(forward, backward, left, right, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !Jumping)
    {
		verticalVelocity = jumpForce;
        Jumping = true;
    }
}

// ================== Main ==================
int main() {
    // GLFW Init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1980, 1080, "Lighting", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glfwSetCursorPosCallback(window, CallBacks::mouse_callback);
    glfwSetScrollCallback(window, CallBacks::scroll_callback);

    glm::vec3 pointLightPositions[] = {
        glm::vec3(100.0f, 100.0f, 100.0f),
        glm::vec3(100.0f, 100.0f, 100.0f),
        glm::vec3(100.0f, 100.0f, 100.0f),
        glm::vec3(100.0f, 100.0f, 100.0f)
    };

    if (!TestLevel.Load("TestLevel.obj")) {
        std::cerr << "Failed to load model" << std::endl;
        return -1;
    }

    // ================== Shaders ==================
    unsigned int lightingShader = createShaderProgram(vertexShaderSource, fragmentShaderSource1);
    unsigned int skyboxShader = createShaderProgram(CubeMapVShader, CubeMapFShader);

    std::vector<std::string> faces = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };
    Skybox skybox(faces, skyboxShader);

    glUseProgram(lightingShader);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ================== Main Render Loop ==================
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);


        // Clear Buffers
        glClearColor(ScreenColor[0], ScreenColor[1], ScreenColor[2], ScreenColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        verticalVelocity += gravity * deltaTime;
        camera.Position.y += verticalVelocity * deltaTime;
        camera.UpdateSpeed(5.5f);

        if (camera.Position.y <= 0.0f)
        {
            camera.Position.y = 0.0f;
            verticalVelocity = 0.0f;
            Jumping = false;
            camera.UpdateSpeed(6.5f);
        }
        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 1980.0f / 1080.0f, 0.01f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ========== Lighting Pass ==========
        glUseProgram(lightingShader);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, glm::value_ptr(camera.Position));

        glUniform1f(glGetUniformLocation(lightingShader, "FogIntensity"), FogIntensity);
        glUniform3f(glGetUniformLocation(lightingShader, "fogColor"), FogColor[0], FogColor[1], FogColor[2]);

        // Directional light
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.direction"), DirLightDirection[0], DirLightDirection[1], DirLightDirection[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.ambient"), DirLightAmbient[0], DirLightAmbient[1], DirLightAmbient[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.diffuse"), DirLightDiff[0], DirLightDiff[1], DirLightDiff[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.specular"), DirLightSpec[0], DirLightSpec[1], DirLightSpec[2]);
        glUniform1f(glGetUniformLocation(lightingShader, "dirIntensity"), DirLightIntensity);

        // Point lights
        for (int i = 0; i < 4; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(lightingShader, (base + ".position").c_str()), 1, &pointLightPositions[i][0]);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".ambient").c_str()), 1.0f, 1.0f, 1.0f);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".diffuse").c_str()), PointLightDiff[0], PointLightDiff[1], PointLightDiff[2]);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".specular").c_str()), PointLightSpec[0], PointLightSpec[1], PointLightSpec[2]);
            glUniform1f(glGetUniformLocation(lightingShader, (base + ".constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader, (base + ".linear").c_str()), 0.09f);
            glUniform1f(glGetUniformLocation(lightingShader, (base + ".quadratic").c_str()), 0.032f);
        }

        // Spotlight
        glUniform3fv(glGetUniformLocation(lightingShader, "spotLight.position"), 1, &camera.Position[0]);
        glUniform3fv(glGetUniformLocation(lightingShader, "spotLight.direction"), 1, &camera.Front[0]);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.diffuse"), SpotLightDiff[0], SpotLightDiff[1], SpotLightDiff[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.specular"), SpotLightSpec[0], SpotLightSpec[1], SpotLightSpec[2]);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.cutOff"), glm::cos(glm::radians(SpotlightInnerCutoff)));
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.outerCutOff"), glm::cos(glm::radians(SpotlightOuterCutoff)));

        glm::mat4 modelTestLevel = glm::mat4(1.0f);
        modelTestLevel = glm::translate(modelTestLevel, glm::vec3(0.0f, -1.5f, 0.0f));
        modelTestLevel = transformer.ScaleMeshComb(modelTestLevel, 2.0f);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(modelTestLevel));

        TestLevel.Render(lightingShader);
        glUseProgram(lightingShader);


        if (skyBoxOn) {
            skybox.Render(view, projection);
        }

        ImGui::Begin("Scene Controls");
        ImGui::Checkbox("Skybox?", &skyBoxOn);
        ImGui::ColorEdit4("Sky Color", ScreenColor);
        ImGui::Text("Directional Light");
        ImGui::SliderFloat3("Directional Light Direction", DirLightDirection, -1.0f, 1.0f);
        ImGui::ColorEdit3("Directional Light Ambient", DirLightAmbient);
        ImGui::ColorEdit3("Directional Light Specular", DirLightSpec);
        ImGui::ColorEdit3("Directional Light Diffuse", DirLightDiff);
        ImGui::SliderFloat("Dir Light Intensity", &DirLightIntensity, 0.1f, 50.0f);
        /*ImGui::Text("Point Light");
        ImGui::ColorEdit3("Point Light Specular", PointLightSpec);
        ImGui::ColorEdit3("Point Light Diffuse", PointLightDiff);
        ImGui::Text("Spot Light");
        ImGui::ColorEdit3("Spot Light Specular", SpotLightSpec);
        ImGui::ColorEdit3("Spot Light Diffuse", SpotLightDiff);
        ImGui::SliderFloat("Inner Cut Off", &SpotlightInnerCutoff, 3.0f, 20.0f);
        ImGui::SliderFloat("Outer Cut Off", &SpotlightOuterCutoff, 5.0f, 25.0f);
        ImGui::Text("Fog");
        ImGui::SliderFloat("Fog Intensity", &FogIntensity, 0.1f, 10.0f);
        ImGui::ColorEdit3("Fog Color", FogColor);*/
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // ========== End Frame ==========
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // ================== Cleanup ==================
    skybox.Cleanup();
    //TestLevel.Cleanup();
    glDeleteProgram(lightingShader);
    glDeleteProgram(skyboxShader);

    glfwTerminate();
    return 0;
}