#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// GlossHook functions dari libpreloader
typedef bool (*GlossInit_t)(bool);
typedef void* (*pl_resolve_signature_t)(const char*, const char*);

GlossInit_t GlossInit = nullptr;
pl_resolve_signature_t pl_resolve_signature = nullptr;

// Signature MCBE 1.21.x untuk getLevel time
// Format: wildcard pattern
const char* TIME_SIG = "? ? 40 F9 ? ? ? EB ? ? ? 54";

void* timeFunc = nullptr;

__attribute__((constructor))
void init() {
    LOGI("DayCounter loading...");

    // Load preloader
    void* preloader = dlopen("libpreloader.so", RTLD_NOW);
    if (!preloader) {
        LOGI("Failed to load libpreloader: %s", dlerror());
        return;
    }

    // Get functions
    GlossInit = (GlossInit_t)dlsym(preloader, "GlossInit");
    pl_resolve_signature = (pl_resolve_signature_t)dlsym(preloader, "pl_resolve_signature");

    if (!GlossInit || !pl_resolve_signature) {
        LOGI("Failed to get functions!");
        return;
    }

    // Init GlossHook
    GlossInit(false);
    LOGI("DayCounter loaded successfully!");
}
