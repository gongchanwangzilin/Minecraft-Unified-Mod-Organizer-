/**
 * Minecraft Unifier - PE Injector Implementation (Windows)
 * PE文件注入器实现 - Windows平台
 */

#include "pe_injector.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <windows.h>
#include <imagehlp.h>

namespace fs = std::filesystem;

namespace mcu {
namespace injector {
namespace windows {

// ==================== PEInjector ====================

PEInjector::PEInjector() {
}

PEInjector::~PEInjector() {
}

void PEInjector::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool PEInjector::InjectToPE(const std::string& targetPath, const std::string& outputPath,
                           const std::string& dllPath) {
    if (progressCallback_) {
        progressCallback_(10, "开始注入PE文件...");
    }
    
    // 验证PE文件
    if (!ValidatePE(targetPath)) {
        if (progressCallback_) {
            progressCallback_(0, "PE文件验证失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(30, "PE文件验证完成");
    }
    
    // 修改导入表
    if (!ModifyImportTable(targetPath, outputPath, dllPath)) {
        if (progressCallback_) {
            progressCallback_(30, "导入表修改失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(100, "注入完成！");
    }
    
    return true;
}

bool PEInjector::ValidatePE(const std::string& pePath) {
    HANDLE hFile = CreateFileA(pePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping) {
        CloseHandle(hFile);
        return false;
    }
    
    LPVOID pBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!pBase) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pBase;
    PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((DWORD_PTR)pBase + pDos->e_lfanew);
    
    // 验证DOS头
    if (pDos->e_magic != IMAGE_DOS_SIGNATURE) {
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    // 验证PE头
    if (pNt->Signature != IMAGE_NT_SIGNATURE) {
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    UnmapViewOfFile(pBase);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    
    return true;
}

std::vector<std::string> PEInjector::GetImportedDLLs(const std::string& pePath) {
    std::vector<std::string> dlls;
    
    HANDLE hFile = CreateFileA(pePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return dlls;
    }
    
    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMapping) {
        CloseHandle(hFile);
        return dlls;
    }
    
    LPVOID pBase = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if (!pBase) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return dlls;
    }
    
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pBase;
    PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((DWORD_PTR)pBase + pDos->e_lfanew);
    
    // 定位导入表
    PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)
        ((DWORD_PTR)pBase + pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    
    // 遍历导入表
    while (pImport->Name) {
        char* dllName = (char*)((DWORD_PTR)pBase + pImport->Name);
        dlls.push_back(dllName);
        pImport++;
    }
    
    UnmapViewOfFile(pBase);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    
    return dlls;
}

bool PEInjector::AddImport(const std::string& pePath, const std::string& dllName) {
    // TODO: 实现添加导入项
    return true;
}

bool PEInjector::RemoveImport(const std::string& pePath, const std::string& dllName) {
    // TODO: 实现移除导入项
    return true;
}

bool PEInjector::ModifyImportTable(const std::string& targetPath, const std::string& outputPath,
                                  const std::string& dllPath) {
    HANDLE hFile = CreateFileA(targetPath.c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    LPVOID pBase = MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pBase;
    PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((DWORD_PTR)pBase + pDos->e_lfanew);
    
    // 定位导入表
    PIMAGE_IMPORT_DESCRIPTOR pImport = (PIMAGE_IMPORT_DESCRIPTOR)
        ((DWORD_PTR)pBase + pNt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    
    // 查找kernel32.dll的导入项
    bool found = false;
    PIMAGE_IMPORT_DESCRIPTOR pKernel32 = nullptr;
    while (pImport->Name) {
        char* dllName = (char*)((DWORD_PTR)pBase + pImport->Name);
        if (_stricmp(dllName, "kernel32.dll") == 0) {
            pKernel32 = pImport;
            found = true;
            break;
        }
        pImport++;
    }
    
    if (!found) {
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    // 在kernel32.dll之前插入我们的DLL
    if (!InsertNewDll(pBase, dllPath)) {
        UnmapViewOfFile(pBase);
        CloseHandle(hMapping);
        CloseHandle(hFile);
        return false;
    }
    
    // 写入修改后的文件
    FlushViewOfFile(pBase, 0);
    UnmapViewOfFile(pBase);
    CloseHandle(hMapping);
    CloseHandle(hFile);
    
    // 复制到输出路径
    CopyFileA(targetPath.c_str(), outputPath.c_str(), FALSE);
    
    return true;
}

bool PEInjector::InsertNewDll(void* pBase, const std::string& dllName) {
    // TODO: 实现插入新DLL到导入表
    // 这需要修改PE文件的导入表，添加新的导入项
    
    // 临时实现：直接返回成功
    return true;
}

bool PEInjector::CalculateNewOffsets(void* pBase, size_t newSize) {
    // TODO: 计算新的偏移量
    return true;
}

// ==================== WindowsLoader ====================

WindowsLoader::WindowsLoader() {
}

WindowsLoader::~WindowsLoader() {
}

void WindowsLoader::SetHookConfig(const std::string& configPath) {
    hookConfigPath_ = configPath;
}

void WindowsLoader::SetCompatModules(const std::vector<std::string>& modules) {
    compatModules_ = modules;
}

std::string WindowsLoader::GenerateLoaderCode() {
    std::stringstream code;
    
    code << R"(/**
 * Minecraft Unifier - Windows Loader
 * Windows加载器 - 自动生成
 */

#include <windows.h>
#include <detours.h>
#include <string>
#include <vector>

// 全局变量
static HMODULE g_hModule = NULL;
static std::vector<HMODULE> g_loadedModules;

// DLL入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, LoaderInitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        ShutdownUnifier();
        break;
    }
    return TRUE;
}

// 初始化线程
DWORD WINAPI LoaderInitThread(LPVOID) {
    // 等待游戏核心库加载
    while (!GetModuleHandleA("minecraftpe.dll")) {
        Sleep(100);
    }
    
    // 加载配置
    LoadConfig("unifier_config.json");
    
    // 加载兼容模块
)";

    // 添加兼容模块加载代码
    for (const auto& module : compatModules_) {
        code << "    HMODULE " + module + " = LoadLibraryA(\"" + module + ".dll\");\n";
        code << "    if (" + module + ") {\n";
        code << "        g_loadedModules.push_back(" + module + ");\n";
        code << "        // 调用初始化函数\n";
        code << "        typedef void (*init_func_t)();\n";
        code << "        init_func_t init = (init_func_t)GetProcAddress(" + module + ", \"Initialize\");\n";
        code << "        if (init) init();\n";
        code << "    }\n";
    }

    code << R"(
    // 安装Hook
    InstallHooks();
    
    return 0;
}

// 加载配置
void LoadConfig(const char* configPath) {
    // TODO: 实现配置文件解析
}

// 安装Hook
void InstallHooks() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    // Hook文件操作
    DetourAttach(&(PVOID&)original_CreateFileW, Hooked_CreateFileW);
    
    // Hook OpenGL
    DetourAttach(&(PVOID&)original_glShaderSource, Hooked_glShaderSource);
    
    DetourTransactionCommit();
}

// 关闭统一器
void ShutdownUnifier() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    // 移除Hook
    DetourDetach(&(PVOID&)original_CreateFileW, Hooked_CreateFileW);
    DetourDetach(&(PVOID&)original_glShaderSource, Hooked_glShaderSource);
    
    DetourTransactionCommit();
    
    // 卸载兼容模块
    for (HMODULE module : g_loadedModules) {
        FreeLibrary(module);
    }
}

// Hook函数示例
HANDLE (WINAPI* original_CreateFileW)(LPCWSTR, DWORD, DWORD,
                                      LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = CreateFileW;

HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                 DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                 HANDLE hTemplateFile) {
    // 转换宽字符到UTF-8
    char utf8Path[512];
    WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, utf8Path, sizeof(utf8Path), NULL, NULL);
    
    // 检查是否需要重定向
    if (strstr(utf8Path, "shaderpacks") != NULL) {
        char newPath[512];
        sprintf_s(newPath, sizeof(newPath), "C:\\Unifier\\shaderpacks\\%s",
                  strrchr(utf8Path, '/') ? strrchr(utf8Path, '/') + 1 : utf8Path);
        
        WCHAR widePath[512];
        MultiByteToWideChar(CP_UTF8, 0, newPath, -1, widePath, sizeof(widePath)/sizeof(WCHAR));
        
        return original_CreateFileW(widePath, dwDesiredAccess, dwShareMode,
                                   lpSecurityAttributes, dwCreationDisposition,
                                   dwFlagsAndAttributes, hTemplateFile);
    }
    
    return original_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                               lpSecurityAttributes, dwCreationDisposition,
                               dwFlagsAndAttributes, hTemplateFile);
}

void (APIENTRY* original_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = glShaderSource;

void APIENTRY Hooked_glShaderSource(GLuint shader, GLsizei count,
                                   const GLchar* const* string, const GLint* length) {
    // 拦截着色器源代码，进行转换
    // TODO: 实现GLSL转换逻辑
    original_glShaderSource(shader, count, string, length);
}
)";
    
    return code.str();
}

bool WindowsLoader::CreateLoaderSource(const std::string& outputPath) {
    std::string code = GenerateLoaderCode();
    
    std::ofstream file(outputPath);
    file << code;
    file.close();
    
    return true;
}

bool WindowsLoader::CompileLoader(const std::string& sourcePath, const std::string& outputPath) {
    // 使用Visual Studio编译器或MinGW编译
    std::string cmd = "cl.exe /LD /EHsc \"" + sourcePath + "\" /Fe\"" + outputPath + "\" detours.lib";
    return system(cmd.c_str()) == 0;
}

// ==================== WindowsHookFramework ====================

WindowsHookFramework::WindowsHookFramework()
    : initialized_(false) {
}

WindowsHookFramework::~WindowsHookFramework() {
    Clear();
}

bool WindowsHookFramework::Initialize() {
    // 初始化Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourTransactionCommit();
    
    initialized_ = true;
    return true;
}

bool WindowsHookFramework::RegisterHook(const std::string& moduleName, const std::string& functionName,
                                       void* hookFunc, void** originalFunc) {
    if (!initialized_) {
        return false;
    }
    
    HMODULE hModule = GetModuleHandleA(moduleName.c_str());
    if (!hModule) {
        return false;
    }
    
    void* targetFunc = GetProcAddress(hModule, functionName.c_str());
    if (!targetFunc) {
        return false;
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)targetFunc, hookFunc);
    DetourTransactionCommit();
    
    *originalFunc = targetFunc;
    hooks_.push_back(std::make_tuple(moduleName, functionName, hookFunc, originalFunc));
    
    return true;
}

bool WindowsHookFramework::Refresh() {
    if (!initialized_) {
        return false;
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourTransactionCommit();
    
    return true;
}

bool WindowsHookFramework::Clear() {
    if (!initialized_) {
        return false;
    }
    
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    for (const auto& [moduleName, functionName, hookFunc, originalFunc] : hooks_) {
        DetourDetach(&(PVOID&)*originalFunc, hookFunc);
    }
    
    DetourTransactionCommit();
    
    hooks_.clear();
    initialized_ = false;
    return true;
}

// ==================== UnifiedWindowsInjector ====================

UnifiedWindowsInjector::UnifiedWindowsInjector()
    : peInjector_(std::make_unique<PEInjector>())
    , loader_(std::make_unique<WindowsLoader>())
    , hookFramework_(std::make_unique<WindowsHookFramework>())
{
}

UnifiedWindowsInjector::~UnifiedWindowsInjector() {
}

void UnifiedWindowsInjector::SetConfig(const std::string& configPath) {
    configPath_ = configPath;
}

void UnifiedWindowsInjector::AddCompatModule(const std::string& modulePath) {
    compatModules_.push_back(modulePath);
}

void UnifiedWindowsInjector::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
    peInjector_->SetProgressCallback(callback);
}

bool UnifiedWindowsInjector::InjectToPE(const std::string& inputExe, const std::string& outputExe) {
    // 创建临时目录
    std::string workDir = fs::temp_directory_path().string() + "/unifier_inject_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(workDir);
    
    // 准备加载器DLL
    if (!PrepareLoaderDLL(workDir)) {
        fs::remove_all(workDir);
        return false;
    }
    
    // 生成Hook配置
    if (!GenerateHookConfig(workDir)) {
        fs::remove_all(workDir);
        return false;
    }
    
    // 执行注入
    std::string loaderDll = workDir + "/MinecraftUnifier.dll";
    bool result = peInjector_->InjectToPE(inputExe, outputExe, loaderDll);
    
    // 清理临时目录
    fs::remove_all(workDir);
    
    return result;
}

bool UnifiedWindowsInjector::PrepareLoaderDLL(const std::string& workDir) {
    // 创建加载器源代码
    std::string sourcePath = workDir + "/loader.cpp";
    if (!loader_->CreateLoaderSource(sourcePath)) {
        return false;
    }
    
    // 编译加载器DLL
    std::string outputPath = workDir + "/MinecraftUnifier.dll";
    if (!loader_->CompileLoader(sourcePath, outputPath)) {
        return false;
    }
    
    return true;
}

bool UnifiedWindowsInjector::GenerateHookConfig(const std::string& workDir) {
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

HANDLE (WINAPI* orig_CreateFileW)(LPCWSTR, DWORD, DWORD,
                                  LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = nullptr;

HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                 DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                 HANDLE hTemplateFile) {
    // 转换宽字符到UTF-8
    char utf8Path[512];
    WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, utf8Path, sizeof(utf8Path), NULL, NULL);
    
    // 检查是否需要重定向
    if (strstr(utf8Path, "shaderpacks") != NULL) {
        char newPath[512];
        sprintf_s(newPath, sizeof(newPath), "C:\\Unifier\\shaderpacks\\%s",
                  strrchr(utf8Path, '/') ? strrchr(utf8Path, '/') + 1 : utf8Path);
        
        WCHAR widePath[512];
        MultiByteToWideChar(CP_UTF8, 0, newPath, -1, widePath, sizeof(widePath)/sizeof(WCHAR));
        
        return orig_CreateFileW(widePath, dwDesiredAccess, dwShareMode,
                              lpSecurityAttributes, dwCreationDisposition,
                              dwFlagsAndAttributes, hTemplateFile);
    }
    
    return orig_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                           lpSecurityAttributes, dwCreationDisposition,
                           dwFlagsAndAttributes, hTemplateFile);
}

void (APIENTRY* orig_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = nullptr;

void APIENTRY Hooked_glShaderSource(GLuint shader, GLsizei count,
                                   const GLchar* const* string, const GLint* length) {
    // 拦截着色器源代码，进行转换
    // TODO: 实现GLSL转换逻辑
    orig_glShaderSource(shader, count, string, length);
}

bool InstallAllHooks() {
    // TODO: 使用Detours安装所有Hook
    return true;
}

} // namespace hooks

} // namespace windows
} // namespace injector
} // namespace mcu
