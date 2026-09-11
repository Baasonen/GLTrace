#include "ui.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "state.h"
#include "shader.h"

void UIInit(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
}

void UINewFrame(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
}

static void drawRenderSettings(void)
{
    if (!ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {return;}
 
    bool changed = false;
 
    changed |= ImGui::Checkbox("Smooth shading", &g_program.smoothShading);
    changed |= ImGui::Checkbox("Render both sides", &g_program.renderBothSides);
    changed |= ImGui::Checkbox("NEE", &g_program.nee);
    changed |= ImGui::Checkbox("Sky enabled", &g_program.skyEnabled);
    changed |= ImGui::Checkbox("Debug mode (show normals)", &g_program.debugmode);
    changed |= ImGui::SliderFloat("Sun strength", &g_program.sunStrength, 0.0f, 100.0f);
 
    int tod = g_program.timeOfDay;
    if (ImGui::SliderInt("Time of day", &tod, 0, 1440))
    {
        g_program.timeOfDay = tod;
        changed = true;
    }
 
    if (changed) {g_program.frameCount = 0;}
 
    ImGui::Separator();
 
    ImGui::Checkbox("Denoise", &g_program.enableDenoise);
    ImGui::Checkbox("Adaptive denoising", &g_program.adaptiveDenoising);
    ImGui::Checkbox("Pre-denoise clamp", &g_program.preDenoise);
    ImGui::Checkbox("Print samples/s to console", &g_program.printFPS);
}

static void drawMaterialEditor(void)
{
    if (!ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {return;}
 
    if (g_matEditor.matCount <= 0)
    {
        ImGui::TextDisabled("No materials loaded yet.");
        return;
    }
 
    ImGui::SliderInt("Selected", &g_matEditor.selected, 0, g_matEditor.matCount - 1);
 
    Material* m = &g_matEditor.materials[g_matEditor.selected];
    bool changed = false;
 
    changed |= ImGui::ColorEdit3("Color", &m->cr);
    changed |= ImGui::SliderFloat("Roughness", &m->roughness, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Metallic", &m->metallic, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Emission", &m->emission, 0.0f, 20.0f);
    changed |= ImGui::SliderFloat("Opacity", &m->opacity, 0.0f, 1.0f);
    changed |= ImGui::SliderFloat("Visibility", &m->visibility, 0.0f, 1.0f);
 
    if (changed) {uploadMaterial(g_matEditor.selected);}
}
 
static void drawInfo(void)
{
    if (!ImGui::CollapsingHeader("Info")) {return;}
 
    ImGui::Text("Frame: %d", g_program.frameCount);
    ImGui::Text("Delta: %.2f ms", g_program.deltaTime * 1000.0f);
    ImGui::Text("Camera: %.2f, %.2f, %.2f", g_program.camera.x, g_program.camera.y, g_program.camera.z);
    ImGui::Text("Yaw/Pitch: %.1f / %.1f", g_program.camera.yaw, g_program.camera.pitch);
}

void UIDraw(void)
{
    ImGui::Begin("GLTrace");

    drawRenderSettings();
    drawMaterialEditor();
    drawInfo();

    ImGui::End();
}

void UIRender(void)
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIShutdown(void)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    
    ImGui::DestroyContext();
}