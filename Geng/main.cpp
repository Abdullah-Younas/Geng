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
#include <OBJLoad/OBJ_Loader.h>

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "shader_utils.h"
#include "shaders.h"
#include "Camera.h"
#include "Transformations.h"
#include "TextureImage.h"
#include "CallBacks.h"

// ================== Globals ==================
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float ScreenColor[4] = { 0.75f, 0.80f, 0.85f, 1.0f };

float DirLightSpec[3] = { 0.5f, 0.5f, 0.5f };
float DirLightDiff[3] = { 0.4f, 0.4f, 0.4f };

float PointLightSpec[3] = { 0.5f, 0.5f, 0.5f };
float PointLightDiff[3] = { 0.4f, 0.4f, 0.4f };

float SpotLightDiff[3] = { 0.4f, 0.4f, 0.4f };
float SpotLightSpec[3] = { 0.5f, 0.5f, 0.5f };
float SpotlightInnerCutoff = 8.0f;
float SpotlightOuterCutoff = 12.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
Transformations transformer;
objl::Loader Oloader;

struct MeshData {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int indexCount;
    objl::Material material;
};

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

// ================== Texture Loader ==================
/*unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}*/

// ================== Main ==================
int main() {
    // GLFW Init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "GENG", NULL, NULL);
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

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
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

    // ================== OBJ LOADER ===================
    bool loaded = Oloader.LoadFile("A:\\AbdullahWork\\GraphicsEng\\Geng\\nissan_silvia_s13_low-poly.obj");
    if (!loaded) {
        std::cerr << "Failed to load OBJ file" << std::endl;
        return -1;
    }

    Oloader.LoadedMaterials.clear();

    // Create material 0 (mat0)
    {
        objl::Material mat;
        mat.name = "mat0";
        mat.Ns = 236.33569f;
        mat.Ka = objl::Vector3(1, 1, 1);
        mat.Kd = objl::Vector3(1, 1, 1);
        mat.Ks = objl::Vector3(0.5, 0.5, 0.5);
        mat.map_Kd = "material_baseColor.jpg";
        Oloader.LoadedMaterials.push_back(mat);
    }

    // Create material 1 (mat1)
    {
        objl::Material mat;
        mat.name = "mat1";
        mat.Ns = 252.9822f;
        mat.Ka = objl::Vector3(1, 1, 1);
        mat.Kd = objl::Vector3(1, 1, 1);
        mat.Ks = objl::Vector3(0.5, 0.5, 0.5);
        mat.map_Kd = "_009_baseColor.jpg";
        Oloader.LoadedMaterials.push_back(mat);
    }

    {
        objl::Material mat;
        mat.name = "mat2";
        mat.Ns = 68.60834;
        mat.Ka = objl::Vector3(1, 1, 1);
        mat.Kd = objl::Vector3(0.32314, 0.31399, 0.18782);
        mat.Ks = objl::Vector3(0.5, 0.5, 0.5);
        Oloader.LoadedMaterials.push_back(mat);
    }
    {
        objl::Material mat;
        mat.name = "mat3";
        mat.Ns = 51.32967;
        mat.Ka = objl::Vector3(1, 1, 1);
        mat.Kd = objl::Vector3(0.18448, 0.2384, 0.27468);
        mat.Ks = objl::Vector3(0.5, 0.5, 0.5);
        Oloader.LoadedMaterials.push_back(mat);
    }
    {
        objl::Material mat;
        mat.name = "mat4";
        mat.Ns = 25.70027;
        mat.Ka = objl::Vector3(1, 1, 1);
        mat.Kd = objl::Vector3(0.11494, 0.11494, 0.11494);
        mat.Ks = objl::Vector3(0.5, 0.5, 0.5);
        Oloader.LoadedMaterials.push_back(mat);
    }
    {
        objl::Material mat;
        mat.name = "mat5";
        mat.Ns = 250;
        mat.Ka = objl::Vector3(1, 1, 1);
        mat.Kd = objl::Vector3(0.85, 0.85, 0.85);
        Oloader.LoadedMaterials.push_back(mat);
    }

    // Assign materials to meshes
    for (auto& mesh : Oloader.LoadedMeshes) {
        for (auto& mat : Oloader.LoadedMaterials) {
            if (mat.name == mesh.MeshName) {
                mesh.MeshMaterial = mat;
                break;
            }
        }
    }

    // Load textures for each mesh
    vector<unsigned int> diffuseMaps;
    vector<unsigned int> specularMaps;

    for (auto& mesh : Oloader.LoadedMeshes) {
        // Load diffuse texture
        if (!mesh.MeshMaterial.map_Kd.empty()) {
            diffuseMaps.push_back(loadTexture(mesh.MeshMaterial.map_Kd.c_str()));
        }
        else {
            // Default white texture
            unsigned int whiteTex;
            glGenTextures(1, &whiteTex);
            glBindTexture(GL_TEXTURE_2D, whiteTex);
            unsigned char whitePixel[] = { 255, 255, 255, 255 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
            diffuseMaps.push_back(whiteTex);
        }

        // Load specular texture
        if (!mesh.MeshMaterial.map_Ks.empty()) {
            specularMaps.push_back(loadTexture(mesh.MeshMaterial.map_Ks.c_str()));
        }
        else {
            // Default black texture
            unsigned int blackTex;
            glGenTextures(1, &blackTex);
            glBindTexture(GL_TEXTURE_2D, blackTex);
            unsigned char blackPixel[] = { 0, 0, 0, 255 };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackPixel);
            specularMaps.push_back(blackTex);
        }
    }

    // Create VAOs for each mesh
    vector<MeshData> modelMeshes;
    for (auto& mesh : Oloader.LoadedMeshes) {
        MeshData meshData;
        meshData.material = mesh.MeshMaterial;

        glGenVertexArrays(1, &meshData.VAO);
        glGenBuffers(1, &meshData.VBO);
        glGenBuffers(1, &meshData.EBO);

        glBindVertexArray(meshData.VAO);

        // Vertex data
        glBindBuffer(GL_ARRAY_BUFFER, meshData.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.Vertices.size() * sizeof(objl::Vertex),
            &mesh.Vertices[0], GL_STATIC_DRAW);

        // Index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.Indices.size() * sizeof(unsigned int),
            &mesh.Indices[0], GL_STATIC_DRAW);

        // Vertex attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex),
            (void*)offsetof(objl::Vertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex),
            (void*)offsetof(objl::Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(objl::Vertex),
            (void*)offsetof(objl::Vertex, TextureCoordinate));

        meshData.indexCount = mesh.Indices.size();
        modelMeshes.push_back(meshData);
    }

    // ================== Shaders ==================
    unsigned int lightingShader = createShaderProgram(vertexShaderSource, fragmentShaderSource1);
    unsigned int lampShader = createShaderProgram(vertexShaderSource, lampFragmentShaderSource);

    // Set texture units
    glUseProgram(lightingShader);
    glUniform1i(glGetUniformLocation(lightingShader, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader, "material.specular"), 1);

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
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ========== Lighting Pass ==========
        glUseProgram(lightingShader);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, -5.0f));
        model = glm::scale(model, glm::vec3(1.0f));

        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Lighting setup
        glUniform3fv(glGetUniformLocation(lightingShader, "viewPos"), 1, glm::value_ptr(camera.Position));

        // Directional light
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.ambient"), 0.05f, 0.05f, 0.05f);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.diffuse"), DirLightDiff[0], DirLightDiff[1], DirLightDiff[2]);
        glUniform3f(glGetUniformLocation(lightingShader, "dirLight.specular"), DirLightSpec[0], DirLightSpec[1], DirLightSpec[2]);

        // Point lights
        for (int i = 0; i < 4; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            glUniform3fv(glGetUniformLocation(lightingShader, (base + ".position").c_str()), 1, &pointLightPositions[i][0]);
            glUniform3f(glGetUniformLocation(lightingShader, (base + ".ambient").c_str()), 0.05f, 0.05f, 0.05f);
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

        // Render each mesh with its material properties
        for (size_t i = 0; i < modelMeshes.size(); i++) {
            auto& mesh = modelMeshes[i];
            auto& mat = mesh.material;

            // Bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMaps[i]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMaps[i]);

            // Set material properties
            glUniform3f(glGetUniformLocation(lightingShader, "material.ambient"),
                mat.Ka.X, mat.Ka.Y, mat.Ka.Z);
            glUniform3f(glGetUniformLocation(lightingShader, "material.diffuseColor"),
                mat.Kd.X, mat.Kd.Y, mat.Kd.Z);
            glUniform3f(glGetUniformLocation(lightingShader, "material.specularColor"),
                mat.Ks.X, mat.Ks.Y, mat.Ks.Z);
            glUniform1f(glGetUniformLocation(lightingShader, "material.shininess"),
                mat.Ns);

            // Draw mesh
            glBindVertexArray(mesh.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }

        // ========== Lamp Pass ==========
        glUseProgram(lampShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.25f));

        glUniformMatrix4fv(glGetUniformLocation(lampShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lampShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(lampShader, "color"), PointLightDiff[0], PointLightDiff[1], PointLightDiff[2]);

        for (unsigned int i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            glm::vec3 adjustedPos = pointLightPositions[i];
            adjustedPos.y += sin(i);
            model = glm::translate(model, adjustedPos);

            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            model = transformer.ScaleMeshComb(model, 0.3f);

            glUniformMatrix4fv(glGetUniformLocation(lampShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ========== ImGui ==========
        ImGui::Begin("Hehe, me is window");
        ImGui::ColorEdit4("Sky Color", ScreenColor);
        ImGui::Text("Directional Light");
        ImGui::ColorEdit3("Directional Light Specular", DirLightSpec);
        ImGui::ColorEdit3("Directional Light Diffuse", DirLightDiff);
        ImGui::Text("Point Light");
        ImGui::ColorEdit3("Point Light Specular", PointLightSpec);
        ImGui::ColorEdit3("Point Light Diffuse", PointLightDiff);
        ImGui::Text("Spot Light");
        ImGui::ColorEdit3("Spot Light Specular", SpotLightSpec);
        ImGui::ColorEdit3("Spot Light Diffuse", SpotLightDiff);
        ImGui::SliderFloat("Inner Cut Off", &SpotlightInnerCutoff, 3.0f, 20.0f);
        ImGui::SliderFloat("Inner Outer Off", &SpotlightOuterCutoff, 5.0f, 25.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // ========== End Frame ==========
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ================== Cleanup ==================
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for (auto& mesh : modelMeshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(lightingShader);
    glDeleteProgram(lampShader);

    glfwTerminate();
    return 0;
}