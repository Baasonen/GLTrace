#ifndef UI_H
#define UI_H

#include <glad/glad.h>
#include <glfw/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

void UIInit(GLFWwindow* window);

void UINewFrame(void);

void UIDraw(void);

void UIRender(void);

void UIShutdown(void);

#ifdef __cplusplus
}
#endif

#endif