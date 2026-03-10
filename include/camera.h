#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <glfw/glfw3.h>

typedef struct 
{
    float x, y, z;
    float yaw, pitch;
    float focalLength;
} Camera;

void calculateCameraVectors(Camera* camera, float* forwardX, float* forwardZ, float* rightX, float* rightZ);

bool processInput(GLFWwindow* window);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

#endif