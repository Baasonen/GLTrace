#include <glad/glad.h>
#include <glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "camera.h"
#include "file_util.h"
#include "shader_structs.h"

#ifndef M_PI
#define M_PI 3.1415
#endif

float radians(float deg) {return deg * (M_PI / 180.0f);}

#define WIDTH 1280
#define HEIGHT 720

bool g_firstMouse = true;
float g_lastX = WIDTH / 2.0f;
float g_lastY = HEIGHT / 2.0f;
float g_mouseSensitivity = 0.1f;

int g_frameCount = 0;
Camera g_camera = {0.0f, 0.0f, 2.0f, -90.0f, 0.0f, 1.0f};
float g_cameraSpeed = 2.5f;

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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_camera.px += forwardX * cameraVelocity;
        g_camera.pz += forwardZ * cameraVelocity;
        moved = true;
    }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_camera.px -= forwardX * cameraVelocity;
        g_camera.pz -= forwardZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_camera.px += rightX * cameraVelocity;
        g_camera.pz += rightZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_camera.px -= rightX * cameraVelocity;
        g_camera.pz -= rightZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        g_camera.py += cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        g_camera.py -= cameraVelocity;
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
    return shader;
}

GLuint createShaderProgram()
{
    GLuint vertexShader = compileShader("shaders/fullscreen.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader("shaders/raytrace.frag", GL_FRAGMENT_SHADER);

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

// FBO
GLuint g_fbo;
GLuint g_accumTexture;
GLuint g_outputTexture;

// Init frame buffers
void setupAccumulationBuffers(int width, int height)
{
    glGenFramebuffers(1, &g_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);

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

    // Unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Material g_materials[] =
{
    // Matte white
    {1.0f, 1.0f, 1.0f, 0.0, 1.0f, 0.0f, 0.0f, 1.0f}
};

const int NUM_MATERIALS = sizeof(g_materials) / sizeof(Material);

void setupSceneData(GLuint sphereSSBO, GLuint materialSSBO, GLuint vertexSSBO, GLuint indexSSBO)
{
    // TODO: Load .obj files

    // Sphere setup

    Sphere scene[1];

    scene[0] = (Sphere){0.0, 0.0, 0.0, 1.0f, 1};

    // Sphere data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(scene), scene, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sphereSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(g_materials), g_materials, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialSSBO);
}

int main(int argc, char* argv[])
{
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
        glfwTerminate;
        return 1;
    }

    // Vsync
    glfwSwapInterval(1);

    // Buffer setup
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint ssboSpheres;
    GLuint ssboMaterials;
    GLuint ssboVertices;
    GLuint ssboIndices;

    glGenBuffers(1, &ssboSpheres);
    glGenBuffers(1, &ssboMaterials);
    glGenBuffers(1, &ssboVertices);
    glGenBuffers(1, &ssboIndices);

    
    GLuint program = createShaderProgram();
    glUseProgram(program);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        g_deltaTime = currentFrame - g_lastFrame;
        g_lastFrame = currentFrame;

        if (g_framebufferResized)
        {
            glDeleteTextures(1, &g_accumTexture);
            glDeleteTextures(1, &g_outputTexture);
            glDeleteFramebuffers(1, &g_fbo);

            setupAccumulationBuffers(g_newWidth, g_newHeight);

            g_frameCount = 0;
            g_framebufferResized = false;
        }

        bool cameraMoved = processInput(window);
        if (cameraMoved) {g_frameCount = 0;}
        g_frameCount++;

        glUniform2f(glGetUniformLocation(program, "u_resolution"), (float)g_newWidth, (float)g_newHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_accumTexture, 0);

        glUniform1i(glGetUniformLocation(program, "u_frameCount"), g_frameCount);
        glUniform1i(glGetUniformLocation(program, "u_historyTexture"), 0);
        glUniform3f(glGetUniformLocation(program, "u_cameraPos"), g_camera.px, g_camera.py, g_camera.pz);
        glUniform1f(glGetUniformLocation(program, "u_cameraYaw"), g_camera.yaw);
        glUniform1f(glGetUniformLocation(program, "u_cameraPitch"), g_camera.pitch);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_outputTexture);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        GLuint temp = g_accumTexture;
        g_accumTexture = g_outputTexture;
        g_outputTexture = temp;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_accumTexture);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}