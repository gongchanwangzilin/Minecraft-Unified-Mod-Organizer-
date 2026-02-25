/**
 * Minecraft Unifier - Android GUI
 * Android GUI - 原生Android界面
 */

#pragma once
#include <jni.h>
#include <android/log.h>
#include <string>
#include <vector>
#include <functional>

namespace mcu {
namespace gui {
namespace android {

// 模组信息
struct ModInfo {
    std::string id;
    std::string name;
    std::string version;
    bool enabled;
    bool loaded;
};

// 资源包信息
struct ResourcePackInfo {
    std::string id;
    std::string name;
    bool enabled;
    bool applied;
};

// Android GUI管理器
class AndroidGUI {
public:
    AndroidGUI();
    ~AndroidGUI();
    
    // 初始化
    bool Initialize(JNIEnv* env, jobject context);
    
    // 关闭
    void Shutdown();
    
    // 显示主界面
    void ShowMainActivity();
    
    // 显示模组管理界面
    void ShowModManagerActivity();
    
    // 显示资源包管理界面
    void ShowResourcePackManagerActivity();
    
    // 显示设置界面
    void ShowSettingsActivity();
    
    // 显示关于界面
    void ShowAboutActivity();
    
    // 模组管理
    void AddMod(const ModInfo& mod);
    void RemoveMod(const std::string& modId);
    void EnableMod(const std::string& modId);
    void DisableMod(const std::string& modId);
    const std::vector<ModInfo>& GetMods() const;
    
    // 资源包管理
    void AddResourcePack(const ResourcePackInfo& pack);
    void RemoveResourcePack(const std::string& packId);
    void EnableResourcePack(const std::string& packId);
    void DisableResourcePack(const std::string& packId);
    const std::vector<ResourcePackInfo>& GetResourcePacks() const;
    
    // 日志
    void AddLog(const std::string& message);
    void ClearLog();
    
    // 获取JNI环境
    JNIEnv* GetJNIEnv();
    
    // 事件回调
    using ModToggleCallback = std::function<void(const std::string&, bool)>;
    using ResourcePackToggleCallback = std::function<void(const std::string&, bool)>;
    void SetModToggleCallback(ModToggleCallback callback);
    void SetResourcePackToggleCallback(ResourcePackToggleCallback callback);
    
private:
    JNIEnv* env_;
    jobject context_;
    bool initialized_;
    
    // 模组列表
    std::vector<ModInfo> mods_;
    
    // 资源包列表
    std::vector<ResourcePackInfo> resourcePacks_;
    
    // 日志
    std::vector<std::string> logs_;
    
    // 回调
    ModToggleCallback modToggleCallback_;
    ResourcePackToggleCallback resourcePackToggleCallback_;
    
    // Java类和方法
    jclass mainActivityClass_;
    jclass modManagerActivityClass_;
    jclass resourcePackManagerActivityClass_;
    jclass settingsActivityClass_;
    jclass aboutActivityClass_;
    
    jmethodID showMainActivityMethod_;
    jmethodID showModManagerActivityMethod_;
    jmethodID showResourcePackManagerActivityMethod_;
    jmethodID showSettingsActivityMethod_;
    jmethodID showAboutActivityMethod_;
    
    // 初始化Java类和方法
    bool InitializeJavaClasses();
    bool InitializeMainActivity();
    bool InitializeModManagerActivity();
    bool InitializeResourcePackManagerActivity();
    bool InitializeSettingsActivity();
    bool InitializeAboutActivity();
};

// Android GUI JNI接口
namespace jni {
    // 初始化GUI
    JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeInitialize(
        JNIEnv* env, jobject thiz, jobject context);
    
    // 关闭GUI
    JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeShutdown(
        JNIEnv* env, jobject thiz);
    
    // 获取模组列表
    JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_UnifierGUI_nativeGetMods(
        JNIEnv* env, jobject thiz);
    
    // 启用/禁用模组
    JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeToggleMod(
        JNIEnv* env, jobject thiz, jstring modId, jboolean enabled);
    
    // 获取资源包列表
    JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_UnifierGUI_nativeGetResourcePacks(
        JNIEnv* env, jobject thiz);
    
    // 启用/禁用资源包
    JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeToggleResourcePack(
        JNIEnv* env, jobject thiz, jstring packId, jboolean enabled);
    
    // 获取日志
    JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_UnifierGUI_nativeGetLogs(
        JNIEnv* env, jobject thiz);
    
    // 清空日志
    JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeClearLogs(
        JNIEnv* env, jobject thiz);
    
    // 添加日志
    JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeAddLog(
        JNIEnv* env, jobject thiz, jstring message);
}

} // namespace android
} // namespace gui
} // namespace mcu
