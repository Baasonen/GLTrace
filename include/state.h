#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "camera.h"

#ifdef __cplusplus
extern "C" {
#endif

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

    int samples;
    int maxBounces;

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
    bool preDenoise;
    bool showMenu;
} State;

extern State g_program;

#ifdef __cplusplus
}
#endif

#endif