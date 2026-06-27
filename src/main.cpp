#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <string>

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

// Day counter variable
static int dayCount = 0;

// Our hooked eglSwapBuffers
EGLBoolean mySwapBuffers(EGLDisplay display, EGLSurface surface) {
    // Nanti kita tambah render ImGui di sini
    LOGI("Frame rendered! Day: %d", dayCount++);
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

    // Hook eglSwapBuffers
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
        pGlossHook(swapFunc, (void*)mySwapBuffers, (void**)&pOrigSwapBuffers);
        LOGI("eglSwapBuffers hooked!");
    }

    LOGI("=== DayCounter LOADED ===");
}
