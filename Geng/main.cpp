// ================== Includes ==================
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

// ================== Globals ==================
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool InteractCursor = false;

float ScreenColor[4] = { 0.21f, 0.1f, 0.16f, 1.0f };

float DirLightSpec[3] = { 1.0f, 1.0f, 1.0f };
float DirLightDiff[3] = { 1.0f, 1.0f, 1.0f };
float DirLightIntensity = 0.899f;

float PointLightSpec[3] = { 0.0f, 0.0f, 0.0f };
float PointLightDiff[3] = { 0.0f, 0.0f, 0.0f };

float SpotLightDiff[3] = { 0.0f, 0.0f, 0.0f };
float SpotLightSpec[3] = { 0.0f, 0.0f, 0.0f };
float SpotlightInnerCutoff = 8.0f;
float SpotlightOuterCutoff = 12.0f;

float FogIntensity = 1.04f;
float FogColor[3] = { 0.21f, 0.1f, 0.16f };

bool skyBoxOn = false;

Camera camera(glm::vec3(0.0f, 0.0f, 2.0f));
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
Transformations transformer;
Model carModel;


// ================== Input Handling ==================
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_FALSE);
    glDepthFunc(GL_LESS);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glfwSetCursorPosCallback(window, CallBacks::mouse_callback);
    glfwSetScrollCallback(window, CallBacks::scroll_callback);


    glm::vec3 pointLightPositions[] = {
        glm::vec3(100.0f,  100.0f,  100.0f),
        glm::vec3(100.0f, 100.0f, 100.0f),
        glm::vec3(100.0f,  100.0f, 100.0f),
        glm::vec3(100.0f,  100.0f, 100.0f)
    };

    if (!carModel.Load("RenderVehicle.obj")) {
        std::cerr << "Failed to load model" << std::endl;
        return -1;
    }

    for (const auto& pos : pointLightPositions) {
        std::cout << "Light position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
    }

    // ================== Shaders ==================
    unsigned int lightingShader = createShaderProgram(vertexShaderSource, fragmentShaderSource1);
    unsigned int lampShader = createShaderProgram(vertexShaderSource, lampFragmentShaderSource);
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

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 1980.0f / 1080.0f, 0.01f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ========== Lighting Pass ==========
        glUseProgram(lightingShader);
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));  // move up/down
        model = transformer.RotMeshY(model, 180.0f);
        model = transformer.ScaleMeshComb(model, 0.15f);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform1f(glGetUniformLocation(lightingShader, "FogIntensity"), FogIntensity);
        glUniform3f(glGetUniformLocation(lightingShader, "fogColor"), FogColor[0], FogColor[1], FogColor[2]);


        // Directional light
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
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

        // Spotlight (flashlight)
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

        // Render Cube
        carModel.Render(lightingShader);

        if (skyBoxOn) {
            skybox.Render(view, projection);
        }


        ImGui::Begin("Hehe, me is window");
        ImGui::Checkbox("Skybox?", &skyBoxOn);
        ImGui::ColorEdit4("Sky Color", ScreenColor);
        ImGui::Text("Directional Light");
        ImGui::ColorEdit3("Directional Light Specular", DirLightSpec);
        ImGui::ColorEdit3("Directional Light Diffuse", DirLightDiff);
        ImGui::SliderFloat("Dir Light Intensity", &DirLightIntensity, 0.1f, 50.0f);
        ImGui::Text("Point Light");
        ImGui::ColorEdit3("Point Light Specular", PointLightSpec);
        ImGui::ColorEdit3("Point Light Diffuse", PointLightDiff);
        ImGui::Text("Spot Light");
        ImGui::ColorEdit3("Spot Light Specular", SpotLightSpec);
        ImGui::ColorEdit3("Spot Light Diffuse", SpotLightDiff);
        ImGui::SliderFloat("Inner Cut Off", &SpotlightInnerCutoff, 3.0f, 20.0f);
        ImGui::SliderFloat("Inner Outer Off", &SpotlightOuterCutoff, 5.0f, 25.0f);
        ImGui::Text("Fog");
        ImGui::SliderFloat("Fog Intensity", &FogIntensity, 0.1f, 5.0f);
        ImGui::ColorEdit3("Fog Color", FogColor);
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
    skybox.Cleanup();  // or remove if relying on destructor
    carModel.Cleanup();
    glDeleteProgram(lightingShader);
    glDeleteProgram(lampShader);

    glfwTerminate();
    return 0;
}
