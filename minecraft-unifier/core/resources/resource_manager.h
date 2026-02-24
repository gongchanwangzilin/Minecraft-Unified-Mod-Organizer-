/**
 * Minecraft Unifier - Resource Manager
 * 资源管理器 - 跨平台文件系统Hook
 */

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace mcu {
namespace core {
namespace resources {

// 重定向规则
struct RedirectRule {
    std::string fromPattern;    // 源路径模式
    std::string toPattern;      // 目标路径模式
    bool enabled;               // 是否启用
    int priority;               // 优先级
};

// 资源类型
enum class ResourceType {
    UNKNOWN,
    TEXTURE,
    MODEL,
    SOUND,
    LANG,
    SHADER,
    SCRIPT
};

// 资源信息
struct ResourceInfo {
    std::string originalPath;   // 原始路径
    std::string redirectPath;   // 重定向路径
    ResourceType type;          // 资源类型
    bool converted;             // 是否已转换
    uint64_t size;              // 文件大小
    uint64_t timestamp;         // 时间戳
};

// 资源管理器
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    // 初始化
    bool Initialize();
    
    // 关闭
    void Shutdown();
    
    // 添加重定向规则
    void AddRedirectRule(const std::string& from, const std::string& to, int priority = 0);
    
    // 移除重定向规则
    void RemoveRedirectRule(const std::string& from);
    
    // 应用重定向
    std::string ApplyRedirect(const std::string& originalPath);
    
    // 添加资源
    bool AddResource(const std::string& originalPath, const std::string& redirectPath);
    
    // 移除资源
    bool RemoveResource(const std::string& originalPath);
    
    // 获取资源信息
    const ResourceInfo* GetResourceInfo(const std::string& originalPath) const;
    
    // 获取所有资源
    std::vector<ResourceInfo> GetAllResources() const;
    
    // 转换资源
    bool ConvertResource(const std::string& originalPath, const std::string& outputPath);
    
    // 批量转换资源
    bool BatchConvert(const std::string& inputDir, const std::string& outputDir);
    
    // 安装文件系统Hook
    bool InstallFileHooks();
    
    // 卸载文件系统Hook
    bool UninstallFileHooks();
    
    // 事件回调
    using FileAccessCallback = std::function<void(const std::string& path, bool redirected)>;
    void SetFileAccessCallback(FileAccessCallback callback);

private:
    bool initialized_;
    std::vector<RedirectRule> redirectRules_;
    std::unordered_map<std::string, ResourceInfo> resources_;
    FileAccessCallback fileAccessCallback_;
    
    // 平台相关的Hook函数指针
#ifdef _WIN32
    static HANDLE (WINAPI* orig_CreateFileW)(LPCWSTR, DWORD, DWORD,
                                             LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
#elif defined(__linux__)
    static FILE* (*orig_fopen)(const char*, const char*);
    static int (*orig_open)(const char*, int, mode_t);
#elif defined(__ANDROID__)
    static FILE* (*orig_fopen)(const char*, const char*);
    static int (*orig_open)(const char*, int, mode_t);
#endif
    
    // 内部处理函数
    bool DetectResourceType(const std::string& path, ResourceType& type);
    bool ConvertTexture(const std::string& inputPath, const std::string& outputPath);
    bool ConvertModel(const std::string& inputPath, const std::string& outputPath);
    bool ConvertSound(const std::string& inputPath, const std::string& outputPath);
    bool ConvertLang(const std::string& inputPath, const std::string& outputPath);
    
    // Hook回调函数
#ifdef _WIN32
    static HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                            DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                            DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                            HANDLE hTemplateFile);
#else
    static FILE* Hooked_fopen(const char* path, const char* mode);
    static int Hooked_open(const char* pathname, int flags, mode_t mode);
#endif
};

// 资源转换器
class ResourceConverter {
public:
    ResourceConverter();
    ~ResourceConverter();
    
    // 转换纹理
    static bool ConvertTexture(const std::string& inputPath, const std::string& outputPath);
    
    // 转换模型
    static bool ConvertModel(const std::string& inputPath, const std::string& outputPath);
    
    // 转换声音
    static bool ConvertSound(const std::string& inputPath, const std::string& outputPath);
    
    // 转换语言文件
    static bool ConvertLang(const std::string& inputPath, const std::string& outputPath);
    
    // 检测资源类型
    static ResourceType DetectType(const std::string& path);

private:
    // 内部转换函数
    static bool ConvertPNGToTexture(const std::string& inputPath, const std::string& outputPath);
    static bool ConvertOBJToModel(const std::string& inputPath, const std::string& outputPath);
    static bool ConvertWAVToSound(const std::string& inputPath, const std::string& outputPath);
    static bool ConvertJSONToLang(const std::string& inputPath, const std::string& outputPath);
};

// 资源包管理器
class ResourcePackManager {
public:
    ResourcePackManager();
    ~ResourcePackManager();
    
    // 加载资源包
    bool LoadResourcePack(const std::string& packPath);
    
    // 卸载资源包
    bool UnloadResourcePack(const std::string& packId);
    
    // 获取已加载的资源包列表
    std::vector<std::string> GetLoadedPacks() const;
    
    // 应用资源包
    bool ApplyResourcePack(const std::string& packId);
    
    // 移除资源包
    bool RemoveResourcePack(const std::string& packId);
    
    // 获取资源包信息
    bool GetPackInfo(const std::string& packId, std::string& name, std::string& description);

private:
    std::unordered_map<std::string, std::string> loadedPacks_; // packId -> packPath
    
    // 内部处理函数
    bool ParsePackManifest(const std::string& packPath, std::string& packId, 
                          std::string& name, std::string& description);
    bool ExtractPackResources(const std::string& packPath, const std::string& outputDir);
};

} // namespace resources
} // namespace core
} // namespace mcu
