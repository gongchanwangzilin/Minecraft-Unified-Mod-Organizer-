/**
 * Minecraft Unifier - Netease Mod Packer Implementation (Windows)
 * 网易模组打包器实现 - Windows平台
 */

#include "netease_packer.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>

namespace fs = std::filesystem;

namespace mcu {
namespace packer {
namespace windows {

// ==================== NeteaseModConverter ====================

NeteaseModConverter::NeteaseModConverter()
    : outputDir_("./output")
    , apiConfigPath_("./api_mappings.json")
{
}

NeteaseModConverter::~NeteaseModConverter() {
}

void NeteaseModConverter::SetOutputDir(const std::string& dir) {
    outputDir_ = dir;
}

void NeteaseModConverter::SetApiMappings(const std::string& configPath) {
    apiConfigPath_ = configPath;
}

void NeteaseModConverter::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool NeteaseModConverter::Convert(const std::string& inputModPath, const std::string& outputCmcPath) {
    if (progressCallback_) {
        progressCallback_(0, "开始转换网易模组...");
    }
    
    // 创建临时工作目录
    std::string workDir = outputDir_ + "/temp_netease_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(workDir);
    
    // 解析网易模组
    if (!ParseNeteaseMod(inputModPath)) {
        if (progressCallback_) {
            progressCallback_(0, "解析网易模组失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(30, "解析模组完成");
    }
    
    // 转换Python脚本
    std::string scriptsDir = workDir + "/scripts";
    if (fs::exists(scriptsDir)) {
        if (!ConvertPythonScripts(scriptsDir)) {
            if (progressCallback_) {
                progressCallback_(30, "转换Python脚本失败");
            }
            return false;
        }
    }
    
    if (progressCallback_) {
        progressCallback_(60, "Python脚本转换完成");
    }
    
    // 转换资源
    std::string resourcesDir = workDir + "/resources";
    if (fs::exists(resourcesDir)) {
        if (!ConvertResources(resourcesDir)) {
            if (progressCallback_) {
                progressCallback_(60, "转换资源失败");
            }
            return false;
        }
    }
    
    if (progressCallback_) {
        progressCallback_(80, "资源转换完成");
    }
    
    // 生成manifest
    cmc::CMCManifest manifest;
    if (!GenerateManifest(inputModPath, manifest)) {
        if (progressCallback_) {
            progressCallback_(80, "生成manifest失败");
        }
        return false;
    }
    
    // 保存manifest
    std::ofstream manifestFile(workDir + "/manifest.json");
    manifestFile << cmc::SerializeManifest(manifest);
    manifestFile.close();
    
    if (progressCallback_) {
        progressCallback_(90, "生成.cmc包...");
    }
    
    // 创建.cmc包
    if (!CreateCmcPackage(workDir, outputCmcPath)) {
        if (progressCallback_) {
            progressCallback_(90, "创建.cmc包失败");
        }
        return false;
    }
    
    // 清理临时目录
    fs::remove_all(workDir);
    
    if (progressCallback_) {
        progressCallback_(100, "转换完成！");
    }
    
    return true;
}

bool NeteaseModConverter::ParseNeteaseMod(const std::string& modPath) {
    // 检查是否是目录或压缩包
    if (fs::is_directory(modPath)) {
        // 直接处理目录
        return true;
    }
    
    // TODO: 解压压缩包（.zip, .rar等）
    return true;
}

bool NeteaseModConverter::ConvertPythonScripts(const std::string& scriptsDir) {
    // 遍历所有Python文件
    for (const auto& entry : fs::recursive_directory_iterator(scriptsDir)) {
        if (entry.path().extension() == ".py") {
            std::string filePath = entry.path().string();
            
            // 读取Python文件
            std::ifstream file(filePath);
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();
            
            // 转换网易API调用为统一器API
            // TODO: 实现API映射转换
            content = std::regex_replace(content, 
                std::regex("client\\.extraClientApi"), 
                "mcu.client.extraClientApi");
            content = std::regex_replace(content, 
                std::regex("server\\.extraServerApi"), 
                "mcu.server.extraServerApi");
            
            // 写回文件
            std::ofstream outFile(filePath);
            outFile << content;
            outFile.close();
        }
    }
    
    return true;
}

bool NeteaseModConverter::ConvertResources(const std::string& resourcesDir) {
    // 转换纹理、模型等资源
    // TODO: 实现资源格式转换
    return true;
}

bool NeteaseModConverter::GenerateManifest(const std::string& modPath, cmc::CMCManifest& manifest) {
    // 从模组路径提取信息
    fs::path path(modPath);
    manifest.name = path.stem().string();
    manifest.version = "1.0.0";
    manifest.description = "Converted from Netease mod";
    manifest.author = "Unknown";
    manifest.type = cmc::ModType::NETEASE_MOD;
    manifest.minGameVersion = "1.19.0";
    
    // TODO: 从mod.json或其他配置文件读取详细信息
    
    return true;
}

bool NeteaseModConverter::CreateCmcPackage(const std::string& outputDir, const std::string& outputPath) {
    cmc::CMCPacker packer;
    packer.SetCompression(true, 6);
    return packer.Pack(outputDir, outputPath);
}

// ==================== JavaModConverter ====================

JavaModConverter::JavaModConverter()
    : outputDir_("./output")
    , apiConfigPath_("./api_mappings.json")
{
}

JavaModConverter::~JavaModConverter() {
}

void JavaModConverter::SetOutputDir(const std::string& dir) {
    outputDir_ = dir;
}

void JavaModConverter::SetApiMappings(const std::string& configPath) {
    apiConfigPath_ = configPath;
}

void JavaModConverter::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool JavaModConverter::Convert(const std::string& inputJarPath, const std::string& outputCmcPath) {
    if (progressCallback_) {
        progressCallback_(0, "开始转换Java模组...");
    }
    
    // 创建临时工作目录
    std::string workDir = outputDir_ + "/temp_java_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(workDir);
    
    // 解压JAR文件
    if (!ExtractJar(inputJarPath, workDir)) {
        if (progressCallback_) {
            progressCallback_(0, "解压JAR文件失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(30, "JAR文件解压完成");
    }
    
    // 解析Java模组
    if (!ParseJavaMod(workDir)) {
        if (progressCallback_) {
            progressCallback_(30, "解析Java模组失败");
        }
        return false;
    }
    
    // 转换Java类
    std::string classesDir = workDir;
    if (fs::exists(classesDir)) {
        if (!ConvertJavaClasses(classesDir)) {
            if (progressCallback_) {
                progressCallback_(60, "转换Java类失败");
            }
            return false;
        }
    }
    
    if (progressCallback_) {
        progressCallback_(60, "Java类转换完成");
    }
    
    // 转换资源
    std::string resourcesDir = workDir + "/resources";
    if (fs::exists(resourcesDir)) {
        if (!ConvertResources(resourcesDir)) {
            if (progressCallback_) {
                progressCallback_(80, "转换资源失败");
            }
            return false;
        }
    }
    
    if (progressCallback_) {
        progressCallback_(80, "资源转换完成");
    }
    
    // 生成manifest
    cmc::CMCManifest manifest;
    if (!GenerateManifest(inputJarPath, manifest)) {
        if (progressCallback_) {
            progressCallback_(80, "生成manifest失败");
        }
        return false;
    }
    
    // 保存manifest
    std::ofstream manifestFile(workDir + "/manifest.json");
    manifestFile << cmc::SerializeManifest(manifest);
    manifestFile.close();
    
    if (progressCallback_) {
        progressCallback_(90, "生成.cmc包...");
    }
    
    // 创建.cmc包
    if (!CreateCmcPackage(workDir, outputCmcPath)) {
        if (progressCallback_) {
            progressCallback_(90, "创建.cmc包失败");
        }
        return false;
    }
    
    // 清理临时目录
    fs::remove_all(workDir);
    
    if (progressCallback_) {
        progressCallback_(100, "转换完成！");
    }
    
    return true;
}

bool JavaModConverter::ExtractJar(const std::string& jarPath, const std::string& extractDir) {
    // TODO: 使用unzip或Java的jar工具解压
    std::string cmd = "unzip -q \"" + jarPath + "\" -d \"" + extractDir + "\"";
    return system(cmd.c_str()) == 0;
}

bool JavaModConverter::ParseJavaMod(const std::string& extractDir) {
    // 查找mod配置文件
    std::vector<std::string> configFiles = {
        "mcmod.info", "mods.toml", "fabric.mod.json"
    };
    
    for (const auto& configFile : configFiles) {
        std::string configPath = extractDir + "/" + configFile;
        if (fs::exists(configPath)) {
            // TODO: 解析配置文件
            return true;
        }
    }
    
    return true;
}

bool JavaModConverter::ConvertJavaClasses(const std::string& classesDir) {
    // TODO: 实现Java字节码转换
    // 这里需要将Java API调用转换为基岩版API调用
    return true;
}

bool JavaModConverter::ConvertResources(const std::string& resourcesDir) {
    // 转换资源
    return true;
}

bool JavaModConverter::GenerateManifest(const std::string& jarPath, cmc::CMCManifest& manifest) {
    fs::path path(jarPath);
    manifest.name = path.stem().string();
    manifest.version = "1.0.0";
    manifest.description = "Converted from Java mod";
    manifest.author = "Unknown";
    manifest.type = cmc::ModType::JAVA_MOD;
    manifest.minGameVersion = "1.19.0";
    
    return true;
}

bool JavaModConverter::CreateCmcPackage(const std::string& outputDir, const std::string& outputPath) {
    cmc::CMCPacker packer;
    packer.SetCompression(true, 6);
    return packer.Pack(outputDir, outputPath);
}

// ==================== ShaderPackConverter ====================

ShaderPackConverter::ShaderPackConverter()
    : outputDir_("./output")
{
}

ShaderPackConverter::~ShaderPackConverter() {
}

void ShaderPackConverter::SetOutputDir(const std::string& dir) {
    outputDir_ = dir;
}

void ShaderPackConverter::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool ShaderPackConverter::Convert(const std::string& inputShaderPath, const std::string& outputCmcPath) {
    if (progressCallback_) {
        progressCallback_(0, "开始转换光影包...");
    }
    
    // 创建临时工作目录
    std::string workDir = outputDir_ + "/temp_shader_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(workDir);
    
    // 解析光影包
    if (!ParseShaderPack(inputShaderPath)) {
        if (progressCallback_) {
            progressCallback_(0, "解析光影包失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(30, "光影包解析完成");
    }
    
    // 转换GLSL着色器
    std::string shadersDir = inputShaderPath + "/shaders";
    if (fs::exists(shadersDir)) {
        if (!ConvertGlslShaders(shadersDir)) {
            if (progressCallback_) {
                progressCallback_(30, "转换GLSL着色器失败");
            }
            return false;
        }
    }
    
    if (progressCallback_) {
        progressCallback_(60, "GLSL着色器转换完成");
    }
    
    // 转换配置文件
    std::string configDir = inputShaderPath;
    if (fs::exists(configDir)) {
        if (!ConvertProperties(configDir)) {
            if (progressCallback_) {
                progressCallback_(60, "转换配置文件失败");
            }
            return false;
        }
    }
    
    if (progressCallback_) {
        progressCallback_(80, "配置文件转换完成");
    }
    
    // 生成manifest
    cmc::CMCManifest manifest;
    if (!GenerateManifest(inputShaderPath, manifest)) {
        if (progressCallback_) {
            progressCallback_(80, "生成manifest失败");
        }
        return false;
    }
    
    // 保存manifest
    std::ofstream manifestFile(workDir + "/manifest.json");
    manifestFile << cmc::SerializeManifest(manifest);
    manifestFile.close();
    
    if (progressCallback_) {
        progressCallback_(90, "生成.cmc包...");
    }
    
    // 创建.cmc包
    if (!CreateCmcPackage(workDir, outputCmcPath)) {
        if (progressCallback_) {
            progressCallback_(90, "创建.cmc包失败");
        }
        return false;
    }
    
    // 清理临时目录
    fs::remove_all(workDir);
    
    if (progressCallback_) {
        progressCallback_(100, "转换完成！");
    }
    
    return true;
}

bool ShaderPackConverter::ParseShaderPack(const std::string& shaderPath) {
    return fs::exists(shaderPath);
}

bool ShaderPackConverter::ConvertGlslShaders(const std::string& shadersDir) {
    // 遍历所有着色器文件
    for (const auto& entry : fs::recursive_directory_iterator(shadersDir)) {
        std::string ext = entry.path().extension().string();
        if (ext == ".vsh" || ext == ".fsh" || ext == ".gsh") {
            std::string filePath = entry.path().string();
            
            // 读取着色器文件
            std::ifstream file(filePath);
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();
            
            // TODO: 转换GLSL代码为Render Dragon兼容格式
            // 这里需要将Java版GLSL转换为Render Dragon支持的格式
            
            // 写回文件
            std::ofstream outFile(filePath);
            outFile << content;
            outFile.close();
        }
    }
    
    return true;
}

bool ShaderPackConverter::ConvertProperties(const std::string& configDir) {
    // 转换配置文件
    return true;
}

bool ShaderPackConverter::GenerateManifest(const std::string& shaderPath, cmc::CMCManifest& manifest) {
    fs::path path(shaderPath);
    manifest.name = path.stem().string();
    manifest.version = "1.0.0";
    manifest.description = "Converted shader pack";
    manifest.author = "Unknown";
    manifest.type = cmc::ModType::SHADER_PACK;
    manifest.minGameVersion = "1.19.0";
    
    return true;
}

bool ShaderPackConverter::CreateCmcPackage(const std::string& outputDir, const std::string& outputPath) {
    cmc::CMCPacker packer;
    packer.SetCompression(true, 6);
    return packer.Pack(outputDir, outputPath);
}

// ==================== UnifiedPacker ====================

UnifiedPacker::UnifiedPacker()
    : outputDir_("./output")
    , neteaseConverter_(std::make_unique<NeteaseModConverter>())
    , javaConverter_(std::make_unique<JavaModConverter>())
    , shaderConverter_(std::make_unique<ShaderPackConverter>())
{
}

UnifiedPacker::~UnifiedPacker() {
}

void UnifiedPacker::SetOutputDir(const std::string& dir) {
    outputDir_ = dir;
    neteaseConverter_->SetOutputDir(dir);
    javaConverter_->SetOutputDir(dir);
    shaderConverter_->SetOutputDir(dir);
}

void UnifiedPacker::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
    neteaseConverter_->SetProgressCallback(callback);
    javaConverter_->SetProgressCallback(callback);
    shaderConverter_->SetProgressCallback(callback);
}

cmc::ModType UnifiedPacker::DetectModType(const std::string& path) {
    fs::path p(path);
    std::string ext = p.extension().string();
    
    if (ext == ".py" || fs::exists(path + "/scripts")) {
        return cmc::ModType::NETEASE_MOD;
    } else if (ext == ".jar") {
        return cmc::ModType::JAVA_MOD;
    } else if (fs::exists(path + "/shaders")) {
        return cmc::ModType::SHADER_PACK;
    } else if (fs::exists(path + "/assets")) {
        return cmc::ModType::RESOURCE_PACK;
    }
    
    return cmc::ModType::UNKNOWN;
}

bool UnifiedPacker::Pack(const std::string& inputPath, const std::string& outputCmcPath) {
    cmc::ModType type = DetectModType(inputPath);
    
    switch (type) {
        case cmc::ModType::NETEASE_MOD:
            return neteaseConverter_->Convert(inputPath, outputCmcPath);
        case cmc::ModType::JAVA_MOD:
            return javaConverter_->Convert(inputPath, outputCmcPath);
        case cmc::ModType::SHADER_PACK:
            return shaderConverter_->Convert(inputPath, outputCmcPath);
        default:
            if (progressCallback_) {
                progressCallback_(0, "无法识别的模组类型");
            }
            return false;
    }
}

bool UnifiedPacker::BatchPack(const std::vector<std::string>& inputPaths, const std::string& outputDir) {
    fs::create_directories(outputDir);
    
    int successCount = 0;
    for (size_t i = 0; i < inputPaths.size(); i++) {
        fs::path inputPath(inputPaths[i]);
        std::string outputPath = outputDir + "/" + inputPath.stem().string() + ".cmc";
        
        if (Pack(inputPaths[i], outputPath)) {
            successCount++;
        }
    }
    
    return successCount == inputPaths.size();
}

} // namespace windows
} // namespace packer
} // namespace mcu
