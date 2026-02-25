/**
 * Minecraft Unifier - Packer Tests
 * 打包器测试用例
 */

#include <gtest/gtest.h>
#include <packer/windows/java_packer.h>
#include <packer/windows/netease_packer.h>
#include <packer/windows/shader_packer.h>
#include <packer/windows/unified_packer.h>
#include <common/cmc_format.h>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mcu {
namespace packer {
namespace test {

class PackerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录
        test_dir_ = "/tmp/minecraft-unifier-test";
        std::filesystem::create_directories(test_dir_);
        
        // 创建临时目录
        temp_dir_ = test_dir_ + "/temp";
        std::filesystem::create_directories(temp_dir_);
        
        // 创建输出目录
        output_dir_ = test_dir_ + "/output";
        std::filesystem::create_directories(output_dir_);
    }
    
    void TearDown() override {
        // 清理测试目录
        std::filesystem::remove_all(test_dir_);
    }
    
    // 创建测试用的Java模组JAR文件
    std::string CreateTestJavaMod(const std::string& mod_id, const std::string& mod_name, const std::string& version) {
        std::string mod_dir = temp_dir_ + "/" + mod_id;
        std::filesystem::create_directories(mod_dir);
        
        // 创建mcmod.info（Forge旧格式）
        std::ofstream mcmod_info(mod_dir + "/mcmod.info");
        mcmod_info << "{\n";
        mcmod_info << "  \"modid\": \"" << mod_id << "\",\n";
        mcmod_info << "  \"name\": \"" << mod_name << "\",\n";
        mcmod_info << "  \"version\": \"" << version << "\",\n";
        mcmod_info << "  \"description\": \"Test mod for Minecraft Unifier\",\n";
        mcmod_info << "  \"mcversion\": \"1.12.2\",\n";
        mcmod_info << "  \"authorList\": [\"Test Author\"]\n";
        mcmod_info << "}\n";
        mcmod_info.close();
        
        // 创建主类文件
        std::string class_dir = mod_dir + "/" + mod_id;
        std::filesystem::create_directories(class_dir);
        
        std::ofstream main_class(class_dir + "/ModMain.class");
        main_class << "CAFEBABE0000003400"; // Java class文件魔数
        main_class.close();
        
        // 打包为JAR文件
        std::string jar_path = output_dir_ + "/" + mod_id + ".jar";
        std::string cmd = "cd " + mod_dir + " && jar cf " + jar_path + " mcmod.info " + mod_id + "/ModMain.class";
        system(cmd.c_str());
        
        return jar_path;
    }
    
    // 创建测试用的网易模组
    std::string CreateTestNeteaseMod(const std::string& mod_id, const std::string& mod_name, const std::string& version) {
        std::string mod_dir = temp_dir_ + "/" + mod_id;
        std::filesystem::create_directories(mod_dir);
        
        // 创建mod.json
        std::ofstream mod_json(mod_dir + "/mod.json");
        mod_json << "{\n";
        mod_json << "  \"id\": \"" << mod_id << "\",\n";
        mod_json << "  \"name\": \"" << mod_name << "\",\n";
        mod_json << "  \"version\": \"" << version << "\",\n";
        mod_json << "  \"description\": \"Test mod for Minecraft Unifier\",\n";
        mod_json << "  \"dependencies\": [],\n";
        mod_json << "  \"type\": \"client\"\n";
        mod_json << "}\n";
        mod_json.close();
        
        // 创建mod.py
        std::ofstream mod_py(mod_dir + "/mod.py");
        mod_py << "import client.extraClientApi as clientApi\n";
        mod_py << "\n";
        mod_py << "MOD_ID = \"" << mod_id << "\"\n";
        mod_py << "MOD_NAME = \"" << mod_name << "\"\n";
        mod_py << "MOD_VERSION = \"" << version << "\"\n";
        mod_py << "MOD_DESCRIPTION = \"Test mod for Minecraft Unifier\"\n";
        mod_py << "DEPENDENCIES = []\n";
        mod_py << "\n";
        mod_py << "def init():\n";
        mod_py << "    print(\"Mod initialized: \" + MOD_NAME)\n";
        mod_py << "\n";
        mod_py << "def on_load():\n";
        mod_py << "    print(\"Mod loaded: \" + MOD_NAME)\n";
        mod_py << "\n";
        mod_py << "def on_unload():\n";
        mod_py << "    print(\"Mod unloaded: \" + MOD_NAME)\n";
        mod_py.close();
        
        return mod_dir;
    }
    
    // 创建测试用的光影包
    std::string CreateTestShaderPack(const std::string& pack_name) {
        std::string pack_dir = temp_dir_ + "/" + pack_name;
        std::filesystem::create_directories(pack_dir + "/shaders");
        
        // 创建pack.mcmeta
        std::ofstream pack_mcmeta(pack_dir + "/pack.mcmeta");
        pack_mcmeta << "{\n";
        pack_mcmeta << "  \"pack\": {\n";
        pack_mcmeta << "    \"pack_format\": 4,\n";
        pack_mcmeta << "    \"description\": \"Test shader pack for Minecraft Unifier\"\n";
        pack_mcmeta << "  }\n";
        pack_mcmeta << "}\n";
        pack_mcmeta.close();
        
        // 创建顶点着色器
        std::ofstream vertex_shader(pack_dir + "/shaders/terrain.vsh");
        vertex_shader << "// Vertex shader for terrain\n";
        vertex_shader << "attribute vec4 position;\n";
        vertex_shader << "uniform mat4 projection;\n";
        vertex_shader << "uniform mat4 modelview;\n";
        vertex_shader << "\n";
        vertex_shader << "void main() {\n";
        vertex_shader << "    gl_Position = projection * modelview * position;\n";
        vertex_shader << "}\n";
        vertex_shader.close();
        
        // 创建片段着色器
        std::ofstream fragment_shader(pack_dir + "/shaders/terrain.fsh");
        fragment_shader << "// Fragment shader for terrain\n";
        fragment_shader << "uniform sampler2D texture;\n";
        fragment_shader << "varying vec2 texcoord;\n";
        fragment_shader << "\n";
        fragment_shader << "void main() {\n";
        fragment_shader << "    gl_FragColor = texture2D(texture, texcoord);\n";
        fragment_shader << "}\n";
        fragment_shader.close();
        
        return pack_dir;
    }
    
    std::string test_dir_;
    std::string temp_dir_;
    std::string output_dir_;
};

// 测试Java模组打包
TEST_F(PackerTest, PackJavaMod) {
    // 创建测试Java模组
    std::string jar_path = CreateTestJavaMod("testmod", "Test Mod", "1.0.0");
    
    // 创建Java模组打包器
    JavaPacker packer;
    
    // 打包为.cmc格式
    std::string cmc_path = output_dir_ + "/testmod.cmc";
    bool result = packer.Pack(jar_path, cmc_path, "java");
    
    ASSERT_TRUE(result) << "Failed to pack Java mod";
    ASSERT_TRUE(std::filesystem::exists(cmc_path)) << "CMC file not created";
    
    // 验证CMC文件
    CMCPacker cmc_packer;
    CMCHeader header;
    result = cmc_packer.ReadHeader(cmc_path, header);
    
    ASSERT_TRUE(result) << "Failed to read CMC header";
    EXPECT_EQ(header.magic, CMC_MAGIC) << "Invalid CMC magic number";
    EXPECT_EQ(header.version, CMC_VERSION) << "Invalid CMC version";
    EXPECT_EQ(header.mod_type, ModType::Java) << "Invalid mod type";
    EXPECT_EQ(std::string(header.mod_id), "testmod") << "Invalid mod ID";
    EXPECT_EQ(std::string(header.mod_name), "Test Mod") << "Invalid mod name";
    EXPECT_EQ(std::string(header.version), "1.0.0") << "Invalid version";
}

// 测试网易模组打包
TEST_F(PackerTest, PackNeteaseMod) {
    // 创建测试网易模组
    std::string mod_dir = CreateTestNeteaseMod("neteasemod", "Netease Mod", "2.0.0");
    
    // 创建网易模组打包器
    NeteasePacker packer;
    
    // 打包为.cmc格式
    std::string cmc_path = output_dir_ + "/neteasemod.cmc";
    bool result = packer.Pack(mod_dir, cmc_path, "netease");
    
    ASSERT_TRUE(result) << "Failed to pack Netease mod";
    ASSERT_TRUE(std::filesystem::exists(cmc_path)) << "CMC file not created";
    
    // 验证CMC文件
    CMCPacker cmc_packer;
    CMCHeader header;
    result = cmc_packer.ReadHeader(cmc_path, header);
    
    ASSERT_TRUE(result) << "Failed to read CMC header";
    EXPECT_EQ(header.magic, CMC_MAGIC) << "Invalid CMC magic number";
    EXPECT_EQ(header.version, CMC_VERSION) << "Invalid CMC version";
    EXPECT_EQ(header.mod_type, ModType::Netease) << "Invalid mod type";
    EXPECT_EQ(std::string(header.mod_id), "neteasemod") << "Invalid mod ID";
    EXPECT_EQ(std::string(header.mod_name), "Netease Mod") << "Invalid mod name";
    EXPECT_EQ(std::string(header.version), "2.0.0") << "Invalid version";
}

// 测试光影包打包
TEST_F(PackerTest, PackShaderPack) {
    // 创建测试光影包
    std::string pack_dir = CreateTestShaderPack("testshader");
    
    // 创建光影包打包器
    ShaderPacker packer;
    
    // 打包为.cmc格式
    std::string cmc_path = output_dir_ + "/testshader.cmc";
    bool result = packer.Pack(pack_dir, cmc_path, "shader");
    
    ASSERT_TRUE(result) << "Failed to pack shader pack";
    ASSERT_TRUE(std::filesystem::exists(cmc_path)) << "CMC file not created";
    
    // 验证CMC文件
    CMCPacker cmc_packer;
    CMCHeader header;
    result = cmc_packer.ReadHeader(cmc_path, header);
    
    ASSERT_TRUE(result) << "Failed to read CMC header";
    EXPECT_EQ(header.magic, CMC_MAGIC) << "Invalid CMC magic number";
    EXPECT_EQ(header.version, CMC_VERSION) << "Invalid CMC version";
    EXPECT_EQ(header.mod_type, ModType::Shader) << "Invalid mod type";
    EXPECT_EQ(std::string(header.mod_id), "testshader") << "Invalid mod ID";
    EXPECT_EQ(std::string(header.mod_name), "testshader") << "Invalid mod name";
}

// 测试统一打包器
TEST_F(PackerTest, UnifiedPacker) {
    // 创建测试Java模组
    std::string jar_path = CreateTestJavaMod("unifiedmod", "Unified Mod", "3.0.0");
    
    // 创建统一打包器
    UnifiedPacker packer;
    
    // 打包为.cmc格式
    std::string cmc_path = output_dir_ + "/unifiedmod.cmc";
    bool result = packer.Pack(jar_path, cmc_path, "java");
    
    ASSERT_TRUE(result) << "Failed to pack with unified packer";
    ASSERT_TRUE(std::filesystem::exists(cmc_path)) << "CMC file not created";
    
    // 验证CMC文件
    CMCPacker cmc_packer;
    CMCHeader header;
    result = cmc_packer.ReadHeader(cmc_path, header);
    
    ASSERT_TRUE(result) << "Failed to read CMC header";
    EXPECT_EQ(header.magic, CMC_MAGIC) << "Invalid CMC magic number";
    EXPECT_EQ(header.version, CMC_VERSION) << "Invalid CMC version";
    EXPECT_EQ(header.mod_type, ModType::Java) << "Invalid mod type";
    EXPECT_EQ(std::string(header.mod_id), "unifiedmod") << "Invalid mod ID";
}

// 测试CMC文件解包
TEST_F(PackerTest, UnpackCMC) {
    // 创建测试Java模组
    std::string jar_path = CreateTestJavaMod("unpackmod", "Unpack Mod", "1.0.0");
    
    // 打包为.cmc格式
    JavaPacker packer;
    std::string cmc_path = output_dir_ + "/unpackmod.cmc";
    packer.Pack(jar_path, cmc_path, "java");
    
    // 解包CMC文件
    std::string unpack_dir = output_dir_ + "/unpackmod";
    CMCPacker cmc_packer;
    bool result = cmc_packer.Unpack(cmc_path, unpack_dir);
    
    ASSERT_TRUE(result) << "Failed to unpack CMC file";
    ASSERT_TRUE(std::filesystem::exists(unpack_dir)) << "Unpack directory not created";
    ASSERT_TRUE(std::filesystem::exists(unpack_dir + "/mcmod.info")) << "mcmod.info not found";
    ASSERT_TRUE(std::filesystem::exists(unpack_dir + "/data")) << "data directory not found";
}

// 测试批量打包
TEST_F(PackerTest, BatchPack) {
    // 创建多个测试模组
    std::vector<std::string> mod_paths;
    mod_paths.push_back(CreateTestJavaMod("batchmod1", "Batch Mod 1", "1.0.0"));
    mod_paths.push_back(CreateTestJavaMod("batchmod2", "Batch Mod 2", "1.0.0"));
    mod_paths.push_back(CreateTestJavaMod("batchmod3", "Batch Mod 3", "1.0.0"));
    
    // 批量打包
    UnifiedPacker packer;
    std::string output_dir = output_dir_ + "/batch";
    std::filesystem::create_directories(output_dir);
    
    bool result = packer.BatchPack(mod_paths, output_dir, "java");
    
    ASSERT_TRUE(result) << "Failed to batch pack mods";
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/batchmod1.cmc")) << "batchmod1.cmc not created";
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/batchmod2.cmc")) << "batchmod2.cmc not created";
    ASSERT_TRUE(std::filesystem::exists(output_dir + "/batchmod3.cmc")) << "batchmod3.cmc not created";
}

} // namespace test
} // namespace packer
} // namespace mcu

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
