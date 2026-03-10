#include "camera.h"

#include "math_utils.h"
#include "state.h"

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

    if (g_program.cameraLock) {return false;}

    bool moved = false;
    float cameraVelocity = g_program.cameraSpeed * g_program.deltaTime;

    float forwardX, forwardZ, rightX, rightZ;
    calculateCameraVectors(&g_program.camera, &forwardX, &forwardZ, &rightX, &rightZ);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_program.camera.x += forwardX * cameraVelocity;
        g_program.camera.z += forwardZ * cameraVelocity;
        moved = true;
    }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_program.camera.x -= forwardX * cameraVelocity;
        g_program.camera.z -= forwardZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_program.camera.x -= rightX * cameraVelocity;
        g_program.camera.z -= rightZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_program.camera.x += rightX * cameraVelocity;
        g_program.camera.z += rightZ * cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        g_program.camera.y += cameraVelocity;
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        g_program.camera.y -= cameraVelocity;
        moved = true;
    }

    return moved;
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (g_program.cameraLock) {return;}

    if (g_program.firstMouse)
    {
        g_program.lastX = (float)xpos;
        g_program.lastY = (float)ypos;
        g_program.firstMouse = false;
        return;
    }

    float xOffset = (float)xpos - g_program.lastX;
    float yOffset = g_program.lastY - (float)ypos;
    g_program.lastX = (float)xpos;
    g_program.lastY = (float)ypos;
    
    xOffset *= g_program.mouseSensitivity;
    yOffset *= g_program.mouseSensitivity;

    g_program.camera.yaw += xOffset;
    g_program.camera.pitch += yOffset;

    if (g_program.camera.pitch > 89.0f) {g_program.camera.pitch = 89.0f;}
    if (g_program.camera.pitch < -89.0f) {g_program.camera.pitch = -89.0f;}

    g_program.frameCount = 0;
}