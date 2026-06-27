#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

typedef bool (*GlossInit_t)(bool);

__attribute__((constructor))
void init() {
    LOGI("=== DayCounter LOADED ===");
    
    void* preloader = dlopen("libpreloader.so", RTLD_NOW);
    if (!preloader) {
        LOGI("preloader FAILED: %s", dlerror());
        return;
    }
    
    GlossInit_t GlossInit = (GlossInit_t)dlsym(preloader, "GlossInit");
    if (!GlossInit) {
        LOGI("GlossInit FAILED");
        return;
    }
    
    bool result = GlossInit(false);
    LOGI("GlossInit result: %d", result);
}
