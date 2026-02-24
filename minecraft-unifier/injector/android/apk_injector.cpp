/**
 * Minecraft Unifier - APK Injector Implementation (Android)
 * APK注入器实现 - Android平台
 */

#include "apk_injector.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>

namespace fs = std::filesystem;

namespace mcu {
namespace injector {
namespace android {

// ==================== APKModifier ====================

APKModifier::APKModifier() {
}

APKModifier::~APKModifier() {
}

void APKModifier::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool APKModifier::Decompile(const std::string& apkPath, const std::string& outputDir) {
    if (progressCallback_) {
        progressCallback_(10, "开始解包APK...");
    }
    
    // 检查apktool是否安装
    if (!CheckToolInstalled("apktool")) {
        if (progressCallback_) {
            progressCallback_(0, "apktool未安装");
        }
        return false;
    }
    
    // 使用apktool解包
    std::string cmd = "apktool d \"" + apkPath + "\" -o \"" + outputDir + "\" -f";
    if (!ExecuteCommand(cmd)) {
        if (progressCallback_) {
            progressCallback_(10, "APK解包失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(30, "APK解包完成");
    }
    
    return true;
}

bool APKModifier::ModifyManifest(const std::string& workDir) {
    if (progressCallback_) {
        progressCallback_(35, "修改AndroidManifest.xml...");
    }
    
    std::string manifestPath = workDir + "/AndroidManifest.xml";
    
    // 检查文件是否存在
    if (!fs::exists(manifestPath)) {
        if (progressCallback_) {
            progressCallback_(35, "AndroidManifest.xml不存在");
        }
        return false;
    }
    
    // 读取manifest
    std::ifstream file(manifestPath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // 添加必要的权限
    if (content.find("android.permission.WRITE_EXTERNAL_STORAGE") == std::string::npos) {
        content = std::regex_replace(content,
            std::regex("<manifest"),
            "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n"
            "    android:sharedUserId=\"android.uid.system\"");
    }
    
    // 添加meta-data标记
    if (content.find("unifier.enabled") == std::string::npos) {
        content = std::regex_replace(content,
            std::regex("<application"),
            "<application\n"
            "        android:name=\".UnifierApplication\"\n"
            "        android:extractNativeLibs=\"true\"");
        
        // 在application标签内添加meta-data
        content = std::regex_replace(content,
            std::regex("</application>"),
            "        <meta-data\n"
            "            android:name=\"unifier.enabled\"\n"
            "            android:value=\"true\" />\n"
            "    </application>");
    }
    
    // 写回文件
    std::ofstream outFile(manifestPath);
    outFile << content;
    outFile.close();
    
    if (progressCallback_) {
        progressCallback_(40, "AndroidManifest.xml修改完成");
    }
    
    return true;
}

bool APKModifier::ModifySmali(const std::string& workDir) {
    if (progressCallback_) {
        progressCallback_(45, "修改smali代码...");
    }
    
    // 查找MainActivity
    std::string mainActivity = FindMainActivity(workDir);
    if (mainActivity.empty()) {
        if (progressCallback_) {
            progressCallback_(45, "未找到MainActivity");
        }
        return false;
    }
    
    // 转换为smali路径
    std::string smaliPath = workDir + "/smali/" + 
                           std::regex_replace(mainActivity, std::regex("\\."), "/");
    smaliPath += ".smali";
    
    // 检查文件是否存在
    if (!fs::exists(smaliPath)) {
        // 尝试其他可能的路径
        smaliPath = workDir + "/smali_classes2/" + 
                   std::regex_replace(mainActivity, std::regex("\\."), "/");
        smaliPath += ".smali";
    }
    
    if (!fs::exists(smaliPath)) {
        if (progressCallback_) {
            progressCallback_(45, "MainActivity.smali不存在");
        }
        return false;
    }
    
    // 读取smali文件
    std::ifstream file(smaliPath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // 在onCreate方法开头插入加载代码
    std::string insertCode = 
        "    const-string v0, \"MinecraftUnifier\"\n"
        "    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V\n";
    
    content = std::regex_replace(content,
        std::regex("\\.method protected onCreate\\(Landroid/os/Bundle;\\)V"),
        ".method protected onCreate(Landroid/os/Bundle;)V\n" + insertCode);
    
    // 写回文件
    std::ofstream outFile(smaliPath);
    outFile << content;
    outFile.close();
    
    if (progressCallback_) {
        progressCallback_(50, "smali代码修改完成");
    }
    
    return true;
}

bool APKModifier::AddNativeLibs(const std::string& workDir, const std::vector<std::string>& libPaths) {
    if (progressCallback_) {
        progressCallback_(55, "添加native库...");
    }
    
    // 支持的架构
    std::vector<std::string> architectures = {
        "armeabi-v7a", "arm64-v8a", "x86", "x86_64"
    };
    
    for (const auto& arch : architectures) {
        std::string libDir = workDir + "/lib/" + arch;
        fs::create_directories(libDir);
        
        // 复制库文件
        for (const auto& libPath : libPaths) {
            fs::path libFile(libPath);
            std::string targetPath = libDir + "/" + libFile.filename().string();
            
            // 检查源文件是否存在
            if (fs::exists(libPath)) {
                fs::copy_file(libPath, targetPath, 
                             fs::copy_options::overwrite_existing);
            }
        }
    }
    
    if (progressCallback_) {
        progressCallback_(60, "native库添加完成");
    }
    
    return true;
}

bool APKModifier::Repack(const std::string& workDir, const std::string& outputApk) {
    if (progressCallback_) {
        progressCallback_(70, "重新打包APK...");
    }
    
    // 使用apktool重新打包
    std::string cmd = "apktool b \"" + workDir + "\" -o \"" + outputApk + "\"";
    if (!ExecuteCommand(cmd)) {
        if (progressCallback_) {
            progressCallback_(70, "APK重新打包失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(80, "APK重新打包完成");
    }
    
    return true;
}

bool APKModifier::SignApk(const std::string& unsignedApk, const std::string& signedApk,
                         const std::string& keystorePath, const std::string& keystorePass) {
    if (progressCallback_) {
        progressCallback_(85, "签名APK...");
    }
    
    // 检查apksigner是否安装
    if (!CheckToolInstalled("apksigner")) {
        if (progressCallback_) {
            progressCallback_(85, "apksigner未安装");
        }
        return false;
    }
    
    // 使用apksigner签名
    std::string cmd = "apksigner sign --ks \"" + keystorePath + "\" --ks-pass pass:\"" + 
                     keystorePass + "\" --ks-key-alias unifier --key-pass pass:\"" + 
                     keystorePass + "\" --out \"" + signedApk + "\" \"" + unsignedApk + "\"";
    
    if (!ExecuteCommand(cmd)) {
        if (progressCallback_) {
            progressCallback_(85, "APK签名失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(90, "APK签名完成");
    }
    
    return true;
}

bool APKModifier::Inject(const std::string& inputApk, const std::string& outputApk,
                        const std::vector<std::string>& nativeLibs) {
    // 创建临时工作目录
    std::string workDir = fs::temp_directory_path().string() + "/apk_inject_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    
    try {
        // 解包APK
        if (!Decompile(inputApk, workDir)) {
            fs::remove_all(workDir);
            return false;
        }
        
        // 修改manifest
        if (!ModifyManifest(workDir)) {
            fs::remove_all(workDir);
            return false;
        }
        
        // 修改smali
        if (!ModifySmali(workDir)) {
            fs::remove_all(workDir);
            return false;
        }
        
        // 添加native库
        if (!AddNativeLibs(workDir, nativeLibs)) {
            fs::remove_all(workDir);
            return false;
        }
        
        // 重新打包
        std::string unsignedApk = outputApk + ".unsigned";
        if (!Repack(workDir, unsignedApk)) {
            fs::remove_all(workDir);
            return false;
        }
        
        // 签名APK
        std::string keystorePath = "./unifier.keystore";
        std::string keystorePass = "unifier123";
        
        // 如果keystore不存在，创建一个
        if (!fs::exists(keystorePath)) {
            std::string createKeyCmd = "keytool -genkey -v -keystore \"" + keystorePath + 
                                      "\" -alias unifier -keyalg RSA -keysize 2048 -validity 10000 " +
                                      "-storepass \"" + keystorePass + "\" -keypass \"" + keystorePass + "\" " +
                                      "-dname \"CN=Unifier, OU=Dev, O=Unifier, L=City, ST=State, C=US\"";
            ExecuteCommand(createKeyCmd);
        }
        
        if (!SignApk(unsignedApk, outputApk, keystorePath, keystorePass)) {
            fs::remove_all(workDir);
            fs::remove(unsignedApk);
            return false;
        }
        
        // 清理临时文件
        fs::remove_all(workDir);
        fs::remove(unsignedApk);
        
        if (progressCallback_) {
            progressCallback_(100, "注入完成！");
        }
        
        return true;
    } catch (const std::exception& e) {
        fs::remove_all(workDir);
        return false;
    }
}

bool APKModifier::ExecuteCommand(const std::string& cmd) {
    return system(cmd.c_str()) == 0;
}

bool APKModifier::CheckToolInstalled(const std::string& tool) {
    std::string cmd = "which " + tool + " > /dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

std::string APKModifier::FindMainActivity(const std::string& workDir) {
    std::string manifestPath = workDir + "/AndroidManifest.xml";
    
    std::ifstream file(manifestPath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // 查找MAIN action
    std::regex mainActivityRegex("android:name=\"([^\"]+)\"[^>]*>\\s*<intent-filter>\\s*<action android:name=\"android.intent.action.MAIN\"");
    std::smatch match;
    
    if (std::regex_search(content, match, mainActivityRegex)) {
        return match[1].str();
    }
    
    return "";
}

// ==================== SoLoader ====================

SoLoader::SoLoader() {
}

SoLoader::~SoLoader() {
}

void SoLoader::SetHookConfig(const std::unordered_map<std::string, std::string>& hooks) {
    hookConfig_ = hooks;
}

void SoLoader::SetCompatModules(const std::vector<std::string>& modules) {
    compatModules_ = modules;
}

std::string SoLoader::GenerateLoaderCode() {
    std::stringstream code;
    
    code << R"(/**
 * Minecraft Unifier - Native Loader
 * 原生加载器 - 自动生成
 */

#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <string>
#include <vector>

#define LOG_TAG "MinecraftUnifier"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 全局变量
static JavaVM* g_vm = nullptr;
static jobject g_context = nullptr;
static std::vector<void*> g_loadedModules;

// JNI_OnLoad - 在so加载时自动调用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("MinecraftUnifier loader loaded");
    g_vm = vm;
    
    // 创建初始化线程
    pthread_t tid;
    pthread_create(&tid, nullptr, InitThread, vm);
    pthread_detach(tid);
    
    return JNI_VERSION_1_6;
}

// 初始化线程
void* InitThread(void* arg) {
    JavaVM* vm = static_cast<JavaVM*>(arg);
    JNIEnv* env = nullptr;
    vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    
    // 等待libminecraftpe.so加载
    void* handle = nullptr;
    int retryCount = 0;
    while (!handle && retryCount < 60) {
        handle = dlopen("libminecraftpe.so", RTLD_LAZY);
        if (!handle) {
            sleep(1);
            retryCount++;
        }
    }
    
    if (!handle) {
        LOGE("Failed to load libminecraftpe.so");
        return nullptr;
    }
    
    LOGI("libminecraftpe.so loaded at %p", handle);
    
    // 获取应用上下文
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    jmethodID currentApp = env->GetStaticMethodID(activityThread, "currentApplication",
                                                   "()Landroid/app/Application;");
    jobject app = env->CallStaticObjectMethod(activityThread, currentApp);
    g_context = env->NewGlobalRef(app);
    
    // 获取应用私有目录
    jclass context = env->FindClass("android/content/Context");
    jmethodID getFilesDir = env->GetMethodID(context, "getFilesDir", "()Ljava/io/File;");
    jobject filesDir = env->CallObjectMethod(app, getFilesDir);
    
    jclass file = env->FindClass("java/io/File");
    jmethodID getPath = env->GetMethodID(file, "getPath", "()Ljava/lang/String;");
    jstring pathStr = static_cast<jstring>(env->CallObjectMethod(filesDir, getPath));
    
    const char* filesPath = env->GetStringUTFChars(pathStr, nullptr);
    
    // 构建配置路径
    char configPath[256];
    snprintf(configPath, sizeof(configPath), "%s/unifier_config.json", filesPath);
    
    env->ReleaseStringUTFChars(pathStr, filesPath);
    
    LOGI("Config path: %s", configPath);
    
    // 加载配置
    LoadConfig(configPath);
    
    // 加载兼容模块
)";

    // 添加兼容模块加载代码
    for (const auto& module : compatModules_) {
        code << "    void* " + module + " = dlopen(\"" + module + ".so\", RTLD_NOW);\n";
        code << "    if (" + module + ") {\n";
        code << "        LOGI(\"Loaded module: " + module + "\");\n";
        code << "        g_loadedModules.push_back(" + module + ");\n";
        code << "        // 调用初始化函数\n";
        code << "        typedef void (*init_func_t)(JNIEnv*, jobject);\n";
        code << "        init_func_t init = reinterpret_cast<init_func_t>(dlopen(" + module + ", \"Initialize\"));\n";
        code << "        if (init) init(env, g_context);\n";
        code << "    } else {\n";
        code << "        LOGE(\"Failed to load module: " + module + "\");\n";
        code << "    }\n";
    }

    code << R"(
    // 安装Hook
    InstallHooks(handle);
    
    LOGI("MinecraftUnifier initialization complete");
    return nullptr;
}

// 加载配置
void LoadConfig(const char* configPath) {
    // TODO: 实现配置文件解析
    LOGI("Loading config from: %s", configPath);
}

// 安装Hook
void InstallHooks(void* libHandle) {
    // TODO: 使用xHook或其他框架安装Hook
    LOGI("Installing hooks...");
    
    // 示例：Hook fopen
    // xhook_register(".*libminecraftpe\\.so$", "fopen", 
    //                reinterpret_cast<void*>(hooked_fopen), 
    //                reinterpret_cast<void**>(&original_fopen));
    // xhook_refresh(0);
    
    LOGI("Hooks installed");
}

// Hook函数示例
FILE* (*original_fopen)(const char* path, const char* mode) = nullptr;
FILE* hooked_fopen(const char* path, const char* mode) {
    LOGI("fopen called: %s", path);
    
    // 检查是否需要重定向
    if (strstr(path, "shaderpacks") != nullptr) {
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "/sdcard/Unifier/shaderpacks/%s",
                 strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
        FILE* f = original_fopen(newPath, mode);
        if (f) {
            LOGI("Redirected to: %s", newPath);
            return f;
        }
    }
    
    return original_fopen(path, mode);
}
)";
    
    return code.str();
}

bool SoLoader::CreateLoaderSo(const std::string& outputPath) {
    // 生成C++代码
    std::string code = GenerateLoaderCode();
    
    // 保存到临时文件
    std::string sourcePath = outputPath + ".cpp";
    std::ofstream sourceFile(sourcePath);
    sourceFile << code;
    sourceFile.close();
    
    // 编译为so库
    std::string compileCmd = "cd " + fs::path(outputPath).parent_path().string() + " && " +
                            "$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang++ " +
                            "-shared -fPIC " + sourcePath + " -o " + outputPath + " " +
                            "-landroid -llog";
    
    return system(compileCmd.c_str()) == 0;
}

// ==================== AndroidHookFramework ====================

AndroidHookFramework::AndroidHookFramework()
    : initialized_(false) {
}

AndroidHookFramework::~AndroidHookFramework() {
    Clear();
}

bool AndroidHookFramework::Initialize() {
    // TODO: 初始化xHook或其他Hook框架
    initialized_ = true;
    return true;
}

bool AndroidHookFramework::RegisterHook(const std::string& targetLib, const std::string& symbol,
                                       void* hookFunc, void** originalFunc) {
    if (!initialized_) {
        return false;
    }
    
    // TODO: 注册Hook
    hooks_.push_back(std::make_tuple(targetLib, symbol, hookFunc, originalFunc));
    return true;
}

bool AndroidHookFramework::Refresh() {
    if (!initialized_) {
        return false;
    }
    
    // TODO: 刷新Hook
    return true;
}

bool AndroidHookFramework::Clear() {
    // TODO: 清理Hook
    hooks_.clear();
    initialized_ = false;
    return true;
}

// ==================== UnifiedAndroidInjector ====================

UnifiedAndroidInjector::UnifiedAndroidInjector()
    : apkModifier_(std::make_unique<APKModifier>())
    , soLoader_(std::make_unique<SoLoader>())
    , hookFramework_(std::make_unique<AndroidHookFramework>())
{
}

UnifiedAndroidInjector::~UnifiedAndroidInjector() {
}

void UnifiedAndroidInjector::SetConfig(const std::string& configPath) {
    configPath_ = configPath;
}

void UnifiedAndroidInjector::AddCompatModule(const std::string& modulePath) {
    compatModules_.push_back(modulePath);
}

void UnifiedAndroidInjector::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
    apkModifier_->SetProgressCallback(callback);
}

bool UnifiedAndroidInjector::InjectToApk(const std::string& inputApk, const std::string& outputApk) {
    // 创建临时目录
    std::string tempDir = fs::temp_directory_path().string() + "/unifier_inject_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(tempDir);
    
    // 准备native库
    if (!PrepareNativeLibs(tempDir)) {
        fs::remove_all(tempDir);
        return false;
    }
    
    // 生成Hook配置
    if (!GenerateHookConfig(tempDir)) {
        fs::remove_all(tempDir);
        return false;
    }
    
    // 收集所有native库
    std::vector<std::string> nativeLibs;
    for (const auto& entry : fs::directory_iterator(tempDir)) {
        if (entry.path().extension() == ".so") {
            nativeLibs.push_back(entry.path().string());
        }
    }
    
    // 执行注入
    bool result = apkModifier_->Inject(inputApk, outputApk, nativeLibs);
    
    // 清理临时目录
    fs::remove_all(tempDir);
    
    return result;
}

bool UnifiedAndroidInjector::PrepareNativeLibs(const std::string& workDir) {
    // 创建加载器so
    std::string loaderSo = workDir + "/libMinecraftUnifier.so";
    if (!soLoader_->CreateLoaderSo(loaderSo)) {
        return false;
    }
    
    // 复制兼容模块
    for (const auto& module : compatModules_) {
        fs::copy_file(module, workDir + "/" + fs::path(module).filename().string(),
                     fs::copy_options::overwrite_existing);
    }
    
    return true;
}

bool UnifiedAndroidInjector::GenerateHookConfig(const std::string& workDir) {
    // 生成Hook配置文件
    std::string configPath = workDir + "/hook_config.json";
    std::ofstream configFile(configPath);
    
    // TODO: 生成Hook配置
    configFile << "{}";
    configFile.close();
    
    return true;
}

// ==================== hooks namespace ====================

namespace hooks {

FILE* (*orig_fopen)(const char* path, const char* mode) = nullptr;

FILE* hooked_fopen(const char* path, const char* mode) {
    // 检查是否需要重定向
    if (strstr(path, "shaderpacks") != nullptr) {
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "/sdcard/Unifier/shaderpacks/%s",
                 strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
        FILE* f = orig_fopen(newPath, mode);
        if (f) {
            return f;
        }
    }
    
    return orig_fopen(path, mode);
}

int (*orig_open)(const char* pathname, int flags, mode_t mode) = nullptr;

int hooked_open(const char* pathname, int flags, mode_t mode) {
    // 类似fopen的Hook逻辑
    return orig_open(pathname, flags, mode);
}

void (*orig_glShaderSource)(GLuint shader, GLsizei count, 
                           const GLchar* const* string, const GLint* length) = nullptr;

void hooked_glShaderSource(GLuint shader, GLsizei count, 
                          const GLchar* const* string, const GLint* length) {
    // 拦截着色器源代码，进行转换
    // TODO: 实现GLSL转换逻辑
    orig_glShaderSource(shader, count, string, length);
}

bool InstallAllHooks() {
    // TODO: 使用xHook安装所有Hook
    return true;
}

} // namespace hooks

} // namespace android
} // namespace injector
} // namespace mcu
