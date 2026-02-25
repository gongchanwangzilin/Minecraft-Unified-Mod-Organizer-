/**
 * Minecraft Unifier - Android GUI Implementation
 * Android GUI实现 - 原生Android界面
 */

#include "android_gui.h"
#include <android/log.h>
#include <sstream>

#define LOG_TAG "MinecraftUnifier"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace mcu {
namespace gui {
namespace android {

// ==================== AndroidGUI ====================

AndroidGUI::AndroidGUI()
    : env_(nullptr)
    , context_(nullptr)
    , initialized_(false)
    , mainActivityClass_(nullptr)
    , modManagerActivityClass_(nullptr)
    , resourcePackManagerActivityClass_(nullptr)
    , settingsActivityClass_(nullptr)
    , aboutActivityClass_(nullptr)
    , showMainActivityMethod_(nullptr)
    , showModManagerActivityMethod_(nullptr)
    , showResourcePackManagerActivityMethod_(nullptr)
    , showSettingsActivityMethod_(nullptr)
    , showAboutActivityMethod_(nullptr) {
}

AndroidGUI::~AndroidGUI() {
    Shutdown();
}

bool AndroidGUI::Initialize(JNIEnv* env, jobject context) {
    if (initialized_) {
        return true;
    }
    
    env_ = env;
    context_ = env->NewGlobalRef(context);
    
    if (!InitializeJavaClasses()) {
        LOGE("Failed to initialize Java classes");
        return false;
    }
    
    initialized_ = true;
    LOGI("Android GUI initialized successfully");
    return true;
}

void AndroidGUI::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    if (context_) {
        env_->DeleteGlobalRef(context_);
        context_ = nullptr;
    }
    
    initialized_ = false;
    LOGI("Android GUI shutdown");
}

void AndroidGUI::ShowMainActivity() {
    if (!initialized_ || !showMainActivityMethod_) {
        return;
    }
    
    env_->CallVoidMethod(context_, showMainActivityMethod_);
}

void AndroidGUI::ShowModManagerActivity() {
    if (!initialized_ || !showModManagerActivityMethod_) {
        return;
    }
    
    env_->CallVoidMethod(context_, showModManagerActivityMethod_);
}

void AndroidGUI::ShowResourcePackManagerActivity() {
    if (!initialized_ || !showResourcePackManagerActivityMethod_) {
        return;
    }
    
    env_->CallVoidMethod(context_, showResourcePackManagerActivityMethod_);
}

void AndroidGUI::ShowSettingsActivity() {
    if (!initialized_ || !showSettingsActivityMethod_) {
        return;
    }
    
    env_->CallVoidMethod(context_, showSettingsActivityMethod_);
}

void AndroidGUI::ShowAboutActivity() {
    if (!initialized_ || !showAboutActivityMethod_) {
        return;
    }
    
    env_->CallVoidMethod(context_, showAboutActivityMethod_);
}

void AndroidGUI::AddMod(const ModInfo& mod) {
    mods_.push_back(mod);
    LOGI("Mod added: %s (%s)", mod.name.c_str(), mod.id.c_str());
}

void AndroidGUI::RemoveMod(const std::string& modId) {
    mods_.erase(
        std::remove_if(mods_.begin(), mods_.end(),
                      [&modId](const ModInfo& mod) {
                          return mod.id == modId;
                      }),
        mods_.end());
    LOGI("Mod removed: %s", modId.c_str());
}

void AndroidGUI::EnableMod(const std::string& modId) {
    for (auto& mod : mods_) {
        if (mod.id == modId) {
            mod.enabled = true;
            LOGI("Mod enabled: %s", modId.c_str());
            
            if (modToggleCallback_) {
                modToggleCallback_(modId, true);
            }
            break;
        }
    }
}

void AndroidGUI::DisableMod(const std::string& modId) {
    for (auto& mod : mods_) {
        if (mod.id == modId) {
            mod.enabled = false;
            LOGI("Mod disabled: %s", modId.c_str());
            
            if (modToggleCallback_) {
                modToggleCallback_(modId, false);
            }
            break;
        }
    }
}

const std::vector<ModInfo>& AndroidGUI::GetMods() const {
    return mods_;
}

void AndroidGUI::AddResourcePack(const ResourcePackInfo& pack) {
    resourcePacks_.push_back(pack);
    LOGI("Resource pack added: %s (%s)", pack.name.c_str(), pack.id.c_str());
}

void AndroidGUI::RemoveResourcePack(const std::string& packId) {
    resourcePacks_.erase(
        std::remove_if(resourcePacks_.begin(), resourcePacks_.end(),
                      [&packId](const ResourcePackInfo& pack) {
                          return pack.id == packId;
                      }),
        resourcePacks_.end());
    LOGI("Resource pack removed: %s", packId.c_str());
}

void AndroidGUI::EnableResourcePack(const std::string& packId) {
    for (auto& pack : resourcePacks_) {
        if (pack.id == packId) {
            pack.enabled = true;
            LOGI("Resource pack enabled: %s", packId.c_str());
            
            if (resourcePackToggleCallback_) {
                resourcePackToggleCallback_(packId, true);
            }
            break;
        }
    }
}

void AndroidGUI::DisableResourcePack(const std::string& packId) {
    for (auto& pack : resourcePacks_) {
        if (pack.id == packId) {
            pack.enabled = false;
            LOGI("Resource pack disabled: %s", packId.c_str());
            
            if (resourcePackToggleCallback_) {
                resourcePackToggleCallback_(packId, false);
            }
            break;
        }
    }
}

const std::vector<ResourcePackInfo>& AndroidGUI::GetResourcePacks() const {
    return resourcePacks_;
}

void AndroidGUI::AddLog(const std::string& message) {
    logs_.push_back(message);
    
    // 限制日志数量
    if (logs_.size() > 1000) {
        logs_.erase(logs_.begin());
    }
    
    LOGD("Log: %s", message.c_str());
}

void AndroidGUI::ClearLog() {
    logs_.clear();
    LOGI("Log cleared");
}

JNIEnv* AndroidGUI::GetJNIEnv() {
    return env_;
}

void AndroidGUI::SetModToggleCallback(ModToggleCallback callback) {
    modToggleCallback_ = callback;
}

void AndroidGUI::SetResourcePackToggleCallback(ResourcePackToggleCallback callback) {
    resourcePackToggleCallback_ = callback;
}

bool AndroidGUI::InitializeJavaClasses() {
    // 初始化MainActivity
    if (!InitializeMainActivity()) {
        LOGE("Failed to initialize MainActivity");
        return false;
    }
    
    // 初始化ModManagerActivity
    if (!InitializeModManagerActivity()) {
        LOGE("Failed to initialize ModManagerActivity");
        return false;
    }
    
    // 初始化ResourcePackManagerActivity
    if (!InitializeResourcePackManagerActivity()) {
        LOGE("Failed to initialize ResourcePackManagerActivity");
        return false;
    }
    
    // 初始化SettingsActivity
    if (!InitializeSettingsActivity()) {
        LOGE("Failed to initialize SettingsActivity");
        return false;
    }
    
    // 初始化AboutActivity
    if (!InitializeAboutActivity()) {
        LOGE("Failed to initialize AboutActivity");
        return false;
    }
    
    return true;
}

bool AndroidGUI::InitializeMainActivity() {
    jclass clazz = env_->FindClass("com/minecraftunifier/MainActivity");
    if (!clazz) {
        LOGE("Failed to find MainActivity class");
        return false;
    }
    
    mainActivityClass_ = reinterpret_cast<jclass>(env_->NewGlobalRef(clazz));
    
    showMainActivityMethod_ = env_->GetMethodID(mainActivityClass_, "show", "()V");
    if (!showMainActivityMethod_) {
        LOGE("Failed to find show method in MainActivity");
        return false;
    }
    
    return true;
}

bool AndroidGUI::InitializeModManagerActivity() {
    jclass clazz = env_->FindClass("com/minecraftunifier/ModManagerActivity");
    if (!clazz) {
        LOGE("Failed to find ModManagerActivity class");
        return false;
    }
    
    modManagerActivityClass_ = reinterpret_cast<jclass>(env_->NewGlobalRef(clazz));
    
    showModManagerActivityMethod_ = env_->GetMethodID(modManagerActivityClass_, "show", "()V");
    if (!showModManagerActivityMethod_) {
        LOGE("Failed to find show method in ModManagerActivity");
        return false;
    }
    
    return true;
}

bool AndroidGUI::InitializeResourcePackManagerActivity() {
    jclass clazz = env_->FindClass("com/minecraftunifier/ResourcePackManagerActivity");
    if (!clazz) {
        LOGE("Failed to find ResourcePackManagerActivity class");
        return false;
    }
    
    resourcePackManagerActivityClass_ = reinterpret_cast<jclass>(env_->NewGlobalRef(clazz));
    
    showResourcePackManagerActivityMethod_ = env_->GetMethodID(resourcePackManagerActivityClass_, "show", "()V");
    if (!showResourcePackManagerActivityMethod_) {
        LOGE("Failed to find show method in ResourcePackManagerActivity");
        return false;
    }
    
    return true;
}

bool AndroidGUI::InitializeSettingsActivity() {
    jclass clazz = env_->FindClass("com/minecraftunifier/SettingsActivity");
    if (!clazz) {
        LOGE("Failed to find SettingsActivity class");
        return false;
    }
    
    settingsActivityClass_ = reinterpret_cast<jclass>(env_->NewGlobalRef(clazz));
    
    showSettingsActivityMethod_ = env_->GetMethodID(settingsActivityClass_, "show", "()V");
    if (!showSettingsActivityMethod_) {
        LOGE("Failed to find show method in SettingsActivity");
        return false;
    }
    
    return true;
}

bool AndroidGUI::InitializeAboutActivity() {
    jclass clazz = env_->FindClass("com/minecraftunifier/AboutActivity");
    if (!clazz) {
        LOGE("Failed to find AboutActivity class");
        return false;
    }
    
    aboutActivityClass_ = reinterpret_cast<jclass>(env_->NewGlobalRef(clazz));
    
    showAboutActivityMethod_ = env_->GetMethodID(aboutActivityClass_, "show", "()V");
    if (!showAboutActivityMethod_) {
        LOGE("Failed to find show method in AboutActivity");
        return false;
    }
    
    return true;
}

// ==================== JNI接口 ====================

namespace jni {

static AndroidGUI* guiInstance = nullptr;

JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeInitialize(
    JNIEnv* env, jobject thiz, jobject context) {
    
    if (!guiInstance) {
        guiInstance = new AndroidGUI();
    }
    
    guiInstance->Initialize(env, context);
}

JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeShutdown(
    JNIEnv* env, jobject thiz) {
    
    if (guiInstance) {
        guiInstance->Shutdown();
        delete guiInstance;
        guiInstance = nullptr;
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_UnifierGUI_nativeGetMods(
    JNIEnv* env, jobject thiz) {
    
    if (!guiInstance) {
        return nullptr;
    }
    
    const std::vector<ModInfo>& mods = guiInstance->GetMods();
    
    jclass modInfoClass = env->FindClass("com/minecraftunifier/ModInfo");
    if (!modInfoClass) {
        return nullptr;
    }
    
    jmethodID constructor = env->GetMethodID(modInfoClass, "<init>", 
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ZZ)V");
    if (!constructor) {
        return nullptr;
    }
    
    jobjectArray array = env->NewObjectArray(mods.size(), modInfoClass, nullptr);
    
    for (size_t i = 0; i < mods.size(); i++) {
        const ModInfo& mod = mods[i];
        
        jstring id = env->NewStringUTF(mod.id.c_str());
        jstring name = env->NewStringUTF(mod.name.c_str());
        jstring version = env->NewStringUTF(mod.version.c_str());
        
        jobject modObj = env->NewObject(modInfoClass, constructor, 
            id, name, version, mod.enabled, mod.loaded);
        
        env->SetObjectArrayElement(array, i, modObj);
        
        env->DeleteLocalRef(id);
        env->DeleteLocalRef(name);
        env->DeleteLocalRef(version);
        env->DeleteLocalRef(modObj);
    }
    
    return array;
}

JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeToggleMod(
    JNIEnv* env, jobject thiz, jstring modId, jboolean enabled) {
    
    if (!guiInstance) {
        return;
    }
    
    const char* modIdStr = env->GetStringUTFChars(modId, nullptr);
    std::string modIdCpp(modIdStr);
    env->ReleaseStringUTFChars(modId, modIdStr);
    
    if (enabled) {
        guiInstance->EnableMod(modIdCpp);
    } else {
        guiInstance->DisableMod(modIdCpp);
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_UnifierGUI_nativeGetResourcePacks(
    JNIEnv* env, jobject thiz) {
    
    if (!guiInstance) {
        return nullptr;
    }
    
    const std::vector<ResourcePackInfo>& packs = guiInstance->GetResourcePacks();
    
    jclass packInfoClass = env->FindClass("com/minecraftunifier/ResourcePackInfo");
    if (!packInfoClass) {
        return nullptr;
    }
    
    jmethodID constructor = env->GetMethodID(packInfoClass, "<init>", 
        "(Ljava/lang/String;Ljava/lang/String;ZZ)V");
    if (!constructor) {
        return nullptr;
    }
    
    jobjectArray array = env->NewObjectArray(packs.size(), packInfoClass, nullptr);
    
    for (size_t i = 0; i < packs.size(); i++) {
        const ResourcePackInfo& pack = packs[i];
        
        jstring id = env->NewStringUTF(pack.id.c_str());
        jstring name = env->NewStringUTF(pack.name.c_str());
        
        jobject packObj = env->NewObject(packInfoClass, constructor, 
            id, name, pack.enabled, pack.applied);
        
        env->SetObjectArrayElement(array, i, packObj);
        
        env->DeleteLocalRef(id);
        env->DeleteLocalRef(name);
        env->DeleteLocalRef(packObj);
    }
    
    return array;
}

JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeToggleResourcePack(
    JNIEnv* env, jobject thiz, jstring packId, jboolean enabled) {
    
    if (!guiInstance) {
        return;
    }
    
    const char* packIdStr = env->GetStringUTFChars(packId, nullptr);
    std::string packIdCpp(packIdStr);
    env->ReleaseStringUTFChars(packId, packIdStr);
    
    if (enabled) {
        guiInstance->EnableResourcePack(packIdCpp);
    } else {
        guiInstance->DisableResourcePack(packIdCpp);
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_UnifierGUI_nativeGetLogs(
    JNIEnv* env, jobject thiz) {
    
    if (!guiInstance) {
        return nullptr;
    }
    
    const std::vector<std::string>& logs = guiInstance->GetLogs();
    
    jobjectArray array = env->NewObjectArray(logs.size(), 
        env->FindClass("java/lang/String"), nullptr);
    
    for (size_t i = 0; i < logs.size(); i++) {
        jstring log = env->NewStringUTF(logs[i].c_str());
        env->SetObjectArrayElement(array, i, log);
        env->DeleteLocalRef(log);
    }
    
    return array;
}

JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeClearLogs(
    JNIEnv* env, jobject thiz) {
    
    if (guiInstance) {
        guiInstance->ClearLog();
    }
}

JNIEXPORT void JNICALL Java_com_minecraftunifier_UnifierGUI_nativeAddLog(
    JNIEnv* env, jobject thiz, jstring message) {
    
    if (!guiInstance) {
        return;
    }
    
    const char* messageStr = env->GetStringUTFChars(message, nullptr);
    std::string messageCpp(messageStr);
    env->ReleaseStringUTFChars(message, messageStr);
    
    guiInstance->AddLog(messageCpp);
}

} // namespace jni

} // namespace android
} // namespace gui
} // namespace mcu
