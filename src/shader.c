#include "shader.h"

#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "file_util.h"

SSBOS g_ssbo = {0};

GPUTextures g_gpuTextures = {
            .accumTexture = 0,
            .outputTexture = 0,

            .normalTexture = 0,
            .denoisedTexture = 0,
            .denoiseSwapTexture = 0
};

void setupSSBO(void)
{
    glGenBuffers(1, &g_ssbo.ssboSpheres);
    glGenBuffers(1, &g_ssbo.ssboMaterials);
    glGenBuffers(1, &g_ssbo.ssboVertices);
    glGenBuffers(1, &g_ssbo.ssboIndices);
    glGenBuffers(1, &g_ssbo.ssboBVH);
    glGenBuffers(1, &g_ssbo.ssboTriangleMaterial);
}

void setupGpuTextures(int width, int height)
{
    if (g_gpuTextures.accumTexture) glDeleteTextures(1, &g_gpuTextures.accumTexture);
    if (g_gpuTextures.outputTexture) glDeleteTextures(1, &g_gpuTextures.outputTexture);

    // TX2
    glGenTextures(1, &g_gpuTextures.accumTexture);
    glBindTexture(GL_TEXTURE_2D, g_gpuTextures.accumTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // TX 2
    glGenTextures(1, &g_gpuTextures.outputTexture);
    glBindTexture(GL_TEXTURE_2D, g_gpuTextures.outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (g_gpuTextures.normalTexture) {glDeleteTextures(1, &g_gpuTextures.normalTexture);}
    if (g_gpuTextures.denoisedTexture) {glDeleteTextures(1, &g_gpuTextures.denoisedTexture);}
    if (g_gpuTextures.denoiseHorizontalTexture) {glDeleteTextures(1, &g_gpuTextures.denoiseHorizontalTexture);}

    glGenTextures(1, &g_gpuTextures.normalTexture);
    glBindTexture(GL_TEXTURE_2D, g_gpuTextures.normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &g_gpuTextures.denoisedTexture);
    glBindTexture(GL_TEXTURE_2D, g_gpuTextures.denoisedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &g_gpuTextures.denoiseSwapTexture);
    glBindTexture(GL_TEXTURE_2D, g_gpuTextures.denoiseSwapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &g_gpuTextures.denoiseHorizontalTexture);
    glBindTexture(GL_TEXTURE_2D, g_gpuTextures.denoiseHorizontalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);  
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

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_program.newWidth = width;
    g_program.newHeight = height;
    g_program.framebufferResized = true;
}