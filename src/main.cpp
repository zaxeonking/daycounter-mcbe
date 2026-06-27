#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>
#include <sstream>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef bool (*GlossInit_t)(bool);
typedef void* (*pl_resolve_signature_t)(const char*, const char*);
typedef int (*GlossHookFunction_t)(void*, void*, void**);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);

GlossInit_t pGlossInit = nullptr;
pl_resolve_signature_t pResolve = nullptr;
GlossHookFunction_t pGlossHook = nullptr;
eglSwapBuffers_t pOrigSwapBuffers = nullptr;

static bool imguiInitialized = false;
static int screenW = 1080;
static int screenH = 1920;

void initImGui() {
    if (imguiInitialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(screenW, screenH);
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 100");
    imguiInitialized = true;
    LOGI("ImGui initialized %dx%d", screenW, screenH);
}

EGLBoolean mySwapBuffers(EGLDisplay display, EGLSurface surface) {
    // Get screen size
    EGLint w, h;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);
    screenW = w; screenH = h;

    initImGui();

    // Render ImGui
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(w, h);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // DayCounter window
    ImVec4 goldColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
    ImVec4 whiteColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImGui::TextColored(goldColor, "DAY 1");
    ImGui::TextColored(whiteColor, "X: 0 Y: 64 Z: 0");
    ImGui::TextColored(whiteColor, "06:00");

    // Get day from MCBE (placeholder dulu)
    ImGui::TextColored
