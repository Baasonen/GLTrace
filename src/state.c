#include "state.h"

#include "config.h"

State g_program = {
    .width = WINDOW_WIDTH,
    .height = WINDOW_HEIGHT,
    .newWidth = WINDOW_WIDTH,
    .newHeight = WINDOW_HEIGHT,
    .framebufferResized = false,
    
    .deltaTime = 0.0f,
    .lastFrame = 0.0f,
    .frameCount = 0,

    .samples = 1,
    .maxBounces = 7,

    .camera = {0.0f, 0.0f, 200.0f, -90.0f, 0.0f, 1.0f}, 
    .cameraSpeed = 100.0f,
    .cameraLock = true,

    .firstMouse = true,
    .lastX = WINDOW_WIDTH / 2.0f,
    .lastY = WINDOW_HEIGHT / 2.0f,
    .mouseSensitivity = 0.1f,

    .timeOfDay = 450,
    .skyEnabled = true,
    .enableDenoise = true,
    .adaptiveDenoising = false,
    .smoothShading = false,
    .debugmode = false,
    .renderBothSides = false,
    .nee = true,
    .sunStrength = 100.0f,
    .preDenoise = true,
    .showMenu = true,
};