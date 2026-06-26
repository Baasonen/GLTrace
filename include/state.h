#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "camera.h"

typedef struct 
{
    Camera camera;
    float cameraSpeed;
    bool cameraLock;

    int width;
    int height;
    int newWidth;
    int newHeight;
    bool framebufferResized;

    int frameCount;
    float deltaTime;
    float lastFrame;

    bool firstMouse;
    float lastX;
    float lastY;
    float mouseSensitivity;

    int timeOfDay;
    bool skyEnabled;
    bool enableDenoise;
    bool adaptiveDenoising;
    bool smoothShading;
    bool debugmode;
    bool renderBothSides;
    bool nee;
    float sunStrength;
} State;

extern State g_program;

#endif