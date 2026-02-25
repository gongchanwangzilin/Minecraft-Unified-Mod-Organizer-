/**
 * Minecraft Unifier - PE Injector (Windows)
 * PE文件注入器 - Windows平台
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace mcu {
namespace injector {
namespace windows {

// PE文件注入器
class PEInjector {
public:
    PEInjector();
    ~PEInjector();

    // 注入到PE文件
    bool InjectToPE(const std::string& targetPath, const std::string& outputPath,
                   const std::string& dllPath);
    
    // 验证PE文件
    bool ValidatePE(const std::string& pePath);
    
    // 获取PE文件详细信息
    struct PEInfo {
        std::string architecture;  // x86, x64
        std::string subsystem;     // GUI, Console, DLL
        size_t imageSize;          // 镜像大小
        size_t entryPoint;         // 入口点
        std::vector<std::string> sections;  // 节列表
        std::vector<std::string> importedDLLs;  // 导入的DLL
        std::vector<std::string> exportedFunctions;  // 导出的函数
    };
    bool GetPEInfo(const std::string& pePath, PEInfo& info);
    
    // 获取导入表信息
    std::vector<std::string> GetImportedDLLs(const std::string& pePath);
    
    // 获取导出表信息
    std::vector<std::string> GetExportedFunctions(const std::string& pePath);
    
    // 获取节表信息
    struct SectionInfo {
        std::string name;
        size_t virtualAddress;
        size_t virtualSize;
        size_t rawSize;
        size_t rawOffset;
        bool isExecutable;
        bool isReadable;
        bool isWritable;
    };
    std::vector<SectionInfo> GetSections(const std::string& pePath);
    
    // 添加导入项
    bool AddImport(const std::string& pePath, const std::string& dllName);
    
    // 移除导入项
    bool RemoveImport(const std::string& pePath, const std::string& dllName);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    ProgressCallback progressCallback_;
    
    // 内部处理函数
    bool ModifyImportTable(const std::string& targetPath, const std::string& outputPath,
                          const std::string& dllPath);
    bool InsertNewDll(void* pBase, const std::string& dllName);
    bool CalculateNewOffsets(void* pBase, size_t newSize);
};

// Windows加载器DLL
class WindowsLoader {
public:
    WindowsLoader();
    ~WindowsLoader();
    
    // 创建加载器DLL源代码
    bool CreateLoaderSource(const std::string& outputPath);
    
    // 编译加载器DLL
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

// Windows Hook框架
class WindowsHookFramework {
public:
    WindowsHookFramework();
    ~WindowsHookFramework();
    
    // 初始化Hook框架
    bool Initialize();
    
    // 注册Hook
    bool RegisterHook(const std::string& moduleName, const std::string functionName,
                     void* hookFunc, void** originalFunc);
    
    // 刷新Hook
    bool Refresh();
    
    // 清理Hook
    bool Clear();

private:
    bool initialized_;
    std::vector<std::tuple<std::string, std::string, void*, void**>> hooks_;
};

// 统一Windows注入器
class UnifiedWindowsInjector {
public:
    UnifiedWindowsInjector();
    ~UnifiedWindowsInjector();
    
    // 注入到PE文件
    bool InjectToPE(const std::string& inputExe, const std::string& outputExe);
    
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
    
    std::unique_ptr<PEInjector> peInjector_;
    std::unique_ptr<WindowsLoader> loader_;
    std::unique_ptr<WindowsHookFramework> hookFramework_;
    
    // 内部处理
    bool PrepareLoaderDLL(const std::string& workDir);
    bool GenerateHookConfig(const std::string& workDir);
};

// Hook函数定义
namespace hooks {
    // 文件操作Hook
    HANDLE (WINAPI* orig_CreateFileW)(LPCWSTR, DWORD, DWORD,
                                      LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                     DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                     DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                     HANDLE hTemplateFile);
    
    // OpenGL Hook
    void (APIENTRY* orig_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*);
    void APIENTRY Hooked_glShaderSource(GLuint shader, GLsizei count,
                                       const GLchar* const* string, const GLint* length);
    
    // 安装所有Hook
    bool InstallAllHooks();
}

} // namespace windows
} // namespace injector
} // namespace mcu
