#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <string>

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef bool (*GlossInit_t)(bool);
typedef void* (*pl_resolve_signature_t)(const char*, const char*);

GlossInit_t pGlossInit = nullptr;
pl_resolve_signature_t pResolve = nullptr;

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

    // Cari fungsi MCBE pake signature dari ThirdPersonNametag
    void* func = pResolve(
        "libminecraftpe.so",
        "? ? 40 F9 ? ? ? EB ? ? ? 54 ? ? 40 F9 ? 81 40 F9 E0 03 ? AA 00 01 3F D6 ? ? 00 37 ? ? 40 F9 ? ? ? A9 ? ? ? CB ? ? ? D3 ? ? 00 51 ? ? ? 8A"
    );

    if (func) {
        LOGI("Found MCBE function at: %p", func);
    } else {
        LOGI("Function not found!");
    }
}
