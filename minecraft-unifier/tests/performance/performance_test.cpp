/**
 * Minecraft Unifier - Performance Tests
 * 性能测试用例
 */

#include <gtest/gtest.h>
#include <packer/windows/java_packer.h>
#include <packer/windows/netease_packer.h>
#include <packer/windows/shader_packer.h>
#include <packer/windows/unified_packer.h>
#include <core/mods/java_runtime.h>
#include <core/mods/netease_runtime.h>
#include <core/resources/resource_manager.h>
#include <core/render/shader_converter.h>
#include <common/cmc_format.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

namespace mcu {
namespace performance {
namespace test {

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录
        test_dir_ = "/tmp/minecraft-unifier-performance-test";
        std::filesystem::create_directories(test_dir_);
        
        // 创建临时目录
        temp_dir_ = test_dir_ + "/temp";
        std::filesystem::create_directories(temp_dir_);
        
        // 创建输出目录
        output_dir_ = test_dir_ + "/output";
        std::filesystem::create_directories(output_dir_);
        
        // 创建模组目录
        mods_dir_ = test_dir_ + "/mods";
        std::filesystem::create_directories(mods_dir_);
        
        // 创建资源包目录
        resource_packs_dir_ = test_dir_ + "/resource_packs";
        std::filesystem::create_directories(resource_packs_dir_);
    }
    
    void TearDown() override {
        // 清理测试目录
        std::filesystem::remove_all(test_dir_);
    }
    
    // 创建测试用的Java模组
    std::string CreateTestJavaMod(const std::string& mod_id, const std::string& mod_name, const std::string& version) {
        std::string mod_dir = temp_dir_ + "/" + mod_id;
        std::filesystem::create_directories(mod_dir);
        
        // 创建mcmod.info
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
        
        return mod_dir;
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
        vertex_shader << "attribute vec2 texcoord;\n";
        vertex_shader << "uniform mat4 projection;\n";
        vertex_shader << "uniform mat4 modelview;\n";
        vertex_shader << "uniform sampler2D texture;\n";
        vertex_shader << "varying vec2 v_texcoord;\n";
        vertex_shader << "\n";
        vertex_shader << "void main() {\n";
        vertex_shader << "    gl_Position = projection * modelview * position;\n";
        vertex_shader << "    v_texcoord = texcoord;\n";
        vertex_shader << "}\n";
        vertex_shader.close();
        
        // 创建片段着色器
        std::ofstream fragment_shader(pack_dir + "/shaders/terrain.fsh");
        fragment_shader << "// Fragment shader for terrain\n";
        fragment_shader << "uniform sampler2D texture;\n";
        fragment_shader << "varying vec2 v_texcoord;\n";
        fragment_shader << "\n";
        fragment_shader << "void main() {\n";
        fragment_shader << "    gl_FragColor = texture2D(texture, v_texcoord);\n";
        fragment_shader << "}\n";
        fragment_shader.close();
        
        return pack_dir;
    }
    
    // 创建大文件用于测试
    std::string CreateLargeFile(const std::string& file_path, size_t size) {
        std::ofstream file(file_path, std::ios::binary);
        
        // 写入随机数据
        const size_t buffer_size = 1024 * 1024; // 1MB缓冲区
        std::vector<char> buffer(buffer_size, 'A');
        
        size_t remaining = size;
        while (remaining > 0) {
            size_t write_size = std::min(buffer_size, remaining);
            file.write(buffer.data(), write_size);
            remaining -= write_size;
        }
        
        file.close();
        return file_path;
    }
    
    std::string test_dir_;
    std::string temp_dir_;
    std::string output_dir_;
    std::string mods_dir_;
    std::string resource_packs_dir_;
};

// 性能测试1：单个模组打包性能
TEST_F(PerformanceTest, SingleModPackingPerformance) {
    // 创建测试Java模组
    std::string mod_dir = CreateTestJavaMod("perfmod", "Performance Mod", "1.0.0");
    
    // 测试打包性能
    JavaPacker packer;
    std::string cmc_path = output_dir_ + "/perfmod.cmc";
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = packer.Pack(mod_dir, cmc_path, "java");
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to pack mod";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Single mod packing time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：单个模组打包应该在1秒内完成
    EXPECT_LT(duration.count(), 1000) << "Single mod packing took too long";
}

// 性能测试2：批量打包性能
TEST_F(PerformanceTest, BatchPackingPerformance) {
    // 创建多个测试模组
    const int mod_count = 10;
    std::vector<std::string> mod_paths;
    
    for (int i = 0; i < mod_count; i++) {
        std::string mod_id = "batchmod" + std::to_string(i);
        mod_paths.push_back(CreateTestJavaMod(mod_id, "Batch Mod " + std::to_string(i), "1.0.0"));
    }
    
    // 测试批量打包性能
    UnifiedPacker packer;
    std::string batch_output_dir = output_dir_ + "/batch";
    std::filesystem::create_directories(batch_output_dir);
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = packer.BatchPack(mod_paths, batch_output_dir, "java");
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to batch pack mods";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Batch packing time (" << mod_count << " mods): " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per mod: " << duration.count() / mod_count << " ms" << std::endl;
    
    // 性能要求：批量打包10个模组应该在5秒内完成
    EXPECT_LT(duration.count(), 5000) << "Batch packing took too long";
}

// 性能测试3：CMC文件读取性能
TEST_F(PerformanceTest, CMCFileReadPerformance) {
    // 创建测试Java模组
    std::string mod_dir = CreateTestJavaMod("readperfmod", "Read Performance Mod", "1.0.0");
    
    // 打包为.cmc格式
    JavaPacker packer;
    std::string cmc_path = output_dir_ + "/readperfmod.cmc";
    packer.Pack(mod_dir, cmc_path, "java");
    
    // 测试CMC文件读取性能
    CMCPacker cmc_packer;
    
    auto start = std::chrono::high_resolution_clock::now();
    CMCHeader header;
    bool result = cmc_packer.ReadHeader(cmc_path, header);
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to read CMC header";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "CMC file read time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：CMC文件读取应该在100ms内完成
    EXPECT_LT(duration.count(), 100) << "CMC file read took too long";
}

// 性能测试4：CMC文件解包性能
TEST_F(PerformanceTest, CMCFileUnpackPerformance) {
    // 创建测试Java模组
    std::string mod_dir = CreateTestJavaMod("unpackperfmod", "Unpack Performance Mod", "1.0.0");
    
    // 打包为.cmc格式
    JavaPacker packer;
    std::string cmc_path = output_dir_ + "/unpackperfmod.cmc";
    packer.Pack(mod_dir, cmc_path, "java");
    
    // 测试CMC文件解包性能
    CMCPacker cmc_packer;
    std::string unpack_dir = output_dir_ + "/unpackperfmod_unpacked";
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = cmc_packer.Unpack(cmc_path, unpack_dir);
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to unpack CMC file";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "CMC file unpack time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：CMC文件解包应该在500ms内完成
    EXPECT_LT(duration.count(), 500) << "CMC file unpack took too long";
}

// 性能测试5：着色器转换性能
TEST_F(PerformanceTest, ShaderConversionPerformance) {
    // 创建测试光影包
    std::string pack_dir = CreateTestShaderPack("shaderperf");
    
    // 测试着色器转换性能
    core::render::ShaderConverter converter;
    std::string vertex_shader = pack_dir + "/shaders/terrain.vsh";
    std::string rd_vertex_shader = output_dir_ + "/terrain_rd.vsh";
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = converter.ConvertGLSLToRenderDragon(vertex_shader, rd_vertex_shader);
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to convert shader";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Shader conversion time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：着色器转换应该在100ms内完成
    EXPECT_LT(duration.count(), 100) << "Shader conversion took too long";
}

// 性能测试6：批量着色器转换性能
TEST_F(PerformanceTest, BatchShaderConversionPerformance) {
    // 创建多个测试光影包
    const int shader_count = 20;
    std::vector<std::string> shader_paths;
    
    for (int i = 0; i < shader_count; i++) {
        std::string pack_name = "shaderperf" + std::to_string(i);
        std::string pack_dir = CreateTestShaderPack(pack_name);
        shader_paths.push_back(pack_dir + "/shaders/terrain.vsh");
    }
    
    // 测试批量着色器转换性能
    core::render::ShaderConverter converter;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < shader_count; i++) {
        std::string output_path = output_dir_ + "/terrain_rd" + std::to_string(i) + ".vsh";
        converter.ConvertGLSLToRenderDragon(shader_paths[i], output_path);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Batch shader conversion time (" << shader_count << " shaders): " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per shader: " << duration.count() / shader_count << " ms" << std::endl;
    
    // 性能要求：批量转换20个着色器应该在2秒内完成
    EXPECT_LT(duration.count(), 2000) << "Batch shader conversion took too long";
}

// 性能测试7：资源管理器初始化性能
TEST_F(PerformanceTest, ResourceManagerInitializationPerformance) {
    // 创建多个测试资源包
    const int pack_count = 10;
    for (int i = 0; i < pack_count; i++) {
        std::string pack_name = "resourcepack" + std::to_string(i);
        CreateTestShaderPack(pack_name);
    }
    
    // 测试资源管理器初始化性能
    ResourceManager& manager = ResourceManager::GetInstance();
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = manager.Initialize(mods_dir_, resource_packs_dir_);
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to initialize resource manager";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Resource manager initialization time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：资源管理器初始化应该在1秒内完成
    EXPECT_LT(duration.count(), 1000) << "Resource manager initialization took too long";
    
    // 清理
    manager.Shutdown();
}

// 性能测试8：资源包加载性能
TEST_F(PerformanceTest, ResourcePackLoadingPerformance) {
    // 创建测试资源包
    std::string pack_dir = CreateTestShaderPack("loadperfpack");
    
    // 测试资源包加载性能
    ResourceManager& manager = ResourceManager::GetInstance();
    manager.Initialize(mods_dir_, resource_packs_dir_);
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = manager.LoadResourcePack(pack_dir);
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to load resource pack";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Resource pack loading time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：资源包加载应该在500ms内完成
    EXPECT_LT(duration.count(), 500) << "Resource pack loading took too long";
    
    // 清理
    manager.Shutdown();
}

// 性能测试9：批量资源包加载性能
TEST_F(PerformanceTest, BatchResourcePackLoadingPerformance) {
    // 创建多个测试资源包
    const int pack_count = 10;
    std::vector<std::string> pack_dirs;
    
    for (int i = 0; i < pack_count; i++) {
        std::string pack_name = "loadperfpack" + std::to_string(i);
        pack_dirs.push_back(CreateTestShaderPack(pack_name));
    }
    
    // 测试批量资源包加载性能
    ResourceManager& manager = ResourceManager::GetInstance();
    manager.Initialize(mods_dir_, resource_packs_dir_);
    
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& pack_dir : pack_dirs) {
        manager.LoadResourcePack(pack_dir);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Batch resource pack loading time (" << pack_count << " packs): " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per pack: " << duration.count() / pack_count << " ms" << std::endl;
    
    // 性能要求：批量加载10个资源包应该在3秒内完成
    EXPECT_LT(duration.count(), 3000) << "Batch resource pack loading took too long";
    
    // 清理
    manager.Shutdown();
}

// 性能测试10：重定向规则查询性能
TEST_F(PerformanceTest, RedirectRuleQueryPerformance) {
    // 初始化资源管理器
    ResourceManager& manager = ResourceManager::GetInstance();
    manager.Initialize(mods_dir_, resource_packs_dir_);
    
    // 添加大量重定向规则
    const int rule_count = 1000;
    for (int i = 0; i < rule_count; i++) {
        std::string original = "assets/minecraft/textures/blocks/block" + std::to_string(i) + ".png";
        std::string redirected = "mods/testmod/textures/block" + std::to_string(i) + ".png";
        manager.AddRedirectRule(original, redirected);
    }
    
    // 测试重定向规则查询性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < rule_count; i++) {
        std::string original = "assets/minecraft/textures/blocks/block" + std::to_string(i) + ".png";
        std::string redirected = manager.GetRedirectedPath(original);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Redirect rule query time (" << rule_count << " queries): " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per query: " << duration.count() / rule_count << " ms" << std::endl;
    
    // 性能要求：1000次查询应该在100ms内完成
    EXPECT_LT(duration.count(), 100) << "Redirect rule query took too long";
    
    // 清理
    manager.Shutdown();
}

// 性能测试11：大文件处理性能
TEST_F(PerformanceTest, LargeFileProcessingPerformance) {
    // 创建大文件（10MB）
    std::string large_file = output_dir_ + "/large_file.dat";
    CreateLargeFile(large_file, 10 * 1024 * 1024);
    
    // 测试大文件读取性能
    auto start = std::chrono::high_resolution_clock::now();
    std::ifstream file(large_file, std::ios::binary);
    std::vector<char> buffer(10 * 1024 * 1024);
    file.read(buffer.data(), buffer.size());
    file.close();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Large file (10MB) read time: " << duration.count() << " ms" << std::endl;
    
    // 性能要求：读取10MB文件应该在100ms内完成
    EXPECT_LT(duration.count(), 100) << "Large file read took too long";
}

// 性能测试12：内存使用测试
TEST_F(PerformanceTest, MemoryUsageTest) {
    // 创建多个测试模组
    const int mod_count = 50;
    std::vector<std::string> mod_paths;
    
    for (int i = 0; i < mod_count; i++) {
        std::string mod_id = "memmod" + std::to_string(i);
        mod_paths.push_back(CreateTestJavaMod(mod_id, "Memory Mod " + std::to_string(i), "1.0.0"));
    }
    
    // 批量打包
    UnifiedPacker packer;
    std::string batch_output_dir = output_dir_ + "/memory_batch";
    std::filesystem::create_directories(batch_output_dir);
    
    packer.BatchPack(mod_paths, batch_output_dir, "java");
    
    // 注意：这里只是测试代码路径，实际内存使用需要使用专门的内存分析工具
    std::cout << "Memory usage test completed with " << mod_count << " mods" << std::endl;
}

// 性能测试13：并发性能测试
TEST_F(PerformanceTest, ConcurrencyPerformanceTest) {
    // 创建多个测试模组
    const int mod_count = 20;
    std::vector<std::string> mod_paths;
    
    for (int i = 0; i < mod_count; i++) {
        std::string mod_id = "concurrentmod" + std::to_string(i);
        mod_paths.push_back(CreateTestJavaMod(mod_id, "Concurrent Mod " + std::to_string(i), "1.0.0"));
    }
    
    // 测试并发打包性能
    UnifiedPacker packer;
    std::string batch_output_dir = output_dir_ + "/concurrent_batch";
    std::filesystem::create_directories(batch_output_dir);
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = packer.BatchPack(mod_paths, batch_output_dir, "java");
    auto end = std::chrono::high_resolution_clock::now();
    
    ASSERT_TRUE(result) << "Failed to batch pack mods";
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Concurrent packing time (" << mod_count << " mods): " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per mod: " << duration.count() / mod_count << " ms" << std::endl;
    
    // 性能要求：并发打包20个模组应该在3秒内完成
    EXPECT_LT(duration.count(), 3000) << "Concurrent packing took too long";
}

// 性能测试14：缓存性能测试
TEST_F(PerformanceTest, CachePerformanceTest) {
    // 创建测试光影包
    std::string pack_dir = CreateTestShaderPack("cacheshader");
    std::string vertex_shader = pack_dir + "/shaders/terrain.vsh";
    
    // 测试着色器转换器缓存性能
    core::render::ShaderConverter converter;
    std::string rd_vertex_shader = output_dir_ + "/terrain_cache_rd.vsh";
    
    // 第一次转换（无缓存）
    auto start1 = std::chrono::high_resolution_clock::now();
    converter.ConvertGLSLToRenderDragon(vertex_shader, rd_vertex_shader);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
    
    // 第二次转换（使用缓存）
    auto start2 = std::chrono::high_resolution_clock::now();
    converter.ConvertGLSLToRenderDragon(vertex_shader, rd_vertex_shader);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
    
    std::cout << "First conversion time (no cache): " << duration1.count() << " ms" << std::endl;
    std::cout << "Second conversion time (with cache): " << duration2.count() << " ms" << std::endl;
    std::cout << "Cache speedup: " << (double)duration1.count() / duration2.count() << "x" << std::endl;
    
    // 性能要求：缓存应该至少快2倍
    EXPECT_GT(duration1.count(), duration2.count() * 2) << "Cache not effective enough";
}

// 性能测试15：长时间运行稳定性测试
TEST_F(PerformanceTest, LongRunningStabilityTest) {
    // 创建测试模组
    std::string mod_dir = CreateTestJavaMod("stabilitymod", "Stability Mod", "1.0.0");
    
    // 测试长时间运行稳定性
    JavaPacker packer;
    const int iteration_count = 100;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iteration_count; i++) {
        std::string cmc_path = output_dir_ + "/stabilitymod" + std::to_string(i) + ".cmc";
        packer.Pack(mod_dir, cmc_path, "java");
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Long running test time (" << iteration_count << " iterations): " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per iteration: " << duration.count() / iteration_count << " ms" << std::endl;
    
    // 性能要求：100次迭代应该在30秒内完成
    EXPECT_LT(duration.count(), 30000) << "Long running test took too long";
}

} // namespace test
} // namespace performance
} // namespace mcu

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
