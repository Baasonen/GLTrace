// Copyright (c) 2026 Henri Paasonen - GPLv2
// See LICENSE for details
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <bvec/bvec.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "math_utils.h"
#include "file_util.h"
#include "shader_structs.h"
#include "obj_loader.h"
#include "bvh.h"
#include "scene_loader.h"
#include "camera.h"
#include "state.h"
#include "config.h"
#include "input.h"

#ifdef _WIN32
__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif





void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_program.newWidth = width;
    g_program.newHeight = height;
    g_program.framebufferResized = true;
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

GLuint createComputeProgram(const char* filename)
{
    GLuint computeShader = compileShader(filename, GL_COMPUTE_SHADER);
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

GLuint g_normalTexture;
GLuint g_denoisedTexture;
GLuint g_denoiseSwapTexture;

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

    if (g_normalTexture) glDeleteTextures(1, &g_normalTexture);
    if (g_denoisedTexture) glDeleteTextures(1, &g_denoisedTexture);

    glGenTextures(1, &g_normalTexture);
    glBindTexture(GL_TEXTURE_2D, g_normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &g_denoisedTexture);
    glBindTexture(GL_TEXTURE_2D, g_denoisedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &g_denoiseSwapTexture);
    glBindTexture(GL_TEXTURE_2D, g_denoiseSwapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
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

        vec3 axisY = {.x = 0.0f, .y = 1.0f, .z = 0.0f};
        mat4 modelMatrix = mat4Transform(instance->pos, axisY, instance->rotation.y, instance->scale);

        for (int v = 0; v < sourceMesh->vertexCount; v++)
        {
            vec4 localPos;
            localPos.x = sourceMesh->vertices[v].x;
            localPos.y = sourceMesh->vertices[v].y;
            localPos.z = sourceMesh->vertices[v].z;
            localPos.w = 1.0f;

            vec4 worldPos = mat4MulVec4(modelMatrix, localPos);

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

    calculateMeshNormals(&combinedMesh);

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

    printf("\nGLTrace, loading: %s\n\n", scenePath);

    if (!glfwInit())
    {
        fprintf(stderr, "GLFW init failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "GLTrace", NULL, NULL);
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

    glfwSetKeyCallback(window, keyCallback);

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

    GLuint computeProgram = createComputeProgram("shaders/raytrace.comp");
    GLuint displayProgram = createShaderProgram();
    GLuint denoiseProgram = createComputeProgram("shaders/denoise.comp");

    setupTextures(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        g_program.deltaTime = currentFrame - g_program.lastFrame;
        g_program.lastFrame = currentFrame;

        //printf("%f, %f, %f\n", g_camera.x, g_camera.y, g_camera.z);

        if (g_program.framebufferResized)
        {
            setupTextures(g_program.newWidth, g_program.newHeight);

            g_program.frameCount = 0;
            g_program.framebufferResized = false;
        }

        bool cameraMoved = processInput(window);
        if (cameraMoved) {g_program.frameCount = 0;}
        g_program.frameCount++;

        vec3 forward = vec3Zero();

        forward.x = cos(radians(g_program.camera.yaw)) * cos(radians(g_program.camera.pitch));
        forward.y = sin(radians(g_program.camera.pitch));
        forward.z = sin(radians(g_program.camera.yaw)) * cos(radians(g_program.camera.pitch));
        forward = vec3Normalize(forward);

        vec3 up = {.x = 0.0f, .y = 1.0f, .z = 0.0f};
        vec3 right = vec3Cross(forward, up);
        right = vec3Normalize(right);

        vec3 trueUp = vec3Cross(right, forward);
        trueUp = vec3Normalize(trueUp);

        glUseProgram(computeProgram);

        glUniform1i(glGetUniformLocation(computeProgram, "u_renderBothSides"), g_program.renderBothSides);
        glUniform1i(glGetUniformLocation(computeProgram, "u_debugMode"), g_program.debugmode);
        glUniform1i(glGetUniformLocation(computeProgram, "u_smoothShading"), g_program.smoothShading);
        glUniform1i(glGetUniformLocation(computeProgram, "u_isDay"), g_program.isDay);
        glUniform3f(glGetUniformLocation(computeProgram, "u_camForward"), forward.x, forward.y, forward.z);
        glUniform3f(glGetUniformLocation(computeProgram, "u_camRight"), right.x, right.y, right.z);
        glUniform3f(glGetUniformLocation(computeProgram, "u_camUp"), trueUp.x, trueUp.y, trueUp.z);
        glUniform2f(glGetUniformLocation(computeProgram, "u_resolution"), (float)g_program.newWidth, (float)g_program.newHeight);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_accumTexture, 0);

        glUniform1i(glGetUniformLocation(computeProgram, "u_frameCount"), g_program.frameCount);
        glUniform1i(glGetUniformLocation(computeProgram, "u_historyTexture"), 0);
        glUniform3f(glGetUniformLocation(computeProgram, "u_cameraPos"), g_program.camera.x, g_program.camera.y, g_program.camera.z);
        glUniform1f(glGetUniformLocation(computeProgram, "u_cameraYaw"), g_program.camera.yaw);
        glUniform1f(glGetUniformLocation(computeProgram, "u_cameraPitch"), g_program.camera.pitch);

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, g_accumTexture);
        glUniform1i(glGetUniformLocation(computeProgram, "u_historyTexture"), 0);

        // RT 
        glBindImageTexture(0, g_outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, g_normalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((g_program.newWidth + 15) / 16, (g_program.newHeight + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Denoiser
        glUseProgram(denoiseProgram);
        glUniform2f(glGetUniformLocation(denoiseProgram, "u_resolution"), (float)g_program.newWidth, (float)g_program.newHeight);

        GLuint readTex = g_outputTexture;
        GLuint writeTex = g_denoisedTexture;

        int denoisePasses = 0;

        if (g_program.adaptiveDenoising)
        {
            const int maxPasses = 10;

            float t = (float)g_program.frameCount / 3000.0f;

            if (t < 0.0f) {t = 0.0f;}
            if (t > 1.0f) {t = 1.0f;}

            float falloff = 1.0f - sqrt(t);

            denoisePasses = (int)roundf(maxPasses * falloff);
            if (denoisePasses < 3) {denoisePasses = 3;}
        }

        else {denoisePasses = 3;}

        if (g_program.enableDenoise)
        {
            for (int i = 0; i < denoisePasses; i++)
            {
                glUniform1i(glGetUniformLocation(denoiseProgram, "u_stepWidth"), 1 << i);

                glBindImageTexture(0, readTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                glBindImageTexture(1, g_normalTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                glBindImageTexture(2, writeTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

                glDispatchCompute((g_program.newWidth + 15) / 16, (g_program.newHeight + 15) / 16, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                readTex = writeTex;

                // Toggle write target texture
                writeTex = (writeTex == g_denoisedTexture) ? g_denoiseSwapTexture : g_denoisedTexture;
            }
        }

        glViewport(0, 0, g_program.newWidth, g_program.newHeight);
        glClear(GL_COLOR_BUFFER_BIT); // Clear default framebuffer

        glUseProgram(displayProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, readTex);
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
    glDeleteTextures(1, &g_denoisedTexture);
    glDeleteTextures(1, &g_denoiseSwapTexture);
    glDeleteVertexArrays(1, &vao);

    glFinish();
    glfwTerminate();
    return 0;
}