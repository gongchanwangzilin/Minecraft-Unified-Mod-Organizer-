/**
 * Minecraft Unifier - Shader Converter
 * 着色器转换器 - GLSL到Render Dragon
 */

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace mcu {
namespace core {
namespace render {

// 着色器阶段
enum class ShaderStage {
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    COMPUTE
};

// 着色器信息
struct ShaderInfo {
    ShaderStage stage;
    std::string source;
    std::string entryPoint;
    std::vector<uint32_t> spirv; // SPIR-V字节码
    std::unordered_map<std::string, int> uniforms;
    std::unordered_map<std::string, int> attributes;
};

// 材质信息
struct MaterialInfo {
    std::string name;
    std::vector<ShaderInfo> shaders;
    std::unordered_map<std::string, std::string> properties;
    void* renderDragonHandle; // 平台相关句柄
};

// 着色器转换器
class ShaderConverter {
public:
    ShaderConverter();
    ~ShaderConverter();

    // 初始化
    bool Initialize();
    
    // 从Java版光影包加载
    bool LoadJavaShaderpack(const std::string& shaderpackPath);
    
    // 编译为Render Dragon材质
    bool CompileToRenderDragon(const std::string& materialName);
    
    // 更新Uniform值
    void UpdateUniforms(void* material, const std::unordered_map<std::string, float>& values);
    
    // 绑定材质
    void BindMaterial(void* material);
    
    // 获取材质列表
    std::vector<std::string> GetMaterialList() const;
    
    // 获取材质信息
    const MaterialInfo* GetMaterialInfo(const std::string& name) const;

private:
    std::unordered_map<std::string, MaterialInfo> materials_;
    bool initialized_;
    
    // 内部处理函数
    bool ParseShaderpack(const std::string& shaderpackPath);
    bool ReadShaderFile(const std::string& filePath, std::string& content);
    bool CompileGLSLToSPIRV(ShaderInfo& shader);
    bool ParseUniforms(ShaderInfo& shader);
    bool ParseAttributes(ShaderInfo& shader);
    void* CreateRenderDragonMaterial(const MaterialInfo& material);
    bool SetRenderDragonShaderStage(void* material, const ShaderInfo& shader);
    bool AddRenderDragonUniform(void* material, const std::string& name, int location);
};

// Render Dragon API封装
class RenderDragonAPI {
public:
    static RenderDragonAPI& GetInstance();
    
    // 初始化（需要逆向分析获取函数地址）
    bool Initialize();
    
    // 创建材质
    void* CreateMaterial(const char* name);
    
    // 设置着色器阶段
    bool SetShaderStage(void* material, int stage, const uint32_t* spirv, size_t size);
    
    // 添加Uniform
    bool AddUniform(void* material, const char* name, int location);
    
    // 设置Uniform值
    bool SetUniformValue(void* material, const char* name, const float* value, int count);
    
    // 绑定材质
    bool BindMaterial(void* material);
    
    // 销毁材质
    bool DestroyMaterial(void* material);

private:
    RenderDragonAPI();
    ~RenderDragonAPI();
    
    bool initialized_;
    
    // 函数指针（需要通过逆向分析填充）
    void* (*createMaterialFunc_)(const char*) = nullptr;
    bool (*setShaderStageFunc_)(void*, int, const uint32_t*, size_t) = nullptr;
    bool (*addUniformFunc_)(void*, const char*, int) = nullptr;
    bool (*setUniformValueFunc_)(void*, const char*, const float*, int) = nullptr;
    bool (*bindMaterialFunc_)(void*) = nullptr;
    bool (*destroyMaterialFunc_)(void*) = nullptr;
    
    // 平台相关初始化
    bool InitializeWindows();
    bool InitializeLinux();
    bool InitializeAndroid();
};

// 着色器缓存
class ShaderCache {
public:
    ShaderCache();
    ~ShaderCache();
    
    // 检查缓存
    bool HasCached(const std::string& key);
    
    // 获取缓存
    std::vector<uint32_t> GetCached(const std::string& key);
    
    // 保存缓存
    void SetCached(const std::string& key, const std::vector<uint32_t>& spirv);
    
    // 清除缓存
    void Clear();
    
    // 保存到磁盘
    bool SaveToDisk(const std::string& cachePath);
    
    // 从磁盘加载
    bool LoadFromDisk(const std::string& cachePath);

private:
    std::unordered_map<std::string, std::vector<uint32_t>> cache_;
    std::string cachePath_;
};

} // namespace render
} // namespace core
} // namespace mcu
