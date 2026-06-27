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
static int frameCount = 0;

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
    frameCount++;

    EGLint w, h;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);
    screenW = w; screenH = h;

    if (frameCount > 60 && !imguiInitialized) {
        initImGui();
    }

    if (imguiInitialized) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(w, h);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("##dc", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize);

        ImVec4 gold = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
        ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        ImGui::TextColored(gold, "DAY 1");
        ImGui::TextColored(white, "X: 0 Y: 64 Z: 0");
        ImGui::TextColored(white, "06:00");

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return pOrigSwapBuffers(display, surface);
}

__attribute__((constructor))
void init() {
    LOGI("=== DayCounter LOADING ===");

    void* preloader = dlopen("libpreloader.so", RTLD_NOW);
    if (!preloader) {
        LOGI("preloader FAILED: %s", dlerror());
        return;
    }

    pGlossInit = (GlossInit_t)dlsym(preloader, "GlossInit");
    pResolve = (pl_resolve_signature_t)dlsym(preloader, "pl_resolve_signature");
    pGlossHook = (GlossHookFunction_t)dlsym(preloader, "GlossHookFunction");

    if (!pGlossInit) {
        LOGI("Failed get GlossInit!");
        return;
    }

    pGlossInit(false);

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    if (!egl) {
        LOGI("libEGL FAILED: %s", dlerror());
        return;
    }

    void* swapFunc = dlsym(egl, "eglSwapBuffers");
    if (!swapFunc) {
        LOGI("eglSwapBuffers not found!");
        return;
    }

    if (pGlossHook) {
        int result = pGlossHook(swapFunc, (void*)mySwapBuffers, (void**)&pOrigSwapBuffers);
        LOGI("Hook result: %d", result);
    }

    LOGI("=== DayCounter LOADED ===");
}
