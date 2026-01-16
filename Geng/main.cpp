// Includes
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
#include "Player.h"

//Made this player global for the Callbacks
Player player(glm::vec3(0.0f, 0.25f, 1.0f));

#include "Transformations.h"
#include "CallBacks.h"
#include "model_loader.h"
#include "Skybox.h"
#include "TextureImage.h"
#include "Sphere.h"
#include "LevelCollision.h"

// Globals
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

float FramePerSecond = 0;

bool skyBoxOn = true;
bool showSphere = true;

//custom maths
Transformations transformer;

//Models
Model TestLevel;

//Collisions
LevelCollision levelCollision;

//Debugging the collisions box
bool showCollisionBoxes = false;
unsigned int debugShader = 0;

// Input Handling
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    bool forward = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    bool backward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    bool left = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    bool right = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);

    player.ProcessKeyboard(forward, backward, left, right, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        player.Jump();
    }
}

//Rendering the debug collision box
void RenderDebugBox(const BoundingBox& box, unsigned int shader, const glm::mat4& view, const glm::mat4& projection) {
    float vertices[] = {
        box.min.x, box.min.y, box.min.z,
        box.max.x, box.min.y, box.min.z,
        box.max.x, box.max.y, box.min.z,
        box.min.x, box.max.y, box.min.z,
        box.min.x, box.min.y, box.max.z,
        box.max.x, box.min.y, box.max.z,
        box.max.x, box.max.y, box.max.z,
        box.min.x, box.max.y, box.max.z
    };

    unsigned int indices[] = {
        0,1, 1,2, 2,3, 3,0, // front
        4,5, 5,6, 6,7, 7,4, // back
        0,4, 1,5, 2,6, 3,7  // connecting
    };

    static GLuint VAO = 0, VBO, EBO;
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shader);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader, "debugColor"), 0.0f, 1.0f, 0.0f);

    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Main
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

    //loading models error handler
    if (!TestLevel.Load("TestLevel.obj")) {
        std::cerr << "Failed to load model" << std::endl;
        return -1;
    }

    // Shaders
    unsigned int lightingShader = createShaderProgram(vertexShaderSource, fragmentShaderSource1);
    unsigned int skyboxShader = createShaderProgram(CubeMapVShader, CubeMapFShader);
	unsigned int debugShader = createShaderProgram(debugVertexShader, debugFragmentShader);

    // Skybox 
    std::vector<std::string> faces = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };
    Skybox skybox(faces, skyboxShader);

    // Sphere Setup
    Sphere sphere(1.0f, 36, 18, true);

    GLuint sphereVAO, sphereVBO, sphereIBO;
    glGenVertexArrays(1, &sphereVAO);
    glBindVertexArray(sphereVAO);

    glGenBuffers(1, &sphereVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.getInterleavedVertexSize(),
        sphere.getInterleavedVertices(), GL_STATIC_DRAW);

    glGenBuffers(1, &sphereIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.getIndexSize(),
        sphere.getIndices(), GL_STATIC_DRAW);

    int stride = sphere.getInterleavedStride();

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));

    // Texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glUseProgram(lightingShader);

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Frames Counter
    double prevTime = 0.0;
    double crntTime = 0.0;  
    double timeDiff = 0.0;
	unsigned int counter = 0;

    // Main Render Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        double currentTime = glfwGetTime();
        counter++;
        timeDiff = currentTime - prevTime;

        // Update every 1/30th second (~0.0333s)
        if (timeDiff >= 1.0 / 30.0) {
            FramePerSecond = counter / timeDiff;
            counter = 0;
            prevTime = currentTime;
        }

        processInput(window);

        // Clear Buffers
        glClearColor(ScreenColor[0], ScreenColor[1], ScreenColor[2], ScreenColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Calculate intended movement
        player.CalculateMovement(deltaTime);
        glm::vec3 intendedMove = player.GetIntendedMovement();

        // Check what collision WOULD happen
        glm::vec3 futurePosition = player.sphereCenter + intendedMove;
        CollisionResult collision = levelCollision.CheckSphereCollisionDetailed(
            futurePosition, player.sphereScale);

        // Apply corrected movement
        glm::vec3 finalMovement = intendedMove;

        if (collision.collided) {
            player.Colliding = true;
            player.collisionNormal = collision.collisionNormal;
            player.collisionDepth = static_cast<int>(collision.penetrationDepth);

            // If colliding with floor, stop vertical movement
            if (collision.collisionNormal.y > 0.5f) {
                finalMovement.y = 0.0f;
                player.verticalVelocity = 0.0f;
                player.jumpCount = 0;
            }

            // Project movement along collision surface for sliding
            glm::vec3 slidingMove = player.CollideAndSlide(finalMovement, 0);
            finalMovement = slidingMove;

            // Push out of any penetration
            if (collision.penetrationDepth > 0.0f) {
                finalMovement += collision.collisionNormal *
                    (collision.penetrationDepth + player.skinWidth);
            }
        }
        else {
            player.Colliding = false;
        }

        // Actually move the player with the corrected movement
        player.ApplyMovement(finalMovement);
        player.Update(deltaTime);
        
        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(player.GetZoom()), 1980.0f / 1080.0f, 0.01f, 100.0f);
        glm::mat4 view = player.GetViewMatrix();

        // Lighting Pass
        glUseProgram(lightingShader);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, glm::value_ptr(player.GetPosition()));

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
        glUniform3fv(glGetUniformLocation(lightingShader, "spotLight.position"), 1, glm::value_ptr(player.GetPosition()));
        glUniform3fv(glGetUniformLocation(lightingShader, "spotLight.direction"), 1, glm::value_ptr(player.GetFront()));
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.diffuse"), SpotLightDiff[0], SpotLightDiff[1], SpotLightDiff[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "spotLight.specular"), SpotLightSpec[0], SpotLightSpec[1], SpotLightSpec[2]);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.cutOff"), glm::cos(glm::radians(SpotlightInnerCutoff)));
        glUniform1f(glGetUniformLocation(lightingShader, "spotLight.outerCutOff"), glm::cos(glm::radians(SpotlightOuterCutoff)));

        // Render Test Level
        glm::mat4 modelTestLevel = glm::mat4(1.0f);
        modelTestLevel = glm::translate(modelTestLevel, glm::vec3(0.0f, -16.5f, 0.0f));
        modelTestLevel = transformer.ScaleMeshComb(modelTestLevel, 2.0f);
		levelCollision.BuildFromModel(TestLevel, modelTestLevel);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(modelTestLevel));
        TestLevel.Render(lightingShader);
        glUseProgram(lightingShader);

        // Render Sphere
        if (showSphere) {
            glm::mat4 modelSphere = glm::mat4(1.0f);
            modelSphere = glm::translate(modelSphere, player.spherePosition);
            modelSphere = glm::scale(modelSphere, glm::vec3(player.sphereScale));

            glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(modelSphere));

            glBindVertexArray(sphereVAO);
            glDrawElements(GL_TRIANGLES, sphere.getIndexCount(), GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
        }

        // Debug Collision Boxes
        if (showCollisionBoxes) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST); // Draw on top

            auto boxes = levelCollision.GetBoundingBoxes();
            for (const auto& box : boxes) {
                RenderDebugBox(box, debugShader, view, projection);
            }

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        // Skybox Pass
        if (skyBoxOn) {
            skybox.Render(view, projection);
        }

        // ImGui Frame
        ImGui::Begin("Scene Controls");
        ImGui::Checkbox("Skybox?", &skyBoxOn);
        ImGui::ColorEdit4("Sky Color", ScreenColor);

        ImGui::Separator();
        ImGui::Text("Sphere Controls");
        ImGui::Checkbox("Show Sphere", &showSphere);

        ImGui::Separator();
        ImGui::Text("Directional Light");
        ImGui::SliderFloat3("Directional Light Direction", DirLightDirection, -1.0f, 1.0f);
        ImGui::ColorEdit3("Directional Light Ambient", DirLightAmbient);
        ImGui::ColorEdit3("Directional Light Specular", DirLightSpec);
        ImGui::ColorEdit3("Directional Light Diffuse", DirLightDiff);
        ImGui::SliderFloat("Dir Light Intensity", &DirLightIntensity, 0.1f, 50.0f);

        ImGui::Separator();
        ImGui::Text("Camera Position:");
        ImGui::Text("X: %.2f", player.GetPosition().x);
        ImGui::Text("Y: %.2f", player.GetPosition().y);
        ImGui::Text("Z: %.2f", player.GetPosition().z);

        ImGui::Text("Sphere/Center of Sphere Position:");
        ImGui::Text("X: %.2f", player.sphereCenter.x);
        ImGui::Text("Y: %.2f", player.sphereCenter.y);
        ImGui::Text("Z: %.2f", player.sphereCenter.z);

        ImGui::Separator();
        ImGui::Text("Debug Visualization");
        ImGui::Checkbox("Show Collision Boxes", &showCollisionBoxes);

        ImGui::Separator();
        ImGui::Text("FPS: %.2f", FramePerSecond);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // End Frame
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereIBO);

    skybox.Cleanup();
    glDeleteProgram(lightingShader);
    glDeleteProgram(skyboxShader);

    glfwTerminate();
    return 0;
}