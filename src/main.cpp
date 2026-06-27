#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <string>
#include <sstream>

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef bool (*GlossInit_t)(bool);
typedef void* (*pl_resolve_signature_t)(const char*, const char*);
typedef int (*GlossHookFunction_t)(void*, void*, void**);

// ImGui function types
typedef void* (*ImGui_GetIO_t)();
typedef void (*ImGui_Text_t)(const char*, ...);
typedef bool (*ImGui_Begin_t)(const char*, bool*, int);
typedef void (*ImGui_End_t)();
typedef void (*ImGui_SetNextWindowPos_t)(float, float, int, float, float);
typedef void (*ImGui_SetNextWindowBgAlpha_t)(float);

GlossInit_t pGlossInit = nullptr;
pl_resolve_signature_t pResolve = nullptr;

// MCBE time signature dari ThirdPersonNametag
const char* TIME_SIG = "? ? 40 F9 ? ? ? EB ? ? ? 54 ? ? 40 F9 ? 81 40 F9 E0 03 ? AA 00 01 3F D6 ? ? 00 37 ? ? 40 F9 ? ? ? A9 ? ? ? CB ? ? ? D3 ? ? 00 51 ? ? ? 8A";

void* pTimeFunc = nullptr;

// Hook callback
void* pOriginalTime = nullptr;

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

    if (!pGlossInit || !pResolve) {
        LOGI("Failed get functions!");
        return;
    }

    pGlossInit(false);

    // Resolve MCBE function
    pTimeFunc = pResolve("libminecraftpe.so", TIME_SIG);
    if (pTimeFunc) {
        LOGI("Found MCBE time function at: %p", pTimeFunc);
    } else {
        LOGI("MCBE time function not found!");
    }

    LOGI("=== DayCounter LOADED ===");
}
