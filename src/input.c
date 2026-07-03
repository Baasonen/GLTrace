#include "input.h"

#include <stdio.h>

#include "state.h"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_N && (action == GLFW_REPEAT || action == GLFW_PRESS))
    {
        g_program.timeOfDay++;

        if (g_program.timeOfDay > 1440) {g_program.timeOfDay = 0;}

        g_program.frameCount = 0;
    }

    if (key == GLFW_KEY_M && (action == GLFW_REPEAT || action == GLFW_PRESS))
    {
        g_program.timeOfDay--;

        if (g_program.timeOfDay < 0) {g_program.timeOfDay = 1440;}

        g_program.frameCount = 0;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        g_program.skyEnabled = !g_program.skyEnabled;

        g_program.frameCount = 0;
    }

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

    if (key == GLFW_KEY_8 && action == GLFW_PRESS)
    {
        g_program.nee = !g_program.nee;
        g_program.frameCount = 0;
        printf("NEE: %s\n", g_program.nee ? "Enabled" : "Disabled");
    }

    if (key == GLFW_KEY_9 && action == GLFW_PRESS)
    {
        g_program.sunStrength += 1.0f;
        g_program.frameCount = 0;
        printf("SunStrength: %f\n", g_program.sunStrength);
    }

    if (key == GLFW_KEY_7 && action == GLFW_PRESS)
    {
        g_program.sunStrength -= 1.0f;
        g_program.frameCount = 0;
        printf("SunStrength: %f\n", g_program.sunStrength);
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        g_program.printFPS = !g_program.printFPS;
    }

    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        g_program.preDenoise = !g_program.preDenoise;
        g_program.frameCount = 0;

        printf("Pre denoise: %s", g_program.preDenoise ? "Enabled" : "Disabled");
    }
}