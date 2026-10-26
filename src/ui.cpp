#include "ui.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>

#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

#include "state.h"
#include "shader.h"
#include "plane_light.h"
#include "hdri.h"

namespace fs = std::filesystem;

static std::vector<std::string> hdriFiles;
static std::string hdriCurrent;
static bool hdriScanned = false;

static void scanHdri(void)
{
    hdriFiles.clear();

    std::error_code e;
    for (const auto& entry : fs::directory_iterator("HDRI", e))
    {
        if (!entry.is_regular_file()) {continue;}

        std::string extension = entry.path().extension().string();

        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {return (char)std::tolower(c);});
        
        if (extension == ".hdr") {hdriFiles.push_back(entry.path().stem().string());}
    }

    std::sort(hdriFiles.begin(), hdriFiles.end());
    hdriScanned = true;
}

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
    changed |= ImGui::SliderInt("Samples per Pixel", &g_program.samples, 1, 10);
    changed |= ImGui::SliderInt("Max Bounces", &g_program.maxBounces, 1, 10);
 
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
}

static void drawHdriSettings(void)
{
    if (!ImGui::CollapsingHeader("HDRI")) {return;}

    if (!hdriScanned) {scanHdri();}

    bool changed = false;

    const char* preview = hdriCurrent.empty() ? "None" : hdriCurrent.c_str();

    if (ImGui::BeginCombo("Environment", preview))
    {
        for (const std::string& name : hdriFiles)
        {
            bool isSelected = (name == hdriCurrent);

            if (ImGui::Selectable(name.c_str(), isSelected) && !isSelected)
            {
                char path[512];
                snprintf(path, sizeof(path), "HDRI/%s.hdr", name.c_str());

                if (hdriLoad(path))
                {
                    hdriCurrent = name;
                    g_program.hdriEnabled = true;
                    changed = true;
                }
            }

            if (isSelected) {ImGui::SetItemDefaultFocus();}
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Rescan")) {scanHdri();}

    if (!g_hdri.loaded)
    {
        ImGui::TextDisabled("No HDRI loaded");
        if (changed) {g_program.frameCount = 0;}
        return;
    }

    changed |= ImGui::Checkbox("HDRI enabled", &g_program.hdriEnabled);
    changed |= ImGui::SliderFloat("HDRI strength", &g_program.hdriStrength, 0.0f, 10.0f);
    changed |= ImGui::SliderFloat("HDRI rotation", &g_program.hdriRotation, 0.0f, 360.0f);

    if (changed) {g_program.frameCount = 0;}
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

static void drawSphereEditor(void)
{
    if (!ImGui::CollapsingHeader("Spheres")) {return;}

    if (g_sphereEditor.sphereCount <= 0)
    {
        ImGui::TextDisabled("No spheres in scene.");
        return;
    }

    ImGui::SliderInt("Sphere", &g_sphereEditor.selected, 0, g_sphereEditor.sphereCount - 1);

    Sphere* s = &g_sphereEditor.spheres[g_sphereEditor.selected];
    bool changed = false;

    changed |= ImGui::DragFloat3("Position", &s->px, 0.5f);

    float diameter = s->radius * 2.0f;
    if (ImGui::DragFloat("Diameter", &diameter, 0.5f, 0.01f, 10000.0f))
    {
        s->radius = diameter * 0.5f;
        changed = true;
    }

    if (changed) {uploadSphere(g_sphereEditor.selected);}
}
 
static void drawPlaneLightEditor(void)
{
    if (!ImGui::CollapsingHeader("Plane Lights")) {return;}

    PLSystem* sys = &g_planeLights;

    if (ImGui::Button("Add"))
    {
        plAdd(vec3Make(g_program.camera.x, g_program.camera.y + 20.0f, g_program.camera.z));
    }
 
    ImGui::SameLine();
 
    if (ImGui::Button("Remove") && sys->count > 0)
    {
        plRemove(sys->selected);
        sys->needSelect = false;
    }
 
    if (sys->count <= 0)
    {
        ImGui::TextDisabled("No plane lights in scene.");
        return;
    }
 
    ImGui::SliderInt("Light", &sys->selected, 0, sys->count - 1);
 
    PlaneLight* l = &sys->lights[sys->selected];
    bool changed = false;
 
    changed |= ImGui::DragFloat3("Position##pl", &l->px, 0.5f);
    changed |= ImGui::ColorEdit3("Color##pl", &l->cr);
    changed |= ImGui::DragFloat("Intensity##pl", &l->intensity, 0.1f, 0.0f, 10000.0f);
    changed |= ImGui::DragFloat("Width##pl", &l->width, 0.5f, 0.01f, 10000.0f);
    changed |= ImGui::DragFloat("Height##pl", &l->height, 0.5f, 0.01f, 10000.0f);
 
    if (changed) {plUpload();}
 
    ImGui::Text("Facing: %.2f, %.2f, %.2f", l->nx, l->ny, l->nz);
 
    if (ImGui::Button("Flip")) {plFlip(sys->selected);}
 
    ImGui::SameLine();
 
    if (ImGui::Button(sys->needSelect ? "Click a surface... (cancel)" : "Aim at clicked point"))
    {
        sys->needSelect = !sys->needSelect;
    }
 
    ImGui::TextDisabled("Middle-click a light to select it.");
}

static void drawInfo(void)
{
    static float deltaHistory[120] = {0.0f};
    static int deltaOffset = 0;

    static float fpsTimer = 0.0f;
    static int fpsFrameCount = 0;
    static float avgFps = 0.0f;

    deltaHistory[deltaOffset] = g_program.deltaTime * 1000.0f;
    deltaOffset = (deltaOffset + 1) % IM_ARRAYSIZE(deltaHistory);

    fpsTimer += g_program.deltaTime;
    fpsFrameCount++;
    if (fpsTimer >= 1.0f)
    {
        avgFps = (float)fpsFrameCount / fpsTimer;
        fpsTimer = 0.0f;
        fpsFrameCount = 0;
    }

    if (!ImGui::CollapsingHeader("Info", ImGuiTreeNodeFlags_DefaultOpen)) { return; }

    if (ImGui::Button("Reset Camera"))
    {
        g_program.camera.x = 0.0f;
        g_program.camera.y = 0.0f;
        g_program.camera.z = 200.0f;
        g_program.camera.yaw = -90.0f;
        g_program.camera.pitch = 0.0f;

        g_program.frameCount = 0;
    }

    ImGui::Text("Frame: %d", g_program.frameCount);
    ImGui::Text("FPS: %.1f", avgFps);
    ImGui::Text("Camera: %.2f, %.2f, %.2f", g_program.camera.x, g_program.camera.y, g_program.camera.z);
    ImGui::Text("Yaw/Pitch: %.1f / %.1f", g_program.camera.yaw, g_program.camera.pitch);

    float avg = 0.0f;
    for (int i = 0; i < IM_ARRAYSIZE(deltaHistory); ++i) {avg += deltaHistory[i];}
    avg /= (float)IM_ARRAYSIZE(deltaHistory);

    char overlay[32];
    snprintf(overlay, sizeof(overlay), "avg %.2f ms", avg);

    ImGui::PlotLines("##delta", deltaHistory, IM_ARRAYSIZE(deltaHistory), deltaOffset,
                     overlay, 0.0f, FLT_MAX, ImVec2(0.0f, 60.0f));
}

void UIDraw(void)
{
    ImGui::Begin("GLTrace");

    drawRenderSettings();
    drawHdriSettings();
    drawMaterialEditor();
    drawSphereEditor();
    drawPlaneLightEditor();

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