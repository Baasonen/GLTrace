// Copyright (c) 2026 Henri Paasonen - GPLv2
// See LICENSE for details

#include <glad/glad.h>
#include <glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "file_util.h"
#include "shader_structs.h"
#include "obj_loader.h"
#include "bvh.h"
#include "matrix.h"
#include "scene_loader.h"

#ifndef M_PI
#define M_PI 3.1415
#endif

#ifdef _WIN32
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif


float radians(float deg) {return deg * (M_PI / 180.0f);}

#define WIDTH 1920
#define HEIGHT 1080

bool g_firstMouse = true;
float g_lastX = WIDTH / 2.0f;
float g_lastY = HEIGHT / 2.0f;
float g_mouseSensitivity = 0.1f;

int g_frameCount = 0;
Camera g_camera = {0.0f, 0.0f, 200.0f, -90.0f, 0.0f, 1.0f};
float g_cameraSpeed = 100.0f;
bool cameraLock = false;

float g_lastFrame = 0.0f;
float g_deltaTime = 0.0f;

bool g_framebufferResized = false;
int g_newWidth = WIDTH;
int g_newHeight = HEIGHT;

void calculateCameraVectors(Camera* camera, float* forwardX, float* forwardZ, float* rightX, float* rightZ)
{
    float yawRads = radians(camera -> yaw);

    *forwardX = cos(yawRads);
    *forwardZ = sin(yawRads);

    *rightX = cos(yawRads + radians(90.0f));
    *rightZ = sin(yawRads + radians(90.0f));
}

bool processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    bool moved = false;
    float cameraVelocity = g_cameraSpeed * g_deltaTime;

    float forwardX, forwardZ, rightX, rightZ;
    calculateCameraVectors(&g_camera, &forwardX, &forwardZ, &rightX, &rightZ);

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {cameraLock = !cameraLock;}
    
    if (cameraLock) {return moved;}

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_camera.x += forwardX * cameraVelocity;
        g_camera.z += forwardZ * cameraVelocity;
        moved = true;
    }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_camera.x -= forwardX * cameraVelocity;
        g_camera.z -= forwardZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_camera.x -= rightX * cameraVelocity;
        g_camera.z -= rightZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_camera.x += rightX * cameraVelocity;
        g_camera.z += rightZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        g_camera.y += cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        g_camera.y -= cameraVelocity;
        moved = true;
    }

    return moved;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_newWidth = width;
    g_newHeight = height;
    g_framebufferResized = true;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (g_firstMouse)
    {
        g_lastX = (float)xpos;
        g_lastY = (float)ypos;
        g_firstMouse = false;
        return;
    }

    float xOffset = (float)xpos - g_lastX;
    float yOffset = g_lastY - (float)ypos;
    g_lastX = (float)xpos;
    g_lastY = (float)ypos;
    
    xOffset *= g_mouseSensitivity;
    yOffset *= g_mouseSensitivity;

    g_camera.yaw += xOffset;
    g_camera.pitch += yOffset;

    if (g_camera.pitch > 89.0f) {g_camera.pitch = 89.0f;}
    if (g_camera.pitch < -89.0f) {g_camera.pitch = -89.0f;}

    g_frameCount = 0;
}

GLuint compileShader(const char* filename, GLenum type)
{
    char* source = readFileToString(filename);
    if (source == NULL) {return 0;}

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char* const*)&source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compile error in %s: \n %s\n", filename, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    free(source);

    return shader;
}

GLuint createComputeProgram()
{
    GLuint computeShader = compileShader("shaders/raytrace.comp", GL_COMPUTE_SHADER);
    if (!computeShader) {return 0;}

    GLuint program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Linking error \n %s \n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(computeShader);

    return program;
}

GLuint createShaderProgram()
{
    GLuint vertexShader = compileShader("shaders/fullscreen.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader("shaders/display.frag", GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0) {return 0;}

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Linking error \n %s \n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint g_accumTexture;
GLuint g_outputTexture;

// Init frame buffers
void setupAccumulationBuffers(int width, int height)
{
    // TX1
    glGenTextures(1, &g_accumTexture);
    glBindTexture(GL_TEXTURE_2D, g_accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // TX2
    glGenTextures(1, &g_outputTexture);
    glBindTexture(GL_TEXTURE_2D, g_outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_accumTexture, 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Framebuffer incomplete\n");
    }
}

void setupTextures(int width, int height)
{
    if (g_accumTexture) glDeleteTextures(1, &g_accumTexture);
    if (g_outputTexture) glDeleteTextures(1, &g_outputTexture);

    // TX2
    glGenTextures(1, &g_accumTexture);
    glBindTexture(GL_TEXTURE_2D, g_accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // TX 2
    glGenTextures(1, &g_outputTexture);
    glBindTexture(GL_TEXTURE_2D, g_outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
}

MeshData buildSceneMesh(SceneDescription* scene)
{
    MeshData combinedMesh = {0};

    int totalVertices = 0;
    int totalIndices = 0;

    for (int i = 0; i < scene->numberOfInstances; i++)
    {
        int srcIndex = scene->meshInstances[i].meshSourceIndex;

        if (srcIndex >= scene->numberOfSources) {continue;}

        totalVertices += scene->meshSources[srcIndex].vertexCount;
        totalIndices += scene->meshSources[srcIndex].indexCount;
    }

    combinedMesh.vertices = (GPUPackedVertex*)malloc(sizeof(GPUPackedVertex) * totalVertices);
    combinedMesh.indices = (uint32_t*)malloc(sizeof(uint32_t) * totalIndices);
    combinedMesh.triangleMaterials = (uint32_t*)malloc(sizeof(uint32_t) * (totalIndices / 3));

    combinedMesh.vertexCount = totalVertices;
    combinedMesh.indexCount = totalIndices;

    int vOffset = 0;
    int iOffset = 0;
    int tOffset = 0;
    
    for (int i = 0; i < scene->numberOfInstances; i++)
    {
        MeshInstance* instance = &scene->meshInstances[i];
        int srcIndex = instance->meshSourceIndex;

        if (srcIndex >= scene->numberOfSources) {continue;}

        MeshData* sourceMesh = &scene->meshSources[srcIndex];

        Mat4 modelMatrix = transformMatrix(instance->pos, instance->scale, instance->rotation);

        for (int v = 0; v < sourceMesh->vertexCount; v++)
        {
            Vec4 localPos;
            localPos.x = sourceMesh->vertices[v].x;
            localPos.y = sourceMesh->vertices[v].y;
            localPos.z = sourceMesh->vertices[v].z;
            localPos.a = 1.0f;

            Vec4 worldPos = matrixMultiplyVec4(modelMatrix, localPos);

            combinedMesh.vertices[vOffset + v].x = worldPos.x;
            combinedMesh.vertices[vOffset + v].y = worldPos.y;
            combinedMesh.vertices[vOffset + v].z = worldPos.z;
        }
        for (int idx = 0; idx < sourceMesh->indexCount; idx++)
        {
            combinedMesh.indices[iOffset + idx] = sourceMesh->indices[idx] + vOffset;
        }

        int triangleCount = sourceMesh->indexCount / 3;
        for (int t = 0; t < triangleCount; t++)
        {
            combinedMesh.triangleMaterials[tOffset + t] = instance->materialIndex;
        }

        vOffset += sourceMesh->vertexCount;
        iOffset += sourceMesh->indexCount;
        tOffset += triangleCount;
        }

    combinedMesh.triangleCount = totalIndices / 3;

    return combinedMesh;
}

void setupSceneData(GLuint sphereSSBO, GLuint materialSSBO, GLuint vertexSSBO, GLuint indexSSBO, GLuint bvhSSBO, GLuint triangleMaterialSSBO, SceneDescription* sceneDesc)
{
    MeshData sceneMesh;
    sceneMesh = buildSceneMesh(sceneDesc);

    BVH bvh;
    buildBVH(&bvh, &sceneMesh);
    // Upload vertices
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUPackedVertex) * sceneMesh.vertexCount, sceneMesh.vertices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vertexSSBO);
    // Upload indices
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * sceneMesh.indexCount, sceneMesh.indices, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, indexSSBO);
    // Upload BVH nodes
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bvhSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNode) * bvh.nodeCount, bvh.nodes, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bvhSSBO);

    // Material data for triangles
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleMaterialSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * sceneMesh.triangleCount, sceneMesh.triangleMaterials, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, triangleMaterialSSBO);

    // Materials
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * sceneDesc->materialCount, sceneDesc->materials, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialSSBO);

    free(bvh.nodes);
    freeMeshData(&sceneMesh);

    // Sphere data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);

    if (sceneDesc->sphereCount > 0)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * sceneDesc->sphereCount, sceneDesc->spheres, GL_STATIC_DRAW);
    }
    else
    {
        // Allocate some memory to prevent GL errors if empty
        Sphere placeholderSphere = {0};
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere), &placeholderSphere, GL_STATIC_DRAW);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereSSBO);
}

int main(int argc, char* argv[])
{
    char scenePath[512];

    if (argc > 1)
    {
        snprintf(scenePath, sizeof(scenePath), "scenes/%s", argv[1]);
    }
    else
    {
        strncpy(scenePath, "scenes/1.scene", sizeof(scenePath));
    }

    printf("\nGLTrace, loading: %s\n", scenePath);

    if (!glfwInit())
    {
        fprintf(stderr, "GLFW init failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GLTrace", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "GLFW window creation failed\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Capture and hide mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    // Load GL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to load GLAD\n");
        glfwTerminate();
        return 1;
    }

    // Vsync
    glfwSwapInterval(0);

    // Buffer setup
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ssboSpheres;
    GLuint ssboMaterials;
    GLuint ssboVertices;
    GLuint ssboIndices;
    GLuint ssboBVH;
    GLuint ssboTriangleMaterial;

    glGenBuffers(1, &ssboSpheres);
    glGenBuffers(1, &ssboMaterials);
    glGenBuffers(1, &ssboVertices);
    glGenBuffers(1, &ssboIndices);
    glGenBuffers(1, &ssboBVH);
    glGenBuffers(1, &ssboTriangleMaterial);

    SceneDescription scene;

    if (!loadScene(scenePath, &scene))
    {
        fprintf(stderr, "Failed to load scene %s\n", scenePath);
        return 1;
    }

    setupSceneData(ssboSpheres, ssboMaterials, ssboVertices, ssboIndices, ssboBVH, ssboTriangleMaterial, &scene);
    
    int day = 1;

    //GLuint program = createShaderProgram();
    //glUseProgram(program);

    //setupAccumulationBuffers(WIDTH, HEIGHT);

    GLuint computeProgram = createComputeProgram();
    GLuint displayProgram = createShaderProgram();

    setupTextures(WIDTH, HEIGHT);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        g_deltaTime = currentFrame - g_lastFrame;
        g_lastFrame = currentFrame;

        //printf("%f, %f, %f\n", g_camera.x, g_camera.y, g_camera.z);

        if (g_framebufferResized)
        {
            setupTextures(g_newWidth, g_newHeight);

            g_frameCount = 0;
            g_framebufferResized = false;
        }

        bool cameraMoved = processInput(window);
        if (cameraMoved) {g_frameCount = 0;}
        g_frameCount++;

        Vec4 forward = {0.0f, 0.0f, 0.0f, 0.0f};

        forward.x = cos(radians(g_camera.yaw)) * cos(radians(g_camera.pitch));
        forward.y = sin(radians(g_camera.pitch));
        forward.z = sin(radians(g_camera.yaw)) * cos(radians(g_camera.pitch));
        normalize(&forward);

        Vec4 up = {0.0f, 1.0f, 0.0f, 0.0f};
        Vec4 right = crossProduct(forward, up);
        normalize(&right);

        Vec4 trueUp = crossProduct(right, forward);
        normalize(&trueUp);

        glUseProgram(computeProgram);

        glUniform3f(glGetUniformLocation(computeProgram, "u_camForward"), forward.x, forward.y, forward.z);
        glUniform3f(glGetUniformLocation(computeProgram, "u_camRight"), right.x, right.y, right.z);
        glUniform3f(glGetUniformLocation(computeProgram, "u_camUp"), trueUp.x, trueUp.y, trueUp.z);
        glUniform2f(glGetUniformLocation(computeProgram, "u_resolution"), (float)g_newWidth, (float)g_newHeight);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_accumTexture, 0);

        glUniform1i(glGetUniformLocation(computeProgram, "u_frameCount"), g_frameCount);
        glUniform1i(glGetUniformLocation(computeProgram, "u_historyTexture"), 0);
        glUniform3f(glGetUniformLocation(computeProgram, "u_cameraPos"), g_camera.x, g_camera.y, g_camera.z);
        glUniform1f(glGetUniformLocation(computeProgram, "u_cameraYaw"), g_camera.yaw);
        glUniform1f(glGetUniformLocation(computeProgram, "u_cameraPitch"), g_camera.pitch);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, g_accumTexture);
        glUniform1i(glGetUniformLocation(computeProgram, "u_historyTexture"), 0);

        // Binding 0: Write Output (Image Unit)
        glBindImageTexture(0, g_outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((g_newWidth + 15) / 16, (g_newHeight + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glViewport(0, 0, g_newWidth, g_newHeight);
        glClear(GL_COLOR_BUFFER_BIT); // Clear default framebuffer

        glUseProgram(displayProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_outputTexture);
        glUniform1i(glGetUniformLocation(displayProgram, "u_texture"), 0);

        // Draw Fullscreen Quad
        glDrawArrays(GL_TRIANGLES, 0, 3);

        GLuint temp = g_accumTexture;
        g_accumTexture = g_outputTexture;
        g_outputTexture = temp;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }   

    glDeleteBuffers(1, &ssboBVH);
    glDeleteBuffers(1, &ssboIndices);
    glDeleteBuffers(1, &ssboVertices);
    glDeleteBuffers(1, &ssboMaterials);
    glDeleteBuffers(1, &ssboTriangleMaterial);

    glDeleteTextures(1, &g_accumTexture);
    glDeleteTextures(1, &g_outputTexture);
    glDeleteVertexArrays(1, &vao);

    glFinish();
    glfwTerminate();
    return 0;
}