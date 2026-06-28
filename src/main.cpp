#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef bool (*GlossInit_t)(bool);
typedef int (*GlossHook_t)(void*, void*, void**);
typedef EGLBoolean (*eglSwapBuffers_t)(EGLDisplay, EGLSurface);

GlossInit_t pGlossInit = nullptr;
GlossHook_t pGlossHook = nullptr;
eglSwapBuffers_t pOrigSwapBuffers = nullptr;

static bool imguiInitialized = false;
static int frameCount = 0;

void initImGui(int w, int h) {
    if (imguiInitialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(w, h);
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 100");
    imguiInitialized = true;
    LOGI("ImGui initialized %dx%d", w, h);
}

EGLBoolean mySwapBuffers(EGLDisplay display, EGLSurface surface) {
    frameCount++;

    EGLint w, h;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    if (frameCount > 60 && !imguiInitialized) {
        initImGui(w, h);
    }

    if (imguiInitialized) {
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(w, h);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        float px = 100, py = 64, pz = -200;
        int day = 5;
        int hours = 14, minutes = 30;

        float scale = 3.5f;

        // Hitung total lebar teks buat centering
        char tmp[128];
        snprintf(tmp, sizeof(tmp),
            "XYZ: %.0f, %.0f, %.0f  |  DAY: %d  [%02d:%02d]",
            px, py, pz, day, hours, minutes);

        float textH = ImGui::CalcTextSize("A").y * scale;
        float barHeight = h * 0.085f;
        float survivalHeight = h * 0.085f; // naik lebih tinggi
        float posY = h - barHeight - survivalHeight - textH - 5;

        // Pake ImDrawList buat render teks dengan warna berbeda
        ImGui::SetNextWindowPos(ImVec2(0, posY), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowSize(ImVec2(w, textH + 20));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("##dc", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoDecoration);

        ImGui::SetWindowFontScale(scale);

        ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImVec4 white  = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        // Hitung total width buat centering
        float wXYZ    = ImGui::CalcTextSize("XYZ:").x;
        float wCoord  = ImGui::CalcTextSize(" 100, 64, -200  |  ").x;
        float wDAY    = ImGui::CalcTextSize("DAY:").x;
        float wDayVal = ImGui::CalcTextSize(" 5  ").x;
        float wTime   = ImGui::CalcTextSize("[14:30]").x;
        float totalW  = (wXYZ + wCoord + wDAY + wDayVal + wTime) * scale;

        float startX = (w - totalW) / 2.0f;
        ImGui::SetCursorPosX(startX);

        ImGui::TextColored(yellow, "XYZ:");
        ImGui::SameLine(0, 2);
        ImGui::TextColored(white, "%.0f, %.0f, %.0f  |  ", px, py, pz);
        ImGui::SameLine(0, 2);
        ImGui::TextColored(yellow, "DAY:");
        ImGui::SameLine(0, 2);
        ImGui::TextColored(white, "%d  ", day);
        ImGui::SameLine(0, 2);
        ImGui::TextColored(white, "[%02d:%02d]", hours, minutes);

        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
        ImGui::PopStyleVar(2);
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
    pGlossHook = (GlossHook_t)dlsym(preloader, "GlossHook");

    if (!pGlossInit) {
        LOGI("Failed get GlossInit!");
        return;
    }

    pGlossInit(false);

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    if (!egl) {
        LOGI("libEGL FAILED");
        return;
    }

    void* swapFunc = dlsym(egl, "eglSwapBuffers");
    if (pGlossHook && swapFunc) {
        pGlossHook(swapFunc, (void*)mySwapBuffers, (void**)&pOrigSwapBuffers);
        LOGI("Hook installed!");
    }

    LOGI("=== DayCounter LOADED ===");
}