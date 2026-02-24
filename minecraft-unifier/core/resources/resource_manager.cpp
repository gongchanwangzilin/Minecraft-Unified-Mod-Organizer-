/**
 * Minecraft Unifier - Resource Manager Implementation
 * 资源管理器实现 - 跨平台文件系统Hook
 */

#include "resource_manager.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace mcu {
namespace core {
namespace resources {

// ==================== ResourceManager ====================

ResourceManager::ResourceManager()
    : initialized_(false) {
}

ResourceManager::~ResourceManager() {
    Shutdown();
}

bool ResourceManager::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // 添加默认重定向规则
    AddRedirectRule("shaderpacks/", "/sdcard/Unifier/shaderpacks/", 100);
    AddRedirectRule("resource_packs/", "/sdcard/Unifier/resourcepacks/", 100);
    AddRedirectRule("textures/", "/sdcard/Unifier/textures/", 90);
    AddRedirectRule("models/", "/sdcard/Unifier/models/", 90);
    AddRedirectRule("sounds/", "/sdcard/Unifier/sounds/", 90);
    AddRedirectRule("lang/", "/sdcard/Unifier/lang/", 90);
    
    initialized_ = true;
    return true;
}

void ResourceManager::Shutdown() {
    UninstallFileHooks();
    redirectRules_.clear();
    resources_.clear();
    initialized_ = false;
}

void ResourceManager::AddRedirectRule(const std::string& from, const std::string& to, int priority) {
    RedirectRule rule;
    rule.fromPattern = from;
    rule.toPattern = to;
    rule.enabled = true;
    rule.priority = priority;
    
    redirectRules_.push_back(rule);
    
    // 按优先级排序
    std::sort(redirectRules_.begin(), redirectRules_.end(),
              [](const RedirectRule& a, const RedirectRule& b) {
                  return a.priority > b.priority;
              });
}

void ResourceManager::RemoveRedirectRule(const std::string& from) {
    redirectRules_.erase(
        std::remove_if(redirectRules_.begin(), redirectRules_.end(),
                      [&from](const RedirectRule& rule) {
                          return rule.fromPattern == from;
                      }),
        redirectRules_.end());
}

std::string ResourceManager::ApplyRedirect(const std::string& originalPath) {
    for (const auto& rule : redirectRules_) {
        if (!rule.enabled) {
            continue;
        }
        
        // 检查路径是否匹配模式
        if (originalPath.find(rule.fromPattern) != std::string::npos) {
            std::string result = originalPath;
            size_t pos = result.find(rule.fromPattern);
            if (pos != std::string::npos) {
                result.replace(pos, rule.fromPattern.length(), rule.toPattern);
                
                // 触发文件访问回调
                if (fileAccessCallback_) {
                    fileAccessCallback_(originalPath, true);
                }
                
                return result;
            }
        }
    }
    
    // 触发文件访问回调（未重定向）
    if (fileAccessCallback_) {
        fileAccessCallback_(originalPath, false);
    }
    
    return originalPath;
}

bool ResourceManager::AddResource(const std::string& originalPath, const std::string& redirectPath) {
    ResourceInfo info;
    info.originalPath = originalPath;
    info.redirectPath = redirectPath;
    info.converted = false;
    info.size = 0;
    info.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    // 检测资源类型
    if (!DetectResourceType(originalPath, info.type)) {
        info.type = ResourceType::UNKNOWN;
    }
    
    // 获取文件大小
    if (fs::exists(redirectPath)) {
        info.size = fs::file_size(redirectPath);
    }
    
    resources_[originalPath] = info;
    return true;
}

bool ResourceManager::RemoveResource(const std::string& originalPath) {
    return resources_.erase(originalPath) > 0;
}

const ResourceInfo* ResourceManager::GetResourceInfo(const std::string& originalPath) const {
    auto it = resources_.find(originalPath);
    if (it != resources_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<ResourceInfo> ResourceManager::GetAllResources() const {
    std::vector<ResourceInfo> list;
    for (const auto& [path, info] : resources_) {
        list.push_back(info);
    }
    return list;
}

bool ResourceManager::ConvertResource(const std::string& originalPath, const std::string& outputPath) {
    // 检测资源类型
    ResourceType type;
    if (!DetectResourceType(originalPath, type)) {
        return false;
    }
    
    // 根据类型转换
    switch (type) {
        case ResourceType::TEXTURE:
            return ConvertTexture(originalPath, outputPath);
        case ResourceType::MODEL:
            return ConvertModel(originalPath, outputPath);
        case ResourceType::SOUND:
            return ConvertSound(originalPath, outputPath);
        case ResourceType::LANG:
            return ConvertLang(originalPath, outputPath);
        default:
            return false;
    }
}

bool ResourceManager::BatchConvert(const std::string& inputDir, const std::string& outputDir) {
    fs::create_directories(outputDir);
    
    int successCount = 0;
    int totalCount = 0;
    
    for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            totalCount++;
            
            std::string inputPath = entry.path().string();
            std::string relativePath = fs::relative(entry.path(), inputDir).string();
            std::string outputPath = outputDir + "/" + relativePath;
            
            // 创建输出目录
            fs::create_directories(fs::path(outputPath).parent_path());
            
            if (ConvertResource(inputPath, outputPath)) {
                successCount++;
            }
        }
    }
    
    return successCount == totalCount;
}

bool ResourceManager::InstallFileHooks() {
#ifdef _WIN32
    // Windows平台：使用MinHook或Detours
    // TODO: 实现Windows文件Hook
    return true;
#elif defined(__linux__)
    // Linux平台：使用PLT Hook
    // TODO: 实现Linux文件Hook
    return true;
#elif defined(__ANDROID__)
    // Android平台：使用xHook
    // TODO: 实现Android文件Hook
    return true;
#else
    return false;
#endif
}

bool ResourceManager::UninstallFileHooks() {
    // TODO: 卸载文件Hook
    return true;
}

void ResourceManager::SetFileAccessCallback(FileAccessCallback callback) {
    fileAccessCallback_ = callback;
}

bool ResourceManager::DetectResourceType(const std::string& path, ResourceType& type) {
    std::string ext = fs::path(path).extension().string();
    
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga") {
        type = ResourceType::TEXTURE;
        return true;
    } else if (ext == ".obj" || ext == ".geo" || ext == ".json") {
        type = ResourceType::MODEL;
        return true;
    } else if (ext == ".ogg" || ext == ".wav" || ext == ".mp3") {
        type = ResourceType::SOUND;
        return true;
    } else if (ext == ".lang" || ext == ".json") {
        type = ResourceType::LANG;
        return true;
    } else if (ext == ".fsh" || ext == ".vsh" || ext == ".gsh") {
        type = ResourceType::SHADER;
        return true;
    } else if (ext == ".py" || ext == ".js") {
        type = ResourceType::SCRIPT;
        return true;
    }
    
    type = ResourceType::UNKNOWN;
    return false;
}

bool ResourceManager::ConvertTexture(const std::string& inputPath, const std::string& outputPath) {
    return ResourceConverter::ConvertTexture(inputPath, outputPath);
}

bool ResourceManager::ConvertModel(const std::string& inputPath, const std::string& outputPath) {
    return ResourceConverter::ConvertModel(inputPath, outputPath);
}

bool ResourceManager::ConvertSound(const std::string& inputPath, const std::string& outputPath) {
    return ResourceConverter::ConvertSound(inputPath, outputPath);
}

bool ResourceManager::ConvertLang(const std::string& inputPath, const std::string& outputPath) {
    return ResourceConverter::ConvertLang(inputPath, outputPath);
}

#ifdef _WIN32
HANDLE (WINAPI* ResourceManager::orig_CreateFileW)(LPCWSTR, DWORD, DWORD,
                                                    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = nullptr;

HANDLE WINAPI ResourceManager::Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                                   DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                                   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                                   HANDLE hTemplateFile) {
    // 转换宽字符到UTF-8
    char utf8Path[512];
    WideCharToMultiByte(CP_UTF8, 0, lpFileName, -1, utf8Path, sizeof(utf8Path), NULL, NULL);
    
    // 应用重定向
    std::string newPath = ResourceManager::instance().ApplyRedirect(utf8Path);
    
    // 转换回宽字符
    WCHAR widePath[512];
    MultiByteToWideChar(CP_UTF8, 0, newPath.c_str(), -1, widePath, sizeof(widePath)/sizeof(WCHAR));
    
    return orig_CreateFileW(widePath, dwDesiredAccess, dwShareMode,
                           lpSecurityAttributes, dwCreationDisposition,
                           dwFlagsAndAttributes, hTemplateFile);
}
#endif

#ifndef _WIN32
FILE* (*ResourceManager::orig_fopen)(const char*, const char*) = nullptr;
int (*ResourceManager::orig_open)(const char*, int, mode_t) = nullptr;

FILE* ResourceManager::Hooked_fopen(const char* path, const char* mode) {
    // 应用重定向
    std::string newPath = ResourceManager::instance().ApplyRedirect(path);
    
    return orig_fopen(newPath.c_str(), mode);
}

int ResourceManager::Hooked_open(const char* pathname, int flags, mode_t mode) {
    // 应用重定向
    std::string newPath = ResourceManager::instance().ApplyRedirect(pathname);
    
    return orig_open(newPath.c_str(), flags, mode);
}
#endif

// ==================== ResourceConverter ====================

ResourceConverter::ResourceConverter() {
}

ResourceConverter::~ResourceConverter() {
}

bool ResourceConverter::ConvertTexture(const std::string& inputPath, const std::string& outputPath) {
    std::string ext = fs::path(inputPath).extension().string();
    
    if (ext == ".png") {
        return ConvertPNGToTexture(inputPath, outputPath);
    }
    
    // TODO: 支持更多纹理格式
    return false;
}

bool ResourceConverter::ConvertModel(const std::string& inputPath, const std::string& outputPath) {
    std::string ext = fs::path(inputPath).extension().string();
    
    if (ext == ".obj") {
        return ConvertOBJToModel(inputPath, outputPath);
    }
    
    // TODO: 支持更多模型格式
    return false;
}

bool ResourceConverter::ConvertSound(const std::string& inputPath, const std::string& outputPath) {
    std::string ext = fs::path(inputPath).extension().string();
    
    if (ext == ".wav") {
        return ConvertWAVToSound(inputPath, outputPath);
    }
    
    // TODO: 支持更多声音格式
    return false;
}

bool ResourceConverter::ConvertLang(const std::string& inputPath, const std::string& outputPath) {
    std::string ext = fs::path(inputPath).extension().string();
    
    if (ext == ".json") {
        return ConvertJSONToLang(inputPath, outputPath);
    }
    
    // TODO: 支持更多语言文件格式
    return false;
}

ResourceType ResourceConverter::DetectType(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga") {
        return ResourceType::TEXTURE;
    } else if (ext == ".obj" || ext == ".geo" || ext == ".json") {
        return ResourceType::MODEL;
    } else if (ext == ".ogg" || ext == ".wav" || ext == ".mp3") {
        return ResourceType::SOUND;
    } else if (ext == ".lang" || ext == ".json") {
        return ResourceType::LANG;
    } else if (ext == ".fsh" || ext == ".vsh" || ext == ".gsh") {
        return ResourceType::SHADER;
    } else if (ext == ".py" || ext == ".js") {
        return ResourceType::SCRIPT;
    }
    
    return ResourceType::UNKNOWN;
}

bool ResourceConverter::ConvertPNGToTexture(const std::string& inputPath, const std::string& outputPath) {
    // TODO: 使用stb_image或其他库转换PNG纹理
    // 这里只是简单复制文件
    fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
    return true;
}

bool ResourceConverter::ConvertOBJToModel(const std::string& inputPath, const std::string& outputPath) {
    // TODO: 解析OBJ文件并转换为基岩版模型格式
    // 这里只是简单复制文件
    fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
    return true;
}

bool ResourceConverter::ConvertWAVToSound(const std::string& inputPath, const std::string& outputPath) {
    // TODO: 转换WAV为基岩版支持的音频格式
    // 这里只是简单复制文件
    fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
    return true;
}

bool ResourceConverter::ConvertJSONToLang(const std::string& inputPath, const std::string& outputPath) {
    // TODO: 转换JSON语言文件为基岩版格式
    // 这里只是简单复制文件
    fs::copy_file(inputPath, outputPath, fs::copy_options::overwrite_existing);
    return true;
}

// ==================== ResourcePackManager ====================

ResourcePackManager::ResourcePackManager() {
}

ResourcePackManager::~ResourcePackManager() {
}

bool ResourcePackManager::LoadResourcePack(const std::string& packPath) {
    std::string packId, name, description;
    
    // 解析资源包manifest
    if (!ParsePackManifest(packPath, packId, name, description)) {
        return false;
    }
    
    // 检查是否已加载
    if (loadedPacks_.find(packId) != loadedPacks_.end()) {
        return false;
    }
    
    // 提取资源包内容
    std::string extractDir = "./resource_packs/" + packId;
    if (!ExtractPackResources(packPath, extractDir)) {
        return false;
    }
    
    loadedPacks_[packId] = packPath;
    return true;
}

bool ResourcePackManager::UnloadResourcePack(const std::string& packId) {
    auto it = loadedPacks_.find(packId);
    if (it == loadedPacks_.end()) {
        return false;
    }
    
    // TODO: 移除资源包内容
    std::string extractDir = "./resource_packs/" + packId;
    fs::remove_all(extractDir);
    
    loadedPacks_.erase(it);
    return true;
}

std::vector<std::string> ResourcePackManager::GetLoadedPacks() const {
    std::vector<std::string> packs;
    for (const auto& [id, path] : loadedPacks_) {
        packs.push_back(id);
    }
    return packs;
}

bool ResourcePackManager::ApplyResourcePack(const std::string& packId) {
    auto it = loadedPacks_.find(packId);
    if (it == loadedPacks_.end()) {
        return false;
    }
    
    // TODO: 应用资源包到游戏
    std::string extractDir = "./resource_packs/" + packId;
    
    return true;
}

bool ResourcePackManager::RemoveResourcePack(const std::string& packId) {
    return UnloadResourcePack(packId);
}

bool ResourcePackManager::GetPackInfo(const std::string& packId, std::string& name, std::string& description) {
    auto it = loadedPacks_.find(packId);
    if (it == loadedPacks_.end()) {
        return false;
    }
    
    return ParsePackManifest(it->second, packId, name, description);
}

bool ResourcePackManager::ParsePackManifest(const std::string& packPath, std::string& packId,
                                           std::string& name, std::string& description) {
    // 查找manifest.json
    std::string manifestPath = packPath + "/manifest.json";
    if (!fs::exists(manifestPath)) {
        // 使用默认值
        fs::path path(packPath);
        packId = path.stem().string();
        name = packId;
        description = "Unknown resource pack";
        return true;
    }
    
    // TODO: 解析JSON manifest
    fs::path path(packPath);
    packId = path.stem().string();
    name = packId;
    description = "Loaded from manifest";
    
    return true;
}

bool ResourcePackManager::ExtractPackResources(const std::string& packPath, const std::string& outputDir) {
    // TODO: 解压资源包（.zip, .mcpack等）
    fs::create_directories(outputDir);
    return true;
}

} // namespace resources
} // namespace core
} // namespace mcu
