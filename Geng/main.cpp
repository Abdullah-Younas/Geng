// ================== Includes ==================
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <iostream>
#include <string>
using namespace std;

#include "shader_utils.h"
#include "shaders.h"
#include "Camera.h"
#include "Transformations.h"
#include "CallBacks.h"

// ================== Globals ==================
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
Transformations transformer;

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
}

// ================== Texture Loader ==================
unsigned int loadTexture(const char* path);

// ================== Main ==================
int main() {
    // GLFW Init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Lighting", NULL, NULL);
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

    glfwSetCursorPosCallback(window, CallBacks::mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, CallBacks::scroll_callback);

    // ================== Vertex Data ==================
    float vertices[] = {
        // positions          // normals           // tex coords
         -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // ================== VAO/VBO Setup ==================
    unsigned int VBO, cubeVAO, lightVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    // Cube VAO
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Light VAO
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ================== Textures ==================
    unsigned int specularMap = loadTexture("container2.jpg");
    unsigned int diffuseMap = loadTexture("container.jpg");

    // ================== Shaders ==================
    unsigned int lightingShader = createShaderProgram(vertexShaderSource, fragmentShaderSource1);
    unsigned int lampShader = createShaderProgram(vertexShaderSource, lampFragmentShaderSource);

    glUseProgram(lightingShader);
    glUniform1i(glGetUniformLocation(lightingShader, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader, "material.specular"), 1);

    // ================== Main Render Loop ==================
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = float(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Clear Buffers
        glClearColor(0.75f, 0.52f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ========== Lighting Pass ==========
        glUseProgram(lightingShader);
        glm::mat4 model = glm::mat4(1.0f);
        model = transformer.RotMeshY(model, glfwGetTime() * 30.0f);
        model = transformer.RotMeshX(model, glfwGetTime() * 20.0f);
        model = transformer.ScaleMeshComb(model, 0.75f);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Lighting + Material Setup
        /*/for (int i = 0; i < 1; i++) {
            glm::vec3 lightPos = cubePositions[i] * 2.0f; // or your light positions
            string uniformName = "lightPos" + to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(lightingShader, uniformName.c_str()), 1, glm::value_ptr(lightPos));
        }*/
        //glUniform3f(glGetUniformLocation(lightingShader, "light.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3fv(glGetUniformLocation(lightingShader, "light.position"), 1, glm::value_ptr(camera.Position));
        glUniform3fv(glGetUniformLocation(lightingShader, "light.direction"), 1, glm::value_ptr(camera.Front));
        glUniform1f(glGetUniformLocation(lightingShader, "light.cutOff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(lightingShader, "light.outerCutOff"), glm::cos(glm::radians(17.5f)));
        glUniform1f(glGetUniformLocation(lightingShader, "light.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader, "light.linear"), 0.09f); 
        glUniform1f(glGetUniformLocation(lightingShader, "light.quadratic"), 0.032f);
        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform1f(glGetUniformLocation(lightingShader, "material.shininess"), 32.0f);
        glUniform3f(glGetUniformLocation(lightingShader, "material.ambient"), 1.0f, 0.5f, 0.31f);
        glUniform3f(glGetUniformLocation(lightingShader, "material.specular"), 0.5f, 0.5f, 0.5f);


        glm::vec3 lightColor(1.0f);
        glm::vec3 ambientColor = lightColor * 0.5f * .1f;
        glm::vec3 diffuseColor = lightColor * .5f;
        glm::vec3 specularColor = glm::vec3(1.0f);

        glUniform3f(glGetUniformLocation(lightingShader, "light.ambient"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(lightingShader, "light.diffuse"),  0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(lightingShader, "light.specular"),  1.0f, 1.0f, 1.0f);

        // Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // Render Cube
        glBindVertexArray(cubeVAO);

        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f); // Start with identity
            model = glm::translate(model, cubePositions[i]);

            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            // === Send model matrix to shader manually ===
            int modelLoc = glGetUniformLocation(lightingShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // === Now draw the object ===
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // ========== Lamp Pass ==========
        /*
        glUseProgram(lampShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.25f));

        glUniformMatrix4fv(glGetUniformLocation(lampShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


        for (unsigned int i = 0; i < 2; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            // Multiply original position by a factor to spread them out more
            glm::vec3 adjustedPos = cubePositions[i] * 1.0f; // e.g., double spacing

            // Optional: add some offset to Y or Z to shift them in interesting ways
            adjustedPos.y += sin(i); // for wave effect

            model = glm::translate(model, adjustedPos);

            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            model = transformer.ScaleMeshComb(model, 0.3f);

            int modelLoc = glGetUniformLocation(lampShader, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/


        // ========== End Frame ==========
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ================== Cleanup ==================
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightingShader);
    glDeleteProgram(lampShader);

    glfwTerminate();
    return 0;
}
