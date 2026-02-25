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
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

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
    // 定义网易API到统一器API的映射规则
    std::vector<std::pair<std::regex, std::string>> apiMappings = {
        // 客户端API映射
        {std::regex(R"(client\.extraClientApi)"), "mcu.client.extraClientApi"},
        {std::regex(R"(client\.getLevel)"), "mcu.client.getWorld"},
        {std::regex(R"(client\.getPlayer)"), "mcu.client.getLocalPlayer"},
        {std::regex(R"(client\.getPlayers)"), "mcu.client.getAllPlayers"},
        {std::regex(R"(client\.getEntities)"), "mcu.client.getAllEntities"},
        {std::regex(R"(client\.getEntity)"), "mcu.client.getEntity"},
        {std::regex(R"(client\.createEntity)"), "mcu.client.spawnEntity"},
        {std::regex(R"(client\.removeEntity)"), "mcu.client.despawnEntity"},
        {std::regex(R"(client\.getDimension)"), "mcu.client.getDimension"},
        {std::regex(R"(client\.getBiome)"), "mcu.client.getBiome"},
        {std::regex(R"(client\.getBlock)"), "mcu.client.getBlock"},
        {std::regex(R"(client\.setBlock)"), "mcu.client.setBlock"},
        {std::regex(R"(client\.getBlocks)"), "mcu.client.getBlocks"},
        {std::regex(R"(client\.setBlocks)"), "mcu.client.setBlocks"},
        {std::regex(R"(client\.getContainer)"), "mcu.client.getContainer"},
        {std::regex(R"(client\.openContainer)"), "mcu.client.openInventory"},
        {std::regex(R"(client\.closeContainer)"), "mcu.client.closeInventory"},
        {std::regex(R"(client\.getInventory)"), "mcu.client.getInventory"},
        {std::regex(R"(client\.getItem)"), "mcu.client.getItem"},
        {std::regex(R"(client\.setItem)"), "mcu.client.setItem"},
        {std::regex(R"(client\.giveItem)"), "mcu.client.giveItem"},
        {std::regex(R"(client\.removeItem)"), "mcu.client.removeItem"},
        {std::regex(R"(client\.getRecipe)"), "mcu.client.getRecipe"},
        {std::regex(R"(client\.getRecipes)"), "mcu.client.getAllRecipes"},
        {std::regex(R"(client\.craftItem)"), "mcu.client.craftItem"},
        {std::regex(R"(client\.getCommand)"), "mcu.client.getCommand"},
        {std::regex(R"(client\.executeCommand)"), "mcu.client.executeCommand"},
        {std::regex(R"(client\.registerCommand)"), "mcu.client.registerCommand"},
        {std::regex(R"(client\.getEvent)"), "mcu.client.getEvent"},
        {std::regex(R"(client\.registerEvent)"), "mcu.client.registerEvent"},
        {std::regex(R"(client\.unregisterEvent)"), "mcu.client.unregisterEvent"},
        {std::regex(R"(client\.getScoreboard)"), "mcu.client.getScoreboard"},
        {std::regex(R"(client\.getTeam)"), "mcu.client.getTeam"},
        {std::regex(R"(client\.getParticle)"), "mcu.client.getParticle"},
        {std::regex(R"(client\.spawnParticle)"), "mcu.client.spawnParticle"},
        {std::regex(R"(client\.getSound)"), "mcu.client.getSound"},
        {std::regex(R"(client\.playSound)"), "mcu.client.playSound"},
        {std::regex(R"(client\.stopSound)"), "mcu.client.stopSound"},
        {std::regex(R"(client\.getCamera)"), "mcu.client.getCamera"},
        {std::regex(R"(client\.setCamera)"), "mcu.client.setCamera"},
        {std::regex(R"(client\.getGui)"), "mcu.client.getGui"},
        {std::regex(R"(client\.openGui)"), "mcu.client.openGui"},
        {std::regex(R"(client\.closeGui)"), "mcu.client.closeGui"},
        {std::regex(R"(client\.getNetwork)"), "mcu.client.getNetwork"},
        {std::regex(R"(client\.sendPacket)"), "mcu.client.sendPacket"},
        {std::regex(R"(client\.receivePacket)"), "mcu.client.receivePacket"},
        
        // 服务器端API映射
        {std::regex(R"(server\.extraServerApi)"), "mcu.server.extraServerApi"},
        {std::regex(R"(server\.getLevel)"), "mcu.server.getWorld"},
        {std::regex(R"(server\.getPlayer)"), "mcu.server.getPlayer"},
        {std::regex(R"(server\.getPlayers)"), "mcu.server.getAllPlayers"},
        {std::regex(R"(server\.getEntities)"), "mcu.server.getAllEntities"},
        {std::regex(R"(server\.getEntity)"), "mcu.server.getEntity"},
        {std::regex(R"(server\.createEntity)"), "mcu.server.spawnEntity"},
        {std::regex(R"(server\.removeEntity)"), "mcu.server.despawnEntity"},
        {std::regex(R"(server\.getDimension)"), "mcu.server.getDimension"},
        {std::regex(R"(server\.getBiome)"), "mcu.server.getBiome"},
        {std::regex(R"(server\.getBlock)"), "mcu.server.getBlock"},
        {std::regex(R"(server\.setBlock)"), "mcu.server.setBlock"},
        {std::regex(R"(server\.getBlocks)"), "mcu.server.getBlocks"},
        {std::regex(R"(server\.setBlocks)"), "mcu.server.setBlocks"},
        {std::regex(R"(server\.getContainer)"), "mcu.server.getContainer"},
        {std::regex(R"(server\.getInventory)"), "mcu.server.getInventory"},
        {std::regex(R"(server\.getItem)"), "mcu.server.getItem"},
        {std::regex(R"(server\.setItem)"), "mcu.server.setItem"},
        {std::regex(R"(server\.giveItem)"), "mcu.server.giveItem"},
        {std::regex(R"(server\.removeItem)"), "mcu.server.removeItem"},
        {std::regex(R"(server\.getRecipe)"), "mcu.server.getRecipe"},
        {std::regex(R"(server\.getRecipes)"), "mcu.server.getAllRecipes"},
        {std::regex(R"(server\.getCommand)"), "mcu.server.getCommand"},
        {std::regex(R"(server\.executeCommand)"), "mcu.server.executeCommand"},
        {std::regex(R"(server\.registerCommand)"), "mcu.server.registerCommand"},
        {std::regex(R"(server\.getEvent)"), "mcu.server.getEvent"},
        {std::regex(R"(server\.registerEvent)"), "mcu.server.registerEvent"},
        {std::regex(R"(server\.unregisterEvent)"), "mcu.server.unregisterEvent"},
        {std::regex(R"(server\.getScoreboard)"), "mcu.server.getScoreboard"},
        {std::regex(R"(server\.getTeam)"), "mcu.server.getTeam"},
        {std::regex(R"(server\.getParticle)"), "mcu.server.getParticle"},
        {std::regex(R"(server\.spawnParticle)"), "mcu.server.spawnParticle"},
        {std::regex(R"(server\.getSound)"), "mcu.server.getSound"},
        {std::regex(R"(server\.playSound)"), "mcu.server.playSound"},
        {std::regex(R"(server\.stopSound)"), "mcu.server.stopSound"},
        {std::regex(R"(server\.getNetwork)"), "mcu.server.getNetwork"},
        {std::regex(R"(server\.sendPacket)"), "mcu.server.sendPacket"},
        {std::regex(R"(server\.broadcastPacket)"), "mcu.server.broadcastPacket"},
        {std::regex(R"(server\.kickPlayer)"), "mcu.server.kickPlayer"},
        {std::regex(R"(server\.banPlayer)"), "mcu.server.banPlayer"},
        {std::regex(R"(server\.unbanPlayer)"), "mcu.server.unbanPlayer"},
        {std::regex(R"(server\.getWhitelist)"), "mcu.server.getWhitelist"},
        {std::regex(R"(server\.addToWhitelist)"), "mcu.server.addToWhitelist"},
        {std::regex(R"(server\.removeFromWhitelist)"), "mcu.server.removeFromWhitelist"},
        {std::regex(R"(server\.getOpList)"), "mcu.server.getOpList"},
        {std::regex(R"(server\.addOp)"), "mcu.server.addOp"},
        {std::regex(R"(server\.removeOp)"), "mcu.server.removeOp"},
        {std::regex(R"(server\.getGameRules)"), "mcu.server.getGameRules"},
        {std::regex(R"(server\.setGameRule)"), "mcu.server.setGameRule"},
        {std::regex(R"(server\.getDifficulty)"), "mcu.server.getDifficulty"},
        {std::regex(R"(server\.setDifficulty)"), "mcu.server.setDifficulty"},
        {std::regex(R"(server\.getGameMode)"), "mcu.server.getGameMode"},
        {std::regex(R"(server\.setGameMode)"), "mcu.server.setGameMode"},
        {std::regex(R"(server\.getTime)"), "mcu.server.getTime"},
        {std::regex(R"(server\.setTime)"), "mcu.server.setTime"},
        {std::regex(R"(server\.getWeather)"), "mcu.server.getWeather"},
        {std::regex(R"(server\.setWeather)"), "mcu.server.setWeather"},
        {std::regex(R"(server\.saveWorld)"), "mcu.server.saveWorld"},
        {std::regex(R"(server\.loadWorld)"), "mcu.server.loadWorld"},
        {std::regex(R"(server\.getWorldInfo)"), "mcu.server.getWorldInfo"},
        
        // 通用API映射
        {std::regex(R"(MinecraftApi)"), "MCUApi"},
        {std::regex(R"(ServerSystem)"), "MCUServerSystem"},
        {std::regex(R"(ClientSystem)"), "MCUClientSystem"},
        {std::regex(R"(ModInfo)"), "MCUModInfo"},
        {std::regex(R"(from minecraft\.api import)"), "from mcu.api import"},
        {std::regex(R"(from minecraft\.server import)"), "from mcu.server import"},
        {std::regex(R"(from minecraft\.client import)"), "from mcu.client import"},
        {std::regex(R"(import minecraft\.api)"), "import mcu.api"},
        {std::regex(R"(import minecraft\.server)"), "import mcu.server"},
        {std::regex(R"(import minecraft\.client)"), "import mcu.client"}
    };
    
    int processedFiles = 0;
    int totalReplacements = 0;
    
    // 遍历所有Python文件
    for (const auto& entry : fs::recursive_directory_iterator(scriptsDir)) {
        if (entry.path().extension() == ".py") {
            std::string filePath = entry.path().string();
            
            // 读取Python文件
            std::ifstream file(filePath);
            if (!file.is_open()) {
                if (progressCallback_) {
                    progressCallback_(0, "无法打开Python文件: " + filePath);
                }
                continue;
            }
            
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();
            
            int fileReplacements = 0;
            
            // 应用所有API映射规则
            for (const auto& [pattern, replacement] : apiMappings) {
                size_t pos = 0;
                while ((pos = content.find(pattern.str(), pos)) != std::string::npos) {
                    content = std::regex_replace(content, pattern, replacement);
                    fileReplacements++;
                    pos += replacement.length();
                }
            }
            
            // 如果有替换，写回文件
            if (fileReplacements > 0) {
                std::ofstream outFile(filePath);
                outFile << content;
                outFile.close();
                totalReplacements += fileReplacements;
            }
            
            processedFiles++;
        }
    }
    
    if (progressCallback_) {
        std::string message = "Python脚本转换完成: 处理 " + std::to_string(processedFiles) + 
                             " 个文件，替换 " + std::to_string(totalReplacements) + " 处API调用";
        progressCallback_(0, message);
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
    // 检查JAR文件是否存在
    if (!fs::exists(jarPath)) {
        if (progressCallback_) {
            progressCallback_(0, "JAR文件不存在: " + jarPath);
        }
        return false;
    }
    
    // 检查是否是有效的JAR文件（ZIP格式）
    std::ifstream jarFile(jarPath, std::ios::binary);
    if (!jarFile.is_open()) {
        if (progressCallback_) {
            progressCallback_(0, "无法打开JAR文件: " + jarPath);
        }
        return false;
    }
    
    // 检查ZIP文件头签名
    uint32_t zipSignature;
    jarFile.read(reinterpret_cast<char*>(&zipSignature), sizeof(zipSignature));
    jarFile.close();
    
    if (zipSignature != 0x04034b50 && zipSignature != 0x504b0304) {
        if (progressCallback_) {
            progressCallback_(0, "无效的JAR文件格式: " + jarPath);
        }
        return false;
    }
    
    // 创建输出目录
    if (!fs::exists(extractDir)) {
        try {
            fs::create_directories(extractDir);
        } catch (const std::exception& e) {
            if (progressCallback_) {
                progressCallback_(0, "创建输出目录失败: " + std::string(e.what()));
            }
            return false;
        }
    }
    
    // 使用unzip命令解压（Linux/macOS）
    #ifdef __linux__
    std::string cmd = "unzip -q -o \"" + jarPath + "\" -d \"" + extractDir + "\" 2>&1";
    int result = system(cmd.c_str());
    if (result == 0) {
        return true;
    }
    #endif
    
    // 使用PowerShell解压（Windows）
    #ifdef _WIN32
    std::string cmd = "powershell -Command \"Expand-Archive -Path '" + jarPath + 
                      "' -DestinationPath '" + extractDir + "' -Force\"";
    int result = system(cmd.c_str());
    if (result == 0) {
        return true;
    }
    #endif
    
    // 如果系统命令失败，尝试使用Java的jar工具
    std::string javaCmd = "jar -xf \"" + jarPath + "\" -C \"" + extractDir + "\" 2>&1";
    int javaResult = system(javaCmd.c_str());
    if (javaResult == 0) {
        return true;
    }
    
    if (progressCallback_) {
        progressCallback_(0, "解压JAR文件失败，请确保已安装unzip或Java");
    }
    return false;
}

bool JavaModConverter::ParseJavaMod(const std::string& extractDir) {
    // 查找mod配置文件
    std::vector<std::pair<std::string, std::string>> configFiles = {
        {"mcmod.info", "forge"},
        {"mods.toml", "forge"},
        {"fabric.mod.json", "fabric"}
    };
    
    // 检查META-INF目录
    std::string metaInfDir = extractDir + "/META-INF";
    if (fs::exists(metaInfDir)) {
        for (const auto& [configFile, loader] : configFiles) {
            std::string configPath = metaInfDir + "/" + configFile;
            if (fs::exists(configPath)) {
                modInfo_.loader = loader;
                if (configFile == "mcmod.info") {
                    return ParseMcmodInfo(configPath);
                } else if (configFile == "mods.toml") {
                    return ParseModsToml(configPath);
                } else if (configFile == "fabric.mod.json") {
                    return ParseFabricModJson(configPath);
                }
            }
        }
    }
    
    // 检查根目录
    for (const auto& [configFile, loader] : configFiles) {
        std::string configPath = extractDir + "/" + configFile;
        if (fs::exists(configPath)) {
            modInfo_.loader = loader;
            if (configFile == "mcmod.info") {
                return ParseMcmodInfo(configPath);
            } else if (configFile == "mods.toml") {
                return ParseModsToml(configPath);
            } else if (configFile == "fabric.mod.json") {
                return ParseFabricModJson(configPath);
            }
        }
    }
    
    // 如果没有找到配置文件，尝试从JAR文件名推断
    if (progressCallback_) {
        progressCallback_(0, "未找到模组配置文件，使用默认值");
    }
    modInfo_.loader = "unknown";
    return true;
}

bool JavaModConverter::ParseMcmodInfo(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        if (progressCallback_) {
            progressCallback_(0, "无法打开mcmod.info文件");
        }
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // mcmod.info使用JSON格式（Forge 1.13+）或自定义格式（旧版Forge）
    try {
        json data = json::parse(content);
        
        // 解析JSON格式的mcmod.info
        if (data.contains("modList")) {
            // 新版Forge格式
            for (const auto& mod : data["modList"]) {
                if (mod.contains("modid")) modInfo_.modid = mod["modid"];
                if (mod.contains("name")) modInfo_.name = mod["name"];
                if (mod.contains("description")) modInfo_.description = mod["description"];
                if (mod.contains("version")) modInfo_.version = mod["version"];
                if (mod.contains("mcversion")) modInfo_.mcversion = mod["mcversion"];
                if (mod.contains("authorList")) {
                    for (const auto& author : mod["authorList"]) {
                        modInfo_.authors.push_back(author);
                    }
                }
                if (mod.contains("logoFile")) modInfo_.logoFile = mod["logoFile"];
                break;  // 只处理第一个模组
            }
        } else {
            // 旧版Forge格式
            if (data.contains("modid")) modInfo_.modid = data["modid"];
            if (data.contains("name")) modInfo_.name = data["name"];
            if (data.contains("description")) modInfo_.description = data["description"];
            if (data.contains("version")) modInfo_.version = data["version"];
            if (data.contains("mcversion")) modInfo_.mcversion = data["mcversion"];
            if (data.contains("author")) modInfo_.author = data["author"];
            if (data.contains("authors")) {
                for (const auto& author : data["authors"]) {
                    modInfo_.authors.push_back(author);
                }
            }
            if (data.contains("logoFile")) modInfo_.logoFile = data["logoFile"];
        }
        
        if (progressCallback_) {
            progressCallback_(0, "成功解析mcmod.info: " + modInfo_.name);
        }
        return true;
        
    } catch (const json::exception& e) {
        // 尝试解析旧版Forge的自定义格式
        std::regex modidRegex(R"(modid\s*=\s*"([^"]+)")");
        std::regex nameRegex(R"(name\s*=\s*"([^"]+)")");
        std::regex versionRegex(R"(version\s*=\s*"([^"]+)")");
        std::regex mcversionRegex(R"(mcversion\s*=\s*"([^"]+)")");
        std::regex authorRegex(R"(author\s*=\s*"([^"]+)")");
        
        std::smatch match;
        if (std::regex_search(content, match, modidRegex)) modInfo_.modid = match[1];
        if (std::regex_search(content, match, nameRegex)) modInfo_.name = match[1];
        if (std::regex_search(content, match, versionRegex)) modInfo_.version = match[1];
        if (std::regex_search(content, match, mcversionRegex)) modInfo_.mcversion = match[1];
        if (std::regex_search(content, match, authorRegex)) modInfo_.author = match[1];
        
        if (progressCallback_) {
            progressCallback_(0, "成功解析旧版mcmod.info: " + modInfo_.name);
        }
        return true;
    }
}

bool JavaModConverter::ParseModsToml(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        if (progressCallback_) {
            progressCallback_(0, "无法打开mods.toml文件");
        }
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // mods.toml使用TOML格式，这里简化处理，实际应使用TOML解析库
    // 这里使用正则表达式提取关键信息
    std::regex modidRegex(R"(modId\s*=\s*"([^"]+)")");
    std::regex displayNameRegex(R"(displayName\s*=\s*"([^"]+)")");
    std::regex descriptionRegex(R"(description\s*=\s*"([^"]+)")");
    std::regex versionRegex(R"(version\s*=\s*"([^"]+)")");
    std::regex authorRegex(R"(author\s*=\s*"([^"]+)")");
    std::regex authorsRegex(R"(authors\s*=\s*"([^"]+)")");
    
    std::smatch match;
    if (std::regex_search(content, match, modidRegex)) modInfo_.modid = match[1];
    if (std::regex_search(content, match, displayNameRegex)) modInfo_.name = match[1];
    if (std::regex_search(content, match, descriptionRegex)) modInfo_.description = match[1];
    if (std::regex_search(content, match, versionRegex)) modInfo_.version = match[1];
    if (std::regex_search(content, match, authorRegex)) modInfo_.author = match[1];
    if (std::regex_search(content, match, authorsRegex)) modInfo_.authors.push_back(match[1]);
    
    // 提取Minecraft版本
    std::regex mcversionRegex(R"(minecraftVersion\s*=\s*"([^"]+)")");
    if (std::regex_search(content, match, mcversionRegex)) modInfo_.mcversion = match[1];
    
    if (progressCallback_) {
        progressCallback_(0, "成功解析mods.toml: " + modInfo_.name);
    }
    return true;
}

bool JavaModConverter::ParseFabricModJson(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        if (progressCallback_) {
            progressCallback_(0, "无法打开fabric.mod.json文件");
        }
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    try {
        json data = json::parse(content);
        
        if (data.contains("id")) modInfo_.modid = data["id"];
        if (data.contains("name")) modInfo_.name = data["name"];
        if (data.contains("description")) modInfo_.description = data["description"];
        if (data.contains("version")) modInfo_.version = data["version"];
        if (data.contains("authors")) {
            if (data["authors"].is_array()) {
                for (const auto& author : data["authors"]) {
                    if (author.is_string()) {
                        modInfo_.authors.push_back(author);
                    } else if (author.is_object() && author.contains("name")) {
                        modInfo_.authors.push_back(author["name"]);
                    }
                }
            }
        }
        if (data.contains("icon")) modInfo_.logoFile = data["icon"];
        
        if (progressCallback_) {
            progressCallback_(0, "成功解析fabric.mod.json: " + modInfo_.name);
        }
        return true;
        
    } catch (const json::exception& e) {
        if (progressCallback_) {
            progressCallback_(0, "解析fabric.mod.json失败: " + std::string(e.what()));
        }
        return false;
    }
}

bool JavaModConverter::ConvertJavaClasses(const std::string& classesDir) {
    if (progressCallback_) {
        progressCallback_(0, "开始转换Java字节码...");
    }
    
    // 检查classes目录是否存在
    if (!fs::exists(classesDir)) {
        if (progressCallback_) {
            progressCallback_(0, "classes目录不存在");
        }
        return true; // 没有Java类文件，不算错误
    }
    
    // 加载API映射配置
    std::map<std::string, std::string> apiMappings;
    if (!LoadApiMappings(apiMappings)) {
        if (progressCallback_) {
            progressCallback_(0, "加载API映射配置失败，使用默认映射");
        }
        // 使用默认映射
        apiMappings = GetDefaultApiMappings();
    }
    
    // 遍历所有.class文件
    int processedCount = 0;
    int totalCount = 0;
    
    for (const auto& entry : fs::recursive_directory_iterator(classesDir)) {
        if (entry.path().extension() == ".class") {
            totalCount++;
        }
    }
    
    if (totalCount == 0) {
        if (progressCallback_) {
            progressCallback_(100, "没有找到.class文件");
        }
        return true;
    }
    
    for (const auto& entry : fs::recursive_directory_iterator(classesDir)) {
        if (entry.path().extension() == ".class") {
            std::string classPath = entry.path().string();
            
            // 读取.class文件
            std::ifstream classFile(classPath, std::ios::binary);
            if (!classFile.is_open()) {
                if (progressCallback_) {
                    progressCallback_(0, "无法读取.class文件: " + classPath);
                }
                continue;
            }
            
            std::vector<uint8_t> bytecode((std::istreambuf_iterator<char>(classFile)),
                                          std::istreambuf_iterator<char>());
            classFile.close();
            
            // 转换字节码
            std::vector<uint8_t> convertedBytecode;
            if (ConvertSingleClass(bytecode, apiMappings, convertedBytecode)) {
                // 写回转换后的字节码
                std::ofstream outFile(classPath, std::ios::binary);
                outFile.write(reinterpret_cast<const char*>(convertedBytecode.data()),
                             convertedBytecode.size());
                outFile.close();
                
                processedCount++;
                
                if (progressCallback_) {
                    int progress = (processedCount * 100) / totalCount;
                    progressCallback_(progress, 
                        "已转换 " + std::to_string(processedCount) + "/" + 
                        std::to_string(totalCount) + " 个类文件");
                }
            }
        }
    }
    
    if (progressCallback_) {
        progressCallback_(100, "Java字节码转换完成，共转换 " + 
                         std::to_string(processedCount) + " 个类文件");
    }
    
    return true;
}

bool JavaModConverter::ConvertResources(const std::string& resourcesDir) {
    if (progressCallback_) {
        progressCallback_(0, "开始转换资源...");
    }
    
    // 检查resources目录是否存在
    if (!fs::exists(resourcesDir)) {
        if (progressCallback_) {
            progressCallback_(0, "resources目录不存在");
        }
        return true; // 没有资源文件，不算错误
    }
    
    int convertedCount = 0;
    
    // 转换纹理文件
    if (fs::exists(resourcesDir + "/assets")) {
        if (progressCallback_) {
            progressCallback_(10, "转换纹理文件...");
        }
        if (ConvertTextures(resourcesDir + "/assets")) {
            convertedCount++;
        }
    }
    
    // 转换模型文件
    if (fs::exists(resourcesDir + "/assets")) {
        if (progressCallback_) {
            progressCallback_(30, "转换模型文件...");
        }
        if (ConvertModels(resourcesDir + "/assets")) {
            convertedCount++;
        }
    }
    
    // 转换语言文件
    if (fs::exists(resourcesDir + "/assets")) {
        if (progressCallback_) {
            progressCallback_(50, "转换语言文件...");
        }
        if (ConvertLanguages(resourcesDir + "/assets")) {
            convertedCount++;
        }
    }
    
    // 转换声音文件
    if (fs::exists(resourcesDir + "/assets")) {
        if (progressCallback_) {
            progressCallback_(70, "转换声音文件...");
        }
        if (ConvertSounds(resourcesDir + "/assets")) {
            convertedCount++;
        }
    }
    
    // 转换配置文件
    if (progressCallback_) {
        progressCallback_(90, "转换配置文件...");
    }
    if (ConvertConfigs(resourcesDir)) {
        convertedCount++;
    }
    
    if (progressCallback_) {
        progressCallback_(100, "资源转换完成，共转换 " + 
                         std::to_string(convertedCount) + " 类资源");
    }
    
    return true;
}

bool JavaModConverter::GenerateManifest(const std::string& jarPath, cmc::CMCManifest& manifest) {
    // 使用解析到的模组信息
    if (!modInfo_.modid.empty()) {
        manifest.name = modInfo_.modid;
    } else {
        fs::path path(jarPath);
        manifest.name = path.stem().string();
    }
    
    if (!modInfo_.version.empty()) {
        manifest.version = modInfo_.version;
    } else {
        manifest.version = "1.0.0";
    }
    
    if (!modInfo_.description.empty()) {
        manifest.description = modInfo_.description;
    } else {
        manifest.description = "Converted from Java mod (" + modInfo_.loader + ")";
    }
    
    // 合并作者信息
    std::string authorStr;
    if (!modInfo_.author.empty()) {
        authorStr = modInfo_.author;
    }
    if (!modInfo_.authors.empty()) {
        for (const auto& author : modInfo_.authors) {
            if (!authorStr.empty()) {
                authorStr += ", ";
            }
            authorStr += author;
        }
    }
    manifest.author = authorStr.empty() ? "Unknown" : authorStr;
    
    manifest.type = cmc::ModType::JAVA_MOD;
    
    // 设置最小游戏版本
    if (!modInfo_.mcversion.empty()) {
        manifest.minGameVersion = modInfo_.mcversion;
    } else {
        manifest.minGameVersion = "1.19.0";
    }
    
    // 添加自定义属性
    if (!modInfo_.loader.empty()) {
        manifest.customProperties["loader"] = modInfo_.loader;
    }
    if (!modInfo_.logoFile.empty()) {
        manifest.customProperties["logoFile"] = modInfo_.logoFile;
    }
    
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

// ==================== JavaModConverter 辅助函数 ====================

bool JavaModConverter::LoadApiMappings(std::map<std::string, std::string>& mappings) {
    // 检查配置文件是否存在
    if (!fs::exists(apiConfigPath_)) {
        return false;
    }
    
    try {
        std::ifstream file(apiConfigPath_);
        json config = json::parse(file);
        file.close();
        
        // 解析API映射
        if (config.contains("api_mappings") && config["api_mappings"].is_object()) {
            for (auto& [key, value] : config["api_mappings"].items()) {
                if (value.is_string()) {
                    mappings[key] = value.get<std::string>();
                }
            }
        }
        
        return true;
        
    } catch (const json::exception& e) {
        return false;
    }
}

std::map<std::string, std::string> JavaModConverter::GetDefaultApiMappings() {
    std::map<std::string, std::string> mappings;
    
    // Minecraft核心API映射
    mappings["net/minecraft/world/World"] = "com/mojang/blaze3d/World";
    mappings["net/minecraft/entity/player/PlayerEntity"] = "com/mojang/blaze3d/entity/Player";
    mappings["net/minecraft/item/Item"] = "com/mojang/blaze3d/item/Item";
    mappings["net/minecraft/block/Block"] = "com/mojang/blaze3d/block/Block";
    
    // Forge API映射
    mappings["net/minecraftforge/event/Event"] = "com/mojang/blaze3d/event/Event";
    mappings["net/minecraftforge/eventbus/api/Event"] = "com/mojang/blaze3d/event/Event";
    mappings["net/minecraftforge/event/SubscribeEvent"] = "com/mojang/blaze3d/event/SubscribeEvent";
    
    // Fabric API映射
    mappings["net/fabricmc/api/ModInitializer"] = "com/mojang/blaze3d/api/ModInitializer";
    mappings["net/fabricmc/api/ClientModInitializer"] = "com/mojang/blaze3d/api/ClientModInitializer";
    
    // 网易版API映射
    mappings["com/netease/minecraft/api/"] = "com/mojang/blaze3d/api/";
    mappings["com/netease/minecraft/world/"] = "com/mojang/blaze3d/world/";
    mappings["com/netease/minecraft/entity/"] = "com/mojang/blaze3d/entity/";
    
    return mappings;
}

bool JavaModConverter::ConvertSingleClass(const std::vector<uint8_t>& bytecode,
                                         const std::map<std::string, std::string>& apiMappings,
                                         std::vector<uint8_t>& convertedBytecode) {
    // 复制原始字节码
    convertedBytecode = bytecode;
    
    // 检查最小class文件大小
    if (bytecode.size() < 32) {
        return false;
    }
    
    // 验证魔数 (0xCAFEBABE)
    if (bytecode[0] != 0xCA || bytecode[1] != 0xFE || 
        bytecode[2] != 0xBA || bytecode[3] != 0xBE) {
        return false;
    }
    
    // 读取常量池数量（小端序）
    uint16_t constantPoolCount = (bytecode[8] << 8) | bytecode[9];
    
    // 简单实现：在字节码中查找并替换类名引用
    // 注意：这是一个简化的实现，完整的字节码转换需要更复杂的解析
    
    for (const auto& [oldApi, newApi] : apiMappings) {
        // 将Java类名转换为内部格式（/替换.）
        std::string oldInternal = std::regex_replace(oldApi, std::regex("\\."), "/");
        std::string newInternal = std::regex_replace(newApi, std::regex("\\."), "/");
        
        // 在字节码中查找并替换
        size_t pos = 0;
        while ((pos = FindInBytecode(convertedBytecode, oldInternal, pos)) != std::string::npos) {
            // 替换
            for (size_t i = 0; i < newInternal.size() && i < oldInternal.size(); i++) {
                convertedBytecode[pos + i] = static_cast<uint8_t>(newInternal[i]);
            }
            pos += newInternal.size();
        }
    }
    
    return true;
}

size_t JavaModConverter::FindInBytecode(std::vector<uint8_t>& bytecode,
                                       const std::string& pattern,
                                       size_t startPos) {
    if (pattern.empty() || startPos >= bytecode.size()) {
        return std::string::npos;
    }
    
    for (size_t i = startPos; i <= bytecode.size() - pattern.size(); i++) {
        bool match = true;
        for (size_t j = 0; j < pattern.size(); j++) {
            if (bytecode[i + j] != static_cast<uint8_t>(pattern[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            return i;
        }
    }
    
    return std::string::npos;
}

// ==================== 资源转换辅助函数 ====================

bool JavaModConverter::ConvertTextures(const std::string& assetsDir) {
    // 遍历所有纹理文件
    for (const auto& entry : fs::recursive_directory_iterator(assetsDir)) {
        std::string ext = entry.path().extension().string();
        
        // 支持的纹理格式
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
            std::string filePath = entry.path().string();
            
            // 检查是否在textures目录下
            if (filePath.find("/textures/") != std::string::npos) {
                // 基岩版纹理路径转换
                // Java版: assets/modid/textures/block/stone.png
                // 基岩版: textures/blocks/stone.png
                
                std::string newPath = filePath;
                
                // 移除modid命名空间
                size_t assetsPos = newPath.find("/assets/");
                if (assetsPos != std::string::npos) {
                    size_t texturesPos = newPath.find("/textures/", assetsPos);
                    if (texturesPos != std::string::npos) {
                        // 提取textures之后的路径
                        std::string texturePath = newPath.substr(texturesPos + 1);
                        
                        // 转换路径格式
                        texturePath = std::regex_replace(texturePath, 
                            std::regex("/block/"), "/blocks/");
                        texturePath = std::regex_replace(texturePath, 
                            std::regex("/item/"), "/items/");
                        texturePath = std::regex_replace(texturePath, 
                            std::regex("/entity/"), "/entity/");
                        
                        // 创建新路径
                        std::string targetDir = entry.path().parent_path().string();
                        targetDir = std::regex_replace(targetDir, 
                            std::regex("/assets/[^/]+/"), "/textures/");
                        
                        fs::create_directories(targetDir);
                        std::string targetPath = targetDir + "/" + 
                                                entry.path().filename().string();
                        
                        // 移动文件
                        if (filePath != targetPath) {
                            fs::rename(filePath, targetPath);
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

bool JavaModConverter::ConvertModels(const std::string& assetsDir) {
    // 遍历所有模型文件
    for (const auto& entry : fs::recursive_directory_iterator(assetsDir)) {
        if (entry.path().extension() == ".json") {
            std::string filePath = entry.path().string();
            
            // 检查是否在models目录下
            if (filePath.find("/models/") != std::string::npos) {
                // 读取JSON文件
                std::ifstream file(filePath);
                if (!file.is_open()) {
                    continue;
                }
                
                try {
                    json model = json::parse(file);
                    file.close();
                    
                    // 转换Java版模型格式为基岩版格式
                    // Java版使用JSON格式，基岩版也使用JSON但结构不同
                    
                    if (model.contains("parent")) {
                        std::string parent = model["parent"];
                        // 转换parent路径
                        parent = std::regex_replace(parent, 
                            std::regex("minecraft:block/"), "blocks/");
                        parent = std::regex_replace(parent, 
                            std::regex("minecraft:item/"), "items/");
                        model["parent"] = parent;
                    }
                    
                    if (model.contains("textures")) {
                        auto& textures = model["textures"];
                        for (auto& [key, value] : textures.items()) {
                            if (value.is_string()) {
                                std::string texPath = value.get<std::string>();
                                texPath = std::regex_replace(texPath, 
                                    std::regex("minecraft:block/"), "blocks/");
                                texPath = std::regex_replace(texPath, 
                                    std::regex("minecraft:item/"), "items/");
                                textures[key] = texPath;
                            }
                        }
                    }
                    
                    // 写回文件
                    std::ofstream outFile(filePath);
                    outFile << model.dump(4);
                    outFile.close();
                    
                } catch (const json::exception& e) {
                    // JSON解析失败，跳过
                    continue;
                }
            }
        }
    }
    
    return true;
}

bool JavaModConverter::ConvertLanguages(const std::string& assetsDir) {
    // 遍历所有语言文件
    for (const auto& entry : fs::recursive_directory_iterator(assetsDir)) {
        if (entry.path().extension() == ".json") {
            std::string filePath = entry.path().string();
            
            // 检查是否在lang目录下
            if (filePath.find("/lang/") != std::string::npos) {
                // 读取JSON文件
                std::ifstream file(filePath);
                if (!file.is_open()) {
                    continue;
                }
                
                try {
                    json lang = json::parse(file);
                    file.close();
                    
                    // 语言文件格式基本兼容，但需要调整键名
                    // Java版: block.minecraft.stone = Stone
                    // 基岩版: tile.stone.name = Stone
                    
                    json convertedLang;
                    for (auto& [key, value] : lang.items()) {
                        if (value.is_string()) {
                            std::string newKey = key;
                            
                            // 转换键名
                            newKey = std::regex_replace(newKey, 
                                std::regex("block\\.minecraft\\."), "tile.");
                            newKey = std::regex_replace(newKey, 
                                std::regex("item\\.minecraft\\."), "item.");
                            newKey = std::regex_replace(newKey, 
                                std::regex("entity\\.minecraft\\."), "entity.");
                            
                            convertedLang[newKey] = value;
                        }
                    }
                    
                    // 写回文件
                    std::ofstream outFile(filePath);
                    outFile << convertedLang.dump(4);
                    outFile.close();
                    
                } catch (const json::exception& e) {
                    // JSON解析失败，跳过
                    continue;
                }
            }
        }
    }
    
    return true;
}

bool JavaModConverter::ConvertSounds(const std::string& assetsDir) {
    // 遍历所有声音文件
    for (const auto& entry : fs::recursive_directory_iterator(assetsDir)) {
        std::string ext = entry.path().extension().string();
        
        // 支持的声音格式
        if (ext == ".ogg" || ext == ".mp3" || ext == ".wav") {
            std::string filePath = entry.path().string();
            
            // 检查是否在sounds目录下
            if (filePath.find("/sounds/") != std::string::npos) {
                // 声音文件格式基本兼容，但可能需要转换
                // 基岩版主要使用.ogg格式
                
                if (ext == ".mp3" || ext == ".wav") {
                    // TODO: 转换为.ogg格式
                    // 这里需要使用音频转换库如libavcodec
                }
            }
        }
    }
    
    // 转换sounds.json配置文件
    std::string soundsJsonPath = assetsDir + "/minecraft/sounds.json";
    if (fs::exists(soundsJsonPath)) {
        std::ifstream file(soundsJsonPath);
        if (file.is_open()) {
            try {
                json sounds = json::parse(file);
                file.close();
                
                // 转换声音配置
                // Java版和基岩版的声音配置格式基本兼容
                
                std::ofstream outFile(soundsJsonPath);
                outFile << sounds.dump(4);
                outFile.close();
                
            } catch (const json::exception& e) {
                // JSON解析失败，跳过
            }
        }
    }
    
    return true;
}

bool JavaModConverter::ConvertConfigs(const std::string& resourcesDir) {
    // 遍历所有配置文件
    for (const auto& entry : fs::recursive_directory_iterator(resourcesDir)) {
        std::string ext = entry.path().extension().string();
        std::string filePath = entry.path().string();
        
        // 转换Forge配置文件
        if (ext == ".cfg" || ext == ".toml") {
            // Forge配置文件格式
            // 基岩版使用不同的配置格式，可能需要转换
        }
        
        // 转换Fabric配置文件
        if (entry.path().filename() == "fabric.mod.json") {
            // Fabric配置文件已经在ParseFabricModJson中处理
        }
        
        // 转换其他JSON配置文件
        if (ext == ".json") {
            std::ifstream file(filePath);
            if (file.is_open()) {
                try {
                    json config = json::parse(file);
                    file.close();
                    
                    // 根据配置类型进行转换
                    if (config.contains("type")) {
                        std::string type = config["type"];
                        if (type == "forge:config") {
                            // Forge配置转换
                        } else if (type == "fabric:config") {
                            // Fabric配置转换
                        }
                    }
                    
                    std::ofstream outFile(filePath);
                    outFile << config.dump(4);
                    outFile.close();
                    
                } catch (const json::exception& e) {
                    // JSON解析失败，跳过
                }
            }
        }
    }
    
    return true;
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
