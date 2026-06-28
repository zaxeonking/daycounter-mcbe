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
typedef int (*GlossHook_t)(void*, void*, void**);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);

// Type definitions untuk MCBE functions
typedef struct {
    float x, y, z;
} Vec3;

typedef int (*GetDayCount_t)(void*);
typedef int (*GetDayTime_t)(void*);
typedef Vec3 (*GetPlayerPos_t)(void*);

GlossInit_t pGlossInit = nullptr;
pl_resolve_signature_t pResolve = nullptr;
GlossHook_t pGlossHook = nullptr;
eglSwapBuffers_t pOrigSwapBuffers = nullptr;

GetDayCount_t pGetDayCount = nullptr;
GetDayTime_t pGetDayTime = nullptr;
GetPlayerPos_t pGetPlayerPos = nullptr;

static bool imguiInitialized = false;
static int screenW = 1080;
static int screenH = 1920;
static int frameCount = 0;

void* gLevel = nullptr;

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

void initMCBEFunctions() {
    void* mcbe = dlopen("libminecraftpe.so", RTLD_NOW);
    if (!mcbe) {
        LOGI("libminecraftpe open failed");
        return;
    }

    // Try cari fungsi pake nama (ini nama C++ yang di-mangle)
    pGetDayCount = (GetDayCount_t)dlsym(mcbe, "_ZN5Level9getDayCountEv");
    pGetDayTime = (GetDayTime_t)dlsym(mcbe, "_ZN5Level8getDayTimeEv");
    pGetPlayerPos = (GetPlayerPos_t)dlsym(mcbe, "_ZN11LocalPlayer6getPosEv");

    if (pGetDayCount) LOGI("getDayCount found!");
    if (pGetDayTime) LOGI("getDayTime found!");
    if (pGetPlayerPos) LOGI("getPlayerPos found!");
}

EGLBoolean mySwapBuffers(EGLDisplay display, EGLSurface surface) {
    frameCount++;

    EGLint w, h;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);
    screenW = w; screenH = h;

    if (frameCount > 60 && !imguiInitialized) {
        initImGui();
        initMCBEFunctions();
    }

    if (imguiInitialized) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(w, h);

        // Hanya render saat fullscreen (di world)
        bool isInWorld = (w >= 1080 && h >= 1920);

        if (isInWorld) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.7f);
            ImGui::Begin("##dc", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_AlwaysAutoResize);

            ImVec4 gold = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
            ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

            ImGui::SetWindowFontScale(2.0f);

            // Get data from MCBE
            int day = 1;
            int daytime = 6000;
            float px = 0, py = 64, pz = 0;

            if (pGetDayCount && gLevel) {
                day = pGetDayCount(gLevel);
            }

            if (pGetDayTime && gLevel) {
                daytime = pGetDayTime(gLevel);
            }

            // Convert daytime to hours:minutes
            int hours = (daytime / 1000) % 24;
            int minutes = ((daytime % 1000) / 1000) * 60;

            // Format text
            char dayText[32];
            char coordText[64];
            char timeText[32];

            snprintf(dayText, sizeof(dayText), "DAY %d", day);
            snprintf(coordText, sizeof(coordText), "X: %.0f Y: %.0f Z: %.0f", px, py, pz);
            snprintf(timeText, sizeof(timeText), "%02d:%02d", hours, minutes);

            ImGui::TextColored(gold, "%s", dayText);
            ImGui::TextColored(white, "%s", coordText);
            ImGui::TextColored(white, "%s", timeText);

            ImGui::SetWindowFontScale(1.0f);

            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
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
    pGlossHook = (GlossHook_t)dlsym(preloader, "GlossHook");

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
