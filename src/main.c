// Copyright (c) 2026 Henri Paasonen - GPLv2
// See LICENSE for details
#include <bvec/bvec.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "math_utils.h"
#include "file_util.h"
#include "shader_structs.h"
#include "shader.h"
#include "scene_builder.h"
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

int main(int argc, char* argv[])
{
    char scenePath[512];

    if (argc > 1)
    {
        snprintf(scenePath, sizeof(scenePath), "scenes/%s.scene", argv[1]);
    }
    else
    {
        strncpy(scenePath, "scenes/default.scene", sizeof(scenePath));
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

    setupSSBO();

    SceneDescription scene;

    if (!loadScene(scenePath, &scene))
    {
        fprintf(stderr, "Failed to load scene %s\n", scenePath);
        return 1;
    }

    setupSceneData(g_ssbo, &scene);

    GLuint computeProgram = createComputeProgram("shaders/raytrace.comp");
    GLuint displayProgram = createShaderProgram();
    GLuint denoiseProgram = createComputeProgram("shaders/denoise.comp");

    GLint loc_denoiseDir = glGetUniformLocation(denoiseProgram, "u_direction");
    GLint loc_denoiseStepWidth = glGetUniformLocation(denoiseProgram, "u_stepWidth");
    GLint loc_denoiseResolution = glGetUniformLocation(denoiseProgram, "u_resolution");

    GLint loc_renderBothSides = glGetUniformLocation(computeProgram, "u_renderBothSides");
    GLint loc_debugMode = glGetUniformLocation(computeProgram, "u_debugMode");
    GLint loc_smoothShading = glGetUniformLocation(computeProgram, "u_smoothShading");
    GLint loc_timeOfDay = glGetUniformLocation(computeProgram, "u_timeOfDay");
    GLint loc_camForward = glGetUniformLocation(computeProgram, "u_camForward");
    GLint loc_camRight = glGetUniformLocation(computeProgram, "u_camRight");
    GLint loc_camUp = glGetUniformLocation(computeProgram, "u_camUp");
    GLint loc_resolution = glGetUniformLocation(computeProgram, "u_resolution");
    GLint loc_frameCount = glGetUniformLocation(computeProgram, "u_frameCount");
    GLint loc_historyTexture = glGetUniformLocation(computeProgram, "u_historyTexture");
    GLint loc_cameraPos = glGetUniformLocation(computeProgram, "u_cameraPos");
    GLint loc_cameraYaw = glGetUniformLocation(computeProgram, "u_cameraYaw");
    GLint loc_cameraPitch = glGetUniformLocation(computeProgram, "u_cameraPitch");

    setupGpuTextures(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        g_program.deltaTime = currentFrame - g_program.lastFrame;
        g_program.lastFrame = currentFrame;

        //printf("%f, %f, %f\n", g_camera.x, g_camera.y, g_camera.z);

        if (g_program.framebufferResized)
        {
            setupGpuTextures(g_program.newWidth, g_program.newHeight);

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

        glUniform1i(loc_renderBothSides, g_program.renderBothSides);
        glUniform1i(loc_debugMode, g_program.debugmode);
        glUniform1i(loc_smoothShading, g_program.smoothShading);
        glUniform1i(loc_timeOfDay, g_program.timeOfDay);
        glUniform3f(loc_camForward, forward.x, forward.y, forward.z);
        glUniform3f(loc_camRight, right.x, right.y, right.z);
        glUniform3f(loc_camUp, trueUp.x, trueUp.y, trueUp.z);
        glUniform2f(loc_resolution, (float)g_program.newWidth, (float)g_program.newHeight);
        glUniform1i(loc_frameCount, g_program.frameCount);
        glUniform1i(loc_historyTexture, 0);
        glUniform3f(loc_cameraPos, g_program.camera.x, g_program.camera.y, g_program.camera.z);
        glUniform1f(loc_cameraYaw, g_program.camera.yaw);
        glUniform1f(loc_cameraPitch, g_program.camera.pitch);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_gpuTextures.accumTexture, 0);
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, g_gpuTextures.accumTexture);
        glUniform1i(glGetUniformLocation(computeProgram, "u_historyTexture"), 0);

        // RT 
        glBindImageTexture(0, g_gpuTextures.outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(1, g_gpuTextures.normalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute((g_program.newWidth + 15) / 16, (g_program.newHeight + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Denoiser
        glUseProgram(denoiseProgram);
        glUniform2f(loc_denoiseResolution, (float)g_program.newWidth, (float)g_program.newHeight);

        GLuint readTex = g_gpuTextures.outputTexture;
        GLuint writeTex = g_gpuTextures.denoisedTexture;

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
                int stepWidth = 1 << i;

                glUniform1i(loc_denoiseStepWidth, stepWidth);
                glUniform2i(loc_denoiseDir, 1, 0);

                // Horizontal pass
                glBindImageTexture(0, readTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                glBindImageTexture(1, g_gpuTextures.normalTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                glBindImageTexture(2, g_gpuTextures.denoiseHorizontalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                glDispatchCompute((g_program.newWidth + 15) / 16, (g_program.newHeight + 15) / 16, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                glUniform2i(loc_denoiseDir, 0, 1);
                glBindImageTexture(0, g_gpuTextures.denoiseHorizontalTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                glBindImageTexture(1, g_gpuTextures.normalTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
                glBindImageTexture(2, writeTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                glDispatchCompute((g_program.newWidth + 15) / 16, (g_program.newHeight + 15) / 16, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

                readTex = writeTex;

                // Toggle write target texture
                writeTex = (writeTex == g_gpuTextures.denoisedTexture) ? g_gpuTextures.denoiseSwapTexture : g_gpuTextures.denoisedTexture;
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

        GLuint temp = g_gpuTextures.accumTexture;
        g_gpuTextures.accumTexture = g_gpuTextures.outputTexture;
        g_gpuTextures.outputTexture = temp;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }   

    glDeleteBuffers(1, &g_ssbo.ssboBVH);
    glDeleteBuffers(1, &g_ssbo.ssboIndices);
    glDeleteBuffers(1, &g_ssbo.ssboVertices);
    glDeleteBuffers(1, &g_ssbo.ssboMaterials);
    glDeleteBuffers(1, &g_ssbo.ssboTriangleMaterial);

    glDeleteTextures(1, &g_gpuTextures.accumTexture);
    glDeleteTextures(1, &g_gpuTextures.outputTexture);
    glDeleteTextures(1, &g_gpuTextures.denoisedTexture);
    glDeleteTextures(1, &g_gpuTextures.denoiseSwapTexture);
    glDeleteTextures(1, &g_gpuTextures.denoiseHorizontalTexture);
    glDeleteVertexArrays(1, &vao);

    glFinish();
    glfwTerminate();
    return 0;
}