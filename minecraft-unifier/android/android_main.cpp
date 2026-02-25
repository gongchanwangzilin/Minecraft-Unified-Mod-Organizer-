/**
 * Minecraft Unifier - Android Main Entry
 * Android主入口文件
 */

#include <android/log.h>
#include <android/native_activity.h>
#include <gui/android/android_gui.h>

#define LOG_TAG "MinecraftUnifier"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace mcu {
namespace gui {
namespace android {

// 全局GUI实例
static AndroidGUI* g_gui = nullptr;

// Activity回调
static void OnStart(ANativeActivity* activity) {
    LOGI("Activity started");
}

static void OnResume(ANativeActivity* activity) {
    LOGI("Activity resumed");
}

static void OnPause(ANativeActivity* activity) {
    LOGI("Activity paused");
}

static void OnStop(ANativeActivity* activity) {
    LOGI("Activity stopped");
}

static void OnDestroy(ANativeActivity* activity) {
    LOGI("Activity destroyed");
    
    if (g_gui) {
        g_gui->Shutdown();
        delete g_gui;
        g_gui = nullptr;
    }
}

static void OnWindowFocusChanged(ANativeActivity* activity, int focused) {
    LOGI("Window focus changed: %d", focused);
}

static void OnNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    LOGI("Native window created");
}

static void OnNativeWindowResized(ANativeActivity* activity, ANativeWindow* window) {
    LOGI("Native window resized");
}

static void OnNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window) {
    LOGI("Native window destroyed");
}

static void OnInputQueueCreated(ANativeActivity* activity, AInputQueue* queue) {
    LOGI("Input queue created");
}

static void OnInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue) {
    LOGI("Input queue destroyed");
}

static void OnConfigurationChanged(ANativeActivity* activity) {
    LOGI("Configuration changed");
}

static void OnLowMemory(ANativeActivity* activity) {
    LOGI("Low memory warning");
}

// ANativeActivity_onCreate入口点
extern "C" void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    LOGI("Minecraft Unifier Android Native Activity created");
    
    // 设置Activity回调
    activity->callbacks->onStart = OnStart;
    activity->callbacks->onResume = OnResume;
    activity->callbacks->onPause = OnPause;
    activity->callbacks->onStop = OnStop;
    activity->callbacks->onDestroy = OnDestroy;
    activity->callbacks->onWindowFocusChanged = OnWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = OnNativeWindowCreated;
    activity->callbacks->onNativeWindowResized = OnNativeWindowResized;
    activity->callbacks->onNativeWindowDestroyed = OnNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated = OnInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = OnInputQueueDestroyed;
    activity->callbacks->onConfigurationChanged = OnConfigurationChanged;
    activity->callbacks->onLowMemory = OnLowMemory;
    
    // 初始化GUI
    g_gui = new AndroidGUI();
    
    // 获取JNI环境
    JNIEnv* env = nullptr;
    activity->vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    
    if (env) {
        if (g_gui->Initialize(env, activity->clazz)) {
            LOGI("Android GUI initialized successfully");
            
            // 显示主界面
            g_gui->ShowMainActivity();
        } else {
            LOGE("Failed to initialize Android GUI");
        }
    } else {
        LOGE("Failed to get JNI environment");
    }
}

} // namespace android
} // namespace gui
} // namespace mcu
