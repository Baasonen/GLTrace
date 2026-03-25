#include "input.h"

#include <stdio.h>

#include "state.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Day / Night
    if (key == GLFW_KEY_N && (action == GLFW_REPEAT | action == GLFW_PRESS))
    {
        printf("%i\n", g_program.timeOfDay);
        g_program.timeOfDay++;

        if (g_program.timeOfDay > 1440) {g_program.timeOfDay = 0;}

        g_program.frameCount = 0;
    }

    if (key == GLFW_KEY_M && (action == GLFW_REPEAT | action == GLFW_PRESS))
    {
        g_program.timeOfDay--;

        if (g_program.timeOfDay < 0) {g_program.timeOfDay = 1440;}

        g_program.frameCount = 0;
    }

    // Camera lock
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        g_program.cameraLock = !g_program.cameraLock;
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        g_program.enableDenoise = !g_program.enableDenoise;
    }

    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        g_program.smoothShading = !g_program.smoothShading;
        g_program.frameCount = 0;
        printf("Smooth shading: %s\n", g_program.smoothShading ? "Enabled" : "disabled");
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        g_program.debugmode = !g_program.debugmode;
        g_program.frameCount = 0;
    }

    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        g_program.renderBothSides = !g_program.renderBothSides;
        g_program.frameCount = 0;
        printf("Double sided rendering: %s\n", g_program.renderBothSides ? "Enabled" : "disabled");
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        g_program.adaptiveDenoising = !g_program.adaptiveDenoising;
        printf("Adaptive denoising: %s\n", g_program.adaptiveDenoising ? "Enabled" : "Disabled");
    }
}