#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "shader_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    GLuint accumTexture;
    GLuint outputTexture;

    GLuint normalTexture;

    GLuint denoisedTexture;
    GLuint denoiseSwapTexture;
    GLuint denoiseHorizontalTexture;
    GLuint denoisePreTexture;
} GPUTextures;

typedef struct 
{
    GLuint ssboSpheres;
    GLuint ssboMaterials;
    GLuint ssboVertices;
    GLuint ssboIndices;
    GLuint ssboBVH;
    GLuint ssboTriangleMaterial;
} SSBOS;

extern SSBOS g_ssbo;
extern GPUTextures g_gpuTextures;

void setupSSBO(void);
void setupGpuTextures(int width, int height);

GLuint compileShader(const char* filename, GLenum type);

GLuint createShaderProgram();
GLuint createComputeProgram(const char* filename);

void framebufferSizeCallback(GLFWwindow* window, int width, int height);

typedef struct 
{
    Material* materials;
    
    int matCount;
    int selected;
} MaterialEditorState;

extern MaterialEditorState g_matEditor;

void uploadMaterial(int idx);

#ifdef __cplusplus
}
#endif

#endif