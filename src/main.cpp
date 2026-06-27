#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <string>
#include <sstream>

#define LOG_TAG "DayCounter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Offset MCBE 1.21.x untuk koordinat & waktu
// Nanti kita update offset yang tepat
static uintptr_t base = 0;

__attribute__((constructor))
void init() {
    LOGI("DayCounter .so loaded!");
    
    // Get base address MCBE
    void* handle = dlopen("libminecraftpe.so", RTLD_NOLOAD);
    if (handle) {
        LOGI("Got MCBE handle!");
    }
}
