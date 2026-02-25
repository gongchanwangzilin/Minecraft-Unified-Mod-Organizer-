/**
 * Minecraft Unifier - Netease Mod Packer (Windows)
 * 网易模组打包器 - Windows平台
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../common/cmc_format.h"

namespace mcu {
namespace packer {
namespace windows {

// Java模组信息结构
struct JavaModInfo {
    std::string modid;
    std::string name;
    std::string description;
    std::string version;
    std::string mcversion;
    std::string author;
    std::vector<std::string> authors;
    std::string logoFile;
    std::vector<std::string> dependencies;
    std::string loader;  // "forge", "fabric", "neoforge"
    std::map<std::string, std::string> customProperties;
};

// 网易模组转换器
class NeteaseModConverter {
public:
    NeteaseModConverter();
    ~NeteaseModConverter();

    // 转换网易模组为.cmc格式
    bool Convert(const std::string& inputModPath, const std::string& outputCmcPath);
    
    // 设置输出目录
    void SetOutputDir(const std::string& dir);
    
    // 设置API映射配置
    void SetApiMappings(const std::string& configPath);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    std::string outputDir_;
    std::string apiConfigPath_;
    ProgressCallback progressCallback_;
    
    // 内部处理函数
    bool ParseNeteaseMod(const std::string& modPath);
    bool ConvertPythonScripts(const std::string& scriptsDir);
    bool ConvertResources(const std::string& resourcesDir);
    bool GenerateManifest(const std::string& modPath, cmc::CMCManifest& manifest);
    bool CreateCmcPackage(const std::string& outputDir, const std::string& outputPath);
};

// Java模组转换器
class JavaModConverter {
public:
    JavaModConverter();
    ~JavaModConverter();

    // 转换Java模组为.cmc格式
    bool Convert(const std::string& inputJarPath, const std::string& outputCmcPath);
    
    // 设置输出目录
    void SetOutputDir(const std::string& dir);
    
    // 设置API映射配置
    void SetApiMappings(const std::string& configPath);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    std::string outputDir_;
    std::string apiConfigPath_;
    ProgressCallback progressCallback_;
    JavaModInfo modInfo_;  // 存储解析到的模组信息
    
    // 内部处理函数
    bool ExtractJar(const std::string& jarPath, const std::string& extractDir);
    bool ParseJavaMod(const std::string& extractDir);
    bool ParseMcmodInfo(const std::string& configPath);
    bool ParseModsToml(const std::string& configPath);
    bool ParseFabricModJson(const std::string& configPath);
    bool ConvertJavaClasses(const std::string& classesDir);
    bool ConvertResources(const std::string& resourcesDir);
    bool GenerateManifest(const std::string& jarPath, cmc::CMCManifest& manifest);
    bool CreateCmcPackage(const std::string& outputDir, const std::string& outputPath);
    
    // Java字节码转换辅助函数
    bool LoadApiMappings(std::map<std::string, std::string>& mappings);
    std::map<std::string, std::string> GetDefaultApiMappings();
    bool ConvertSingleClass(const std::vector<uint8_t>& bytecode,
                           const std::map<std::string, std::string>& apiMappings,
                           std::vector<uint8_t>& convertedBytecode);
    size_t FindInBytecode(std::vector<uint8_t>& bytecode,
                         const std::string& pattern,
                         size_t startPos);
    
    // 资源转换辅助函数
    bool ConvertTextures(const std::string& assetsDir);
    bool ConvertModels(const std::string& assetsDir);
    bool ConvertLanguages(const std::string& assetsDir);
    bool ConvertSounds(const std::string& assetsDir);
    bool ConvertConfigs(const std::string& resourcesDir);
};

// 光影包转换器
class ShaderPackConverter {
public:
    ShaderPackConverter();
    ~ShaderPackConverter();

    // 转换Java版光影包为.cmc格式
    bool Convert(const std::string& inputShaderPath, const std::string& outputCmcPath);
    
    // 设置输出目录
    void SetOutputDir(const std::string& dir);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    std::string outputDir_;
    ProgressCallback progressCallback_;
    
    // 内部处理函数
    bool ParseShaderPack(const std::string& shaderPath);
    bool ConvertGlslShaders(const std::string& shadersDir);
    bool ConvertProperties(const std::string& configDir);
    bool GenerateManifest(const std::string& shaderPath, cmc::CMCManifest& manifest);
    bool CreateCmcPackage(const std::string& outputDir, const std::string& outputPath);
};

// 统一打包器（整合所有转换器）
class UnifiedPacker {
public:
    UnifiedPacker();
    ~UnifiedPacker();

    // 根据模组类型自动选择转换器
    bool Pack(const std::string& inputPath, const std::string& outputCmcPath);
    
    // 批量打包
    bool BatchPack(const std::vector<std::string>& inputPaths, const std::string& outputDir);
    
    // 设置输出目录
    void SetOutputDir(const std::string& dir);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);

private:
    std::string outputDir_;
    ProgressCallback progressCallback_;
    
    std::unique_ptr<NeteaseModConverter> neteaseConverter_;
    std::unique_ptr<JavaModConverter> javaConverter_;
    std::unique_ptr<ShaderPackConverter> shaderConverter_;
    
    // 检测模组类型
    cmc::ModType DetectModType(const std::string& path);
};

} // namespace windows
} // namespace packer
} // namespace mcu
