/**
 * Minecraft Unifier - CMC Format Implementation
 * 跨平台模组封装格式实现
 */

#include "cmc_format.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <zlib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mcu {
namespace cmc {

CMCPacker::CMCPacker()
    : compressionEnabled_(true)
    , compressionLevel_(6)
    , encryptionEnabled_(false)
    , encryptionKey_("")
{
}

CMCPacker::~CMCPacker() {
}

bool CMCPacker::Pack(const std::string& inputDir, const std::string& outputFile) {
    // 读取manifest.json
    std::string manifestPath = inputDir + "/manifest.json";
    std::ifstream manifestFile(manifestPath);
    if (!manifestFile.is_open()) {
        return false;
    }
    
    std::stringstream manifestBuffer;
    manifestBuffer << manifestFile.rdbuf();
    std::string manifestJson = manifestBuffer.str();
    manifestFile.close();
    
    // 验证manifest
    CMCManifest manifest;
    if (!ParseManifest(manifestJson, manifest)) {
        return false;
    }
    
    // 收集所有文件
    std::vector<std::pair<std::string, std::string>> files; // (相对路径, 绝对路径)
    // TODO: 实现递归目录遍历
    
    // 创建输出文件
    FILE* out = fopen(outputFile.c_str(), "wb");
    if (!out) {
        return false;
    }
    
    // 准备文件头
    CMCHeader header;
    memset(&header, 0, sizeof(header));
    memcpy(header.magic, "CMCF", 4);
    header.version = 1;
    header.manifestSize = manifestJson.size();
    header.fileCount = files.size();
    header.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    header.flags = compressionEnabled_ ? Flags::COMPRESSED_ZLIB : 0;
    
    // 写入文件头（先占位）
    if (!WriteHeader(out, header)) {
        fclose(out);
        return false;
    }
    
    // 写入manifest
    fwrite(manifestJson.c_str(), manifestJson.size(), 1, out);
    
    // 写入文件条目
    for (const auto& file : files) {
        std::string relPath = file.first;
        std::string absPath = file.second;
        
        // 读取文件内容
        std::ifstream fileStream(absPath, std::ios::binary);
        std::stringstream fileBuffer;
        fileBuffer << fileStream.rdbuf();
        std::string fileData = fileBuffer.str();
        fileStream.close();
        
        // 压缩数据（如果启用）
        std::string compressedData;
        if (compressionEnabled_) {
            if (!CompressData(fileData, compressedData)) {
                compressedData = fileData;
            }
        } else {
            compressedData = fileData;
        }
        
        // 写入文件条目
        CMCEntry entry;
        entry.nameLen = relPath.size();
        entry.dataSize = fileData.size();
        entry.compressedSize = compressedData.size();
        entry.flags = compressionEnabled_ ? Flags::COMPRESSED_ZLIB : 0;
        entry.offset = 0; // 将在后续更新
        
        fwrite(&entry, sizeof(entry), 1, out);
        fwrite(relPath.c_str(), relPath.size(), 1, out);
        fwrite(compressedData.c_str(), compressedData.size(), 1, out);
    }
    
    // 计算CRC32
    fseek(out, 0, SEEK_END);
    long fileSize = ftell(out);
    fseek(out, 0, SEEK_SET);
    
    std::vector<char> fileContent(fileSize);
    fread(fileContent.data(), fileSize, 1, out);
    header.crc32 = CalculateCRC32(std::string(fileContent.begin(), fileContent.end()));
    
    // 更新文件头
    fseek(out, 0, SEEK_SET);
    WriteHeader(out, header);
    
    fclose(out);
    return true;
}

bool CMCPacker::Unpack(const std::string& cmcFile, const std::string& outputDir) {
    FILE* in = fopen(cmcFile.c_str(), "rb");
    if (!in) {
        return false;
    }
    
    // 读取文件头
    CMCHeader header;
    if (!ReadHeader(in, header)) {
        fclose(in);
        return false;
    }
    
    // 验证魔数
    if (memcmp(header.magic, "CMCF", 4) != 0) {
        fclose(in);
        return false;
    }
    
    // 读取manifest
    std::string manifestJson;
    manifestJson.resize(header.manifestSize);
    fread(&manifestJson[0], header.manifestSize, 1, in);
    
    // 保存manifest
    std::string manifestPath = outputDir + "/manifest.json";
    std::ofstream manifestOut(manifestPath);
    manifestOut << manifestJson;
    manifestOut.close();
    
    // 读取文件条目
    for (uint32_t i = 0; i < header.fileCount; i++) {
        CMCEntry entry;
        fread(&entry, sizeof(entry), 1, in);
        
        // 读取文件名
        std::string filename;
        filename.resize(entry.nameLen);
        fread(&filename[0], entry.nameLen, 1, in);
        
        // 读取数据
        std::string compressedData;
        compressedData.resize(entry.compressedSize);
        fread(&compressedData[0], entry.compressedSize, 1, in);
        
        // 解压数据（如果需要）
        std::string fileData;
        if (entry.flags & Flags::COMPRESSED_ZLIB) {
            if (!DecompressData(compressedData, fileData)) {
                fileData = compressedData;
            }
        } else {
            fileData = compressedData;
        }
        
        // 保存文件
        std::string fullPath = outputDir + "/" + filename;
        // TODO: 创建目录结构
        std::ofstream fileOut(fullPath, std::ios::binary);
        fileOut.write(fileData.c_str(), fileData.size());
        fileOut.close();
    }
    
    fclose(in);
    return true;
}

bool CMCPacker::Validate(const std::string& cmcFile) {
    FILE* in = fopen(cmcFile.c_str(), "rb");
    if (!in) {
        return false;
    }
    
    CMCHeader header;
    if (!ReadHeader(in, header)) {
        fclose(in);
        return false;
    }
    
    fclose(in);
    
    // 验证魔数
    if (memcmp(header.magic, "CMCF", 4) != 0) {
        return false;
    }
    
    // 验证版本
    if (header.version > 1) {
        return false;
    }
    
    return true;
}

bool CMCPacker::GetManifest(const std::string& cmcFile, CMCManifest& outManifest) {
    FILE* in = fopen(cmcFile.c_str(), "rb");
    if (!in) {
        return false;
    }
    
    CMCHeader header;
    if (!ReadHeader(in, header)) {
        fclose(in);
        return false;
    }
    
    // 读取manifest
    std::string manifestJson;
    manifestJson.resize(header.manifestSize);
    fread(&manifestJson[0], header.manifestSize, 1, in);
    
    fclose(in);
    
    return ParseManifest(manifestJson, outManifest);
}

void CMCPacker::SetCompression(bool enable, int level) {
    compressionEnabled_ = enable;
    compressionLevel_ = level;
}

void CMCPacker::SetEncryption(bool enable, const std::string& key) {
    encryptionEnabled_ = enable;
    encryptionKey_ = key;
}

bool CMCPacker::WriteHeader(FILE* out, const CMCHeader& header) {
    fwrite(&header, sizeof(header), 1, out);
    return true;
}

bool CMCPacker::ReadHeader(FILE* in, CMCHeader& header) {
    fread(&header, sizeof(header), 1, in);
    return true;
}

uint32_t CMCPacker::CalculateCRC32(const std::string& data) {
    return crc32(0, reinterpret_cast<const Bytef*>(data.c_str()), data.size());
}

bool CMCPacker::CompressData(const std::string& input, std::string& output) {
    uLongf compressedSize = compressBound(input.size());
    output.resize(compressedSize);
    
    int result = compress2(
        reinterpret_cast<Bytef*>(&output[0]),
        &compressedSize,
        reinterpret_cast<const Bytef*>(input.c_str()),
        input.size(),
        compressionLevel_
    );
    
    if (result != Z_OK) {
        return false;
    }
    
    output.resize(compressedSize);
    return true;
}

bool CMCPacker::DecompressData(const std::string& input, std::string& output) {
    uLongf decompressedSize = input.size() * 10; // 估算
    output.resize(decompressedSize);
    
    int result = uncompress(
        reinterpret_cast<Bytef*>(&output[0]),
        &decompressedSize,
        reinterpret_cast<const Bytef*>(input.c_str()),
        input.size()
    );
    
    if (result != Z_OK) {
        return false;
    }
    
    output.resize(decompressedSize);
    return true;
}

std::string ModTypeToString(ModType type) {
    switch (type) {
        case ModType::JAVA_MOD: return "java_mod";
        case ModType::NETEASE_MOD: return "netease_mod";
        case ModType::SHADER_PACK: return "shader_pack";
        case ModType::RESOURCE_PACK: return "resource_pack";
        case ModType::BEHAVIOR_PACK: return "behavior_pack";
        default: return "unknown";
    }
}

ModType StringToModType(const std::string& str) {
    if (str == "java_mod") return ModType::JAVA_MOD;
    if (str == "netease_mod") return ModType::NETEASE_MOD;
    if (str == "shader_pack") return ModType::SHADER_PACK;
    if (str == "resource_pack") return ModType::RESOURCE_PACK;
    if (str == "behavior_pack") return ModType::BEHAVIOR_PACK;
    return ModType::UNKNOWN;
}

bool ParseManifest(const std::string& jsonStr, CMCManifest& outManifest) {
    try {
        json j = json::parse(jsonStr);
        
        outManifest.name = j.value("name", "");
        outManifest.version = j.value("version", "");
        outManifest.description = j.value("description", "");
        outManifest.author = j.value("author", "");
        outManifest.type = StringToModType(j.value("type", "unknown"));
        outManifest.minGameVersion = j.value("minGameVersion", "");
        
        if (j.contains("dependencies")) {
            for (const auto& dep : j["dependencies"]) {
                outManifest.dependencies.push_back(dep.get<std::string>());
            }
        }
        
        if (j.contains("metadata")) {
            for (auto& [key, value] : j["metadata"].items()) {
                outManifest.metadata[key] = value.get<std::string>();
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string SerializeManifest(const CMCManifest& manifest) {
    json j;
    j["name"] = manifest.name;
    j["version"] = manifest.version;
    j["description"] = manifest.description;
    j["author"] = manifest.author;
    j["type"] = ModTypeToString(manifest.type);
    j["minGameVersion"] = manifest.minGameVersion;
    j["dependencies"] = manifest.dependencies;
    j["metadata"] = manifest.metadata;
    
    return j.dump(2);
}

} // namespace cmc
} // namespace mcu
