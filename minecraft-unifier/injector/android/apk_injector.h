/**
 * Minecraft Unifier - APK Injector (Android)
 * APK注入器 - Android平台
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace mcu {
namespace injector {
namespace android {

// APK修改器
class APKModifier {
public:
    APKModifier();
    ~APKModifier();

    // 解包APK
    bool Decompile(const std::string& apkPath, const std::string& outputDir);
    
    // 修改AndroidManifest.xml
    bool ModifyManifest(const std::string& workDir);
    
    // 修改smali代码
    bool ModifySmali(const std::string& workDir);
    
    // 添加native库
    bool AddNativeLibs(const std::string& workDir, const std::vector<std::string>& libPaths);
    
    // 重新打包APK
    bool Repack(const std::string& workDir, const std::string& outputApk);
    
    // 签名APK
    bool SignApk(const std::string& unsignedApk, const std::string& signedApk, 
                 const std::string& keystorePath, const std::string& keystorePass);
    
    // 完整注入流程
    bool Inject(const std::string& inputApk, const std::string& outputApk,
                const std::vector<std::string>& nativeLibs);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    ProgressCallback progressCallback_;
    
    // 内部辅助函数
    bool ExecuteCommand(const std::string& cmd);
    bool CheckToolInstalled(const std::string& tool);
    std::string FindMainActivity(const std::string& workDir);
};

// SO库加载器
class SoLoader {
public:
    SoLoader();
    ~SoLoader();

    // 创建加载器so库
    bool CreateLoaderSo(const std::string& outputPath);
    
    // 设置Hook配置
    void SetHookConfig(const std::unordered_map<std::string, std::string>& hooks);
    
    // 设置兼容模块路径
    void SetCompatModules(const std::vector<std::string>& modules);

private:
    std::unordered_map<std::string, std::string> hookConfig_;
    std::vector<std::string> compatModules_;
    
    // 生成C++代码
    std::string GenerateLoaderCode();
};

// Android Hook框架封装
class AndroidHookFramework {
public:
    AndroidHookFramework();
    ~AndroidHookFramework();

    // 初始化Hook框架
    bool Initialize();
    
    // 注册Hook
    bool RegisterHook(const std::string& targetLib, const std::string& symbol,
                      void* hookFunc, void** originalFunc);
    
    // 刷新Hook
    bool Refresh();
    
    // 清理Hook
    bool Clear();

private:
    bool initialized_;
    std::vector<std::tuple<std::string, std::string, void*, void**>> hooks_;
};

// 统一Android注入器
class UnifiedAndroidInjector {
public:
    UnifiedAndroidInjector();
    ~UnifiedAndroidInjector();

    // 注入到APK
    bool InjectToApk(const std::string& inputApk, const std::string& outputApk);
    
    // 设置注入配置
    void SetConfig(const std::string& configPath);
    
    // 添加兼容模块
    void AddCompatModule(const std::string& modulePath);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    std::string configPath_;
    std::vector<std::string> compatModules_;
    ProgressCallback progressCallback_;
    
    std::unique_ptr<APKModifier> apkModifier_;
    std::unique_ptr<SoLoader> soLoader_;
    std::unique_ptr<AndroidHookFramework> hookFramework_;
    
    // 内部处理
    bool PrepareNativeLibs(const std::string& workDir);
    bool GenerateHookConfig(const std::string& workDir);
};

// Hook函数定义
namespace hooks {
    // 文件操作Hook
    FILE* (*orig_fopen)(const char* path, const char* mode);
    FILE* hooked_fopen(const char* path, const char* mode);
    
    int (*orig_open)(const char* pathname, int flags, mode_t mode);
    int hooked_open(const char* pathname, int flags, mode_t mode);
    
    // OpenGL Hook
    void (*orig_glShaderSource)(GLuint shader, GLsizei count, 
                                const GLchar* const* string, const GLint* length);
    void hooked_glShaderSource(GLuint shader, GLsizei count, 
                               const GLchar* const* string, const GLint* length);
    
    // 安装所有Hook
    bool InstallAllHooks();
}

} // namespace android
} // namespace injector
} // namespace mcu
