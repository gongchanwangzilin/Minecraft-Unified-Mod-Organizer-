/**
 * Minecraft Unifier - CMC Format Definition
 * 跨平台模组封装格式
 */

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace mcu {
namespace cmc {

// CMC文件头结构
#pragma pack(push, 1)
struct CMCHeader {
    char magic[4];           // "CMCF" - CMC Format
    uint32_t version;        // 格式版本 (当前: 1)
    uint32_t manifestSize;   // manifest.json大小
    uint32_t fileCount;      // 包含的文件数量
    uint64_t timestamp;      // 创建时间戳
    uint32_t crc32;          // 整体校验和
    uint32_t flags;          // 标志位 (压缩、加密等)
    uint32_t reserved[2];    // 保留字段
};
#pragma pack(pop)

// 文件条目结构
#pragma pack(push, 1)
struct CMCEntry {
    uint32_t nameLen;        // 文件名长度
    uint32_t dataSize;       // 原始数据大小
    uint32_t compressedSize; // 压缩后大小 (0表示未压缩)
    uint32_t flags;          // 文件标志位
    uint64_t offset;         // 数据偏移量
};
#pragma pack(pop)

// 标志位定义
namespace Flags {
    constexpr uint32_t COMPRESSED_ZLIB = 0x01;
    constexpr uint32_t COMPRESSED_LZ4  = 0x02;
    constexpr uint32_t ENCRYPTED_AES   = 0x04;
    constexpr uint32_t EXECUTABLE      = 0x08;
}

// 模组类型
enum class ModType {
    UNKNOWN,
    JAVA_MOD,        // Java版模组
    NETEASE_MOD,     // 网易版模组
    SHADER_PACK,     // 光影包
    RESOURCE_PACK,   // 资源包
    BEHAVIOR_PACK    // 行为包
};

// Manifest结构
struct CMCManifest {
    std::string name;           // 模组名称
    std::string version;        // 模组版本
    std::string description;    // 描述
    std::string author;         // 作者
    ModType type;               // 模组类型
    std::string minGameVersion; // 最低游戏版本
    std::vector<std::string> dependencies; // 依赖项
    std::unordered_map<std::string, std::string> metadata; // 额外元数据
};

// 打包器类
class CMCPacker {
public:
    CMCPacker();
    ~CMCPacker();

    // 打包目录为.cmc文件
    bool Pack(const std::string& inputDir, const std::string& outputFile);
    
    // 解包.cmc文件到目录
    bool Unpack(const std::string& cmcFile, const std::string& outputDir);
    
    // 验证.cmc文件
    bool Validate(const std::string& cmcFile);
    
    // 获取manifest信息
    bool GetManifest(const std::string& cmcFile, CMCManifest& outManifest);
    
    // 设置压缩选项
    void SetCompression(bool enable, int level = 6);
    
    // 设置加密选项
    void SetEncryption(bool enable, const std::string& key = "");

private:
    bool compressionEnabled_;
    int compressionLevel_;
    bool encryptionEnabled_;
    std::string encryptionKey_;
    
    // 内部辅助函数
    bool WriteHeader(FILE* out, const CMCHeader& header);
    bool ReadHeader(FILE* in, CMCHeader& header);
    uint32_t CalculateCRC32(const std::string& data);
    bool CompressData(const std::string& input, std::string& output);
    bool DecompressData(const std::string& input, std::string& output);
};

// 工具函数
std::string ModTypeToString(ModType type);
ModType StringToModType(const std::string& str);
bool ParseManifest(const std::string& json, CMCManifest& outManifest);
std::string SerializeManifest(const CMCManifest& manifest);

} // namespace cmc
} // namespace mcu
