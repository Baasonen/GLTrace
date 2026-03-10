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

    .camera = {0.0f, 0.0f, 200.0f, -90.0f, 0.0f, 1.0f}, 
    .cameraSpeed = 100.0f,
    .cameraLock = false,

    .firstMouse = true,
    .lastX = WINDOW_WIDTH / 2.0f,
    .lastY = WINDOW_HEIGHT / 2.0f,
    .mouseSensitivity = 0.1f,

    .isDay = 1,
    .enableDenoise = true,
    .adaptiveDenoising = false,
    .smoothShading = false,
    .debugmode = false,
    .renderBothSides = false
};