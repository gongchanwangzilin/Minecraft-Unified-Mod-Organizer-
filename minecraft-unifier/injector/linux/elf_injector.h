/**
 * Minecraft Unifier - ELF Injector (Linux)
 * ELF文件注入器 - Linux平台
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace mcu {
namespace injector {
namespace linux {

// ELF文件注入器
class ELFInjector {
public:
    ELFInjector();
    ~ELFInjector();

    // 注入到ELF文件
    bool InjectToELF(const std::string& targetPath, const std::string& outputPath,
                    const std::string& soPath);
    
    // 验证ELF文件
    bool ValidateELF(const std::string& elfPath);
    
    // 获取ELF文件详细信息
    struct ELFInfo {
        std::string architecture;  // x86_64, ARM64
        std::string type;         // EXEC, DYN, REL
        size_t entryPoint;        // 入口点
        std::vector<std::string> sections;  // 节列表
        std::vector<std::string> neededLibraries;  // 需要的动态库
        std::vector<std::string> exportedSymbols;  // 导出的符号
    };
    bool GetELFInfo(const std::string& elfPath, ELFInfo& info);
    
    // 获取动态库依赖
    std::vector<std::string> GetNeededLibraries(const std::string& elfPath);
    
    // 获取导出符号
    std::vector<std::string> GetExportedSymbols(const std::string& elfPath);
    
    // 获取节表信息
    struct SectionInfo {
        std::string name;
        size_t virtualAddress;
        size_t size;
        size_t offset;
        bool isExecutable;
        bool isWritable;
        bool isAllocatable;
    };
    std::vector<SectionInfo> GetSections(const std::string& elfPath);
    
    // 添加动态库依赖
    bool AddNeededLibrary(const std::string& elfPath, const std::string& libName);
    
    // 移除动态库依赖
    bool RemoveNeededLibrary(const std::string& elfPath, const std::string& libName);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    ProgressCallback progressCallback_;
    
    // 内部处理函数
    bool ModifyDynamicSection(const std::string& targetPath, const std::string& outputPath,
                             const std::string& soPath);
    bool InsertNeededEntry(void* map, const std::string& libName);
    bool CalculateNewOffsets(void* map, size_t newSize);
};

// Linux加载器SO
class LinuxLoader {
public:
    LinuxLoader();
    ~LinuxLoader();
    
    // 创建加载器SO源代码
    bool CreateLoaderSource(const std::string& outputPath);
    
    // 编译加载器SO
    bool CompileLoader(const std::string& sourcePath, const std::string& outputPath);
    
    // 设置Hook配置
    void SetHookConfig(const std::string& configPath);
    
    // 设置兼容模块路径
    void SetCompatModules(const std::vector<std::string>& modules);

private:
    std::string hookConfigPath_;
    std::vector<std::string> compatModules_;
    
    // 生成C++代码
    std::string GenerateLoaderCode();
};

// Linux Hook框架
class LinuxHookFramework {
public:
    LinuxHookFramework();
    ~LinuxHookFramework();
    
    // 初始化Hook框架
    bool Initialize();
    
    // 注册Hook
    bool RegisterHook(const std::string& libraryName, const std::string& symbol,
                     void* hookFunc, void** originalFunc);
    
    // 刷新Hook
    bool Refresh();
    
    // 清理Hook
    bool Clear();

private:
    bool initialized_;
    std::vector<std::tuple<std::string, std::string, void*, void**>> hooks_;
};

// 统一Linux注入器
class UnifiedLinuxInjector {
public:
    UnifiedLinuxInjector();
    ~UnifiedLinuxInjector();
    
    // 注入到ELF文件
    bool InjectToELF(const std::string& inputElf, const std::string& outputElf);
    
    // 设置配置
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
    
    std::unique_ptr<ELFInjector> elfInjector_;
    std::unique_ptr<LinuxLoader> loader_;
    std::unique_ptr<LinuxHookFramework> hookFramework_;
    
    // 内部处理
    bool PrepareLoaderSO(const std::string& workDir);
    bool GenerateHookConfig(const std::string& workDir);
};

// Hook函数定义
namespace hooks {
    // 文件操作Hook
    FILE* (*orig_fopen)(const char*, const char*);
    FILE* hooked_fopen(const char* path, const char* mode);
    
    int (*orig_open)(const char*, int, mode_t);
    int hooked_open(const char* pathname, int flags, mode_t mode);
    
    // OpenGL Hook
    void (*orig_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
    void hooked_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
    
    // 安装所有Hook
    bool InstallAllHooks();
}

} // namespace linux
} // namespace injector
} // namespace mcu
