/**
 * Minecraft Unifier - Shader Converter Implementation
 * 着色器转换器实现 - GLSL到Render Dragon
 */

#include "shader_converter.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

namespace mcu {
namespace core {
namespace render {

// ==================== ShaderConverter ====================

ShaderConverter::ShaderConverter()
    : initialized_(false) {
}

ShaderConverter::~ShaderConverter() {
}

bool ShaderConverter::Initialize() {
    // 初始化Render Dragon API
    if (!RenderDragonAPI::GetInstance().Initialize()) {
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool ShaderConverter::LoadJavaShaderpack(const std::string& shaderpackPath) {
    if (!initialized_) {
        return false;
    }
    
    // 解析光影包
    if (!ParseShaderpack(shaderpackPath)) {
        return false;
    }
    
    return true;
}

bool ShaderConverter::ParseShaderpack(const std::string& shaderpackPath) {
    // 检查光影包目录
    if (!fs::exists(shaderpackPath)) {
        return false;
    }
    
    std::string shadersDir = shaderpackPath + "/shaders";
    if (!fs::exists(shadersDir)) {
        return false;
    }
    
    // 遍历着色器文件
    for (const auto& entry : fs::directory_iterator(shadersDir)) {
        std::string filename = entry.path().filename().string();
        std::string ext = entry.path().extension().string();
        
        // 解析着色器文件名格式：gbuffers_terrain.vsh, gbuffers_terrain.fsh等
        std::regex shaderRegex("^(\\w+)\\.(vsh|fsh|gsh)$");
        std::smatch match;
        
        if (std::regex_match(filename, match, shaderRegex)) {
            std::string materialName = match[1].str();
            std::string stageStr = match[2].str();
            
            // 确定着色器阶段
            ShaderStage stage;
            if (stageStr == "vsh") {
                stage = ShaderStage::VERTEX;
            } else if (stageStr == "fsh") {
                stage = ShaderStage::FRAGMENT;
            } else if (stageStr == "gsh") {
                stage = ShaderStage::GEOMETRY;
            } else {
                continue;
            }
            
            // 读取着色器源代码
            std::string source;
            if (!ReadShaderFile(entry.path().string(), source)) {
                continue;
            }
            
            // 创建或获取材质
            if (materials_.find(materialName) == materials_.end()) {
                MaterialInfo material;
                material.name = materialName;
                material.renderDragonHandle = nullptr;
                materials_[materialName] = material;
            }
            
            // 添加着色器
            ShaderInfo shader;
            shader.stage = stage;
            shader.source = source;
            shader.entryPoint = "main";
            
            // 编译为SPIR-V
            if (!CompileGLSLToSPIRV(shader)) {
                continue;
            }
            
            // 解析Uniform和属性
            ParseUniforms(shader);
            ParseAttributes(shader);
            
            materials_[materialName].shaders.push_back(shader);
        }
    }
    
    // 读取配置文件
    std::string configPath = shaderpackPath + "/shaders.properties";
    if (fs::exists(configPath)) {
        std::ifstream configFile(configPath);
        std::stringstream buffer;
        buffer << configFile.rdbuf();
        std::string content = buffer.str();
        configFile.close();
        
        // 解析配置
        std::regex propRegex("^(\\w+)=(.+)$");
        std::smatch match;
        std::istringstream stream(content);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (std::regex_match(line, match, propRegex)) {
                std::string key = match[1].str();
                std::string value = match[2].str();
                
                // 将配置应用到所有材质
                for (auto& [name, material] : materials_) {
                    material.properties[key] = value;
                }
            }
        }
    }
    
    return true;
}

bool ShaderConverter::ReadShaderFile(const std::string& filePath, std::string& content) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    file.close();
    
    return true;
}

bool ShaderConverter::CompileGLSLToSPIRV(ShaderInfo& shader) {
    // 转换GLSL到Render Dragon兼容格式
    std::string convertedSource = ConvertGLSLToRenderDragon(shader.source, shader.stage);
    
    // 存储转换后的源代码
    shader.source = convertedSource;
    
    // TODO: 使用glslang库编译GLSL到SPIR-V
    // 这里需要集成glslang库进行实际的SPIR-V编译
    // 当前实现：直接存储转换后的源代码
    
    return true;
}

bool ShaderConverter::ParseUniforms(ShaderInfo& shader) {
    // 解析着色器中的Uniform变量
    std::regex uniformRegex("uniform\\s+(\\w+)\\s+(\\w+)(?:\\[(\\d+)\\])?;");
    std::smatch match;
    
    std::istringstream stream(shader.source);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (std::regex_search(line, match, uniformRegex)) {
            std::string type = match[1].str();
            std::string name = match[2].str();
            
            // 分配位置
            int location = shader.uniforms.size();
            shader.uniforms[name] = location;
        }
    }
    
    return true;
}

bool ShaderConverter::ParseAttributes(ShaderInfo& shader) {
    // 解析着色器中的属性变量
    std::regex attrRegex("attribute\\s+(\\w+)\\s+(\\w+);");
    std::smatch match;
    
    std::istringstream stream(shader.source);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (std::regex_search(line, match, attrRegex)) {
            std::string type = match[1].str();
            std::string name = match[2].str();
            
            // 分配位置
            int location = shader.attributes.size();
            shader.attributes[name] = location;
        }
    }
    
    return true;
}

std::string ShaderConverter::ConvertGLSLToRenderDragon(const std::string& glslSource, ShaderStage stage) {
    std::string converted = glslSource;
    
    // 转换版本声明
    converted = std::regex_replace(converted, 
        std::regex("#version\\s+\\d+\\s+(core|compatibility|es)?"),
        "#version 330 core");
    
    // 转换Java版特定的Uniform变量到Render Dragon格式
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferModelView;"),
        "uniform mat4 u_modelViewMatrix;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferProjection;"),
        "uniform mat4 u_projectionMatrix;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferProjectionInverse;"),
        "uniform mat4 u_projectionMatrixInverse;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferModelViewInverse;"),
        "uniform mat4 u_modelViewMatrixInverse;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferPreviousModelView;"),
        "uniform mat4 u_previousModelViewMatrix;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferPreviousProjection;"),
        "uniform mat4 u_previousProjectionMatrix;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+mat4\\s+gbufferTextureMatrix;"),
        "uniform mat4 u_textureMatrix;");
    
    // 转换时间相关Uniform
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+float\\s+frameTimeCounter;"),
        "uniform float u_time;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+float\\s+sunAngle;"),
        "uniform float u_sunAngle;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+float\\s+shadowAngle;"),
        "uniform float u_shadowAngle;");
    
    // 转换视口相关Uniform
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec2\\s+viewWidth;"),
        "uniform vec2 u_viewportSize;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec2\\s+viewHeight;"),
        "uniform vec2 u_viewportSize;");
    
    // 转换相机相关Uniform
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec3\\s+cameraPosition;"),
        "uniform vec3 u_cameraPosition;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec3\\s+previousCameraPosition;"),
        "uniform vec3 u_previousCameraPosition;");
    
    // 转换光照相关Uniform
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec3\\s+sunPosition;"),
        "uniform vec3 u_sunPosition;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec3\\s+moonPosition;"),
        "uniform vec3 u_moonPosition;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+vec3\\s+shadowLightPosition;"),
        "uniform vec3 u_shadowLightPosition;");
    
    // 转换纹理采样器
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+sampler2D\\s+texture;"),
        "uniform sampler2D u_texture;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+sampler2D\\s+normals;"),
        "uniform sampler2D u_normalMap;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+sampler2D\\s+specular;"),
        "uniform sampler2D u_specularMap;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+sampler2D\\s+shadow;"),
        "uniform sampler2D u_shadowMap;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+sampler2D\\s+shadowcolor0;"),
        "uniform sampler2D u_shadowColorMap;");
    
    converted = std::regex_replace(converted,
        std::regex("uniform\\s+sampler2D\\s+shadowcolor1;"),
        "uniform sampler2D u_shadowColorMap1;");
    
    // 转换顶点属性
    if (stage == ShaderStage::VERTEX) {
        converted = std::regex_replace(converted,
            std::regex("attribute\\s+vec3\\s+position;"),
            "in vec3 a_position;");
        
        converted = std::regex_replace(converted,
            std::regex("attribute\\s+vec4\\s+color;"),
            "in vec4 a_color;");
        
        converted = std::regex_replace(converted,
            std::regex("attribute\\s+vec2\\s+texcoord;"),
            "in vec2 a_texCoord;");
        
        converted = std::regex_replace(converted,
            std::regex("attribute\\s+vec2\\s+lmcoord;"),
            "in vec2 a_lightMapCoord;");
        
        converted = std::regex_replace(converted,
            std::regex("attribute\\s+vec3\\s+normal;"),
            "in vec3 a_normal;");
        
        converted = std::regex_replace(converted,
            std::regex("attribute\\s+vec4\\s+tangent;"),
            "in vec4 a_tangent;");
        
        // 转换varying到out
        converted = std::regex_replace(converted,
            std::regex("varying\\s+(\\w+)\\s+(\\w+);"),
            "out $1 $2;");
    } else if (stage == ShaderStage::FRAGMENT) {
        // 转换varying到in
        converted = std::regex_replace(converted,
            std::regex("varying\\s+(\\w+)\\s+(\\w+);"),
            "in $1 $2;");
        
        // 转换gl_FragColor到输出变量
        converted = std::regex_replace(converted,
            std::regex("gl_FragColor"),
            "fragColor");
        
        // 如果没有定义fragColor，添加它
        if (converted.find("out vec4 fragColor;") == std::string::npos) {
            converted = "out vec4 fragColor;\n" + converted;
        }
    }
    
    // 转换内置函数
    converted = std::regex_replace(converted,
        std::regex("ftransform\\(\\)"),
        "(gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex)");
    
    // 转换纹理函数
    converted = std::regex_replace(converted,
        std::regex("texture2D\\s*\\("),
        "texture(");
    
    converted = std::regex_replace(converted,
        std::regex("texture2DLod\\s*\\("),
        "textureLod(");
    
    converted = std::regex_replace(converted,
        std::regex("shadow2D\\s*\\("),
        "texture(");
    
    converted = std::regex_replace(converted,
        std::regex("shadow2DLod\\s*\\("),
        "textureLod(");
    
    return converted;
}

bool ShaderConverter::CompileToRenderDragon(const std::string& materialName) {
    auto it = materials_.find(materialName);
    if (it == materials_.end()) {
        return false;
    }
    
    MaterialInfo& material = it->second;
    
    // 创建Render Dragon材质
    material.renderDragonHandle = CreateRenderDragonMaterial(material);
    if (!material.renderDragonHandle) {
        return false;
    }
    
    // 设置着色器阶段
    for (const auto& shader : material.shaders) {
        if (!SetRenderDragonShaderStage(material.renderDragonHandle, shader)) {
            return false;
        }
    }
    
    // 添加Uniform
    for (const auto& shader : material.shaders) {
        for (const auto& [name, location] : shader.uniforms) {
            AddRenderDragonUniform(material.renderDragonHandle, name, location);
        }
    }
    
    return true;
}

void* ShaderConverter::CreateRenderDragonMaterial(const MaterialInfo& material) {
    return RenderDragonAPI::GetInstance().CreateMaterial(material.name.c_str());
}

bool ShaderConverter::SetRenderDragonShaderStage(void* material, const ShaderInfo& shader) {
    int stage = static_cast<int>(shader.stage);
    return RenderDragonAPI::GetInstance().SetShaderStage(
        material, stage, shader.spirv.data(), shader.spirv.size());
}

bool ShaderConverter::AddRenderDragonUniform(void* material, const std::string& name, int location) {
    return RenderDragonAPI::GetInstance().AddUniform(material, name.c_str(), location);
}

void ShaderConverter::UpdateUniforms(void* material, const std::unordered_map<std::string, float>& values) {
    for (const auto& [name, value] : values) {
        RenderDragonAPI::GetInstance().SetUniformValue(material, name.c_str(), &value, 1);
    }
}

void ShaderConverter::BindMaterial(void* material) {
    RenderDragonAPI::GetInstance().BindMaterial(material);
}

std::vector<std::string> ShaderConverter::GetMaterialList() const {
    std::vector<std::string> list;
    for (const auto& [name, material] : materials_) {
        list.push_back(name);
    }
    return list;
}

const MaterialInfo* ShaderConverter::GetMaterialInfo(const std::string& name) const {
    auto it = materials_.find(name);
    if (it != materials_.end()) {
        return &it->second;
    }
    return nullptr;
}

// ==================== RenderDragonAPI ====================

RenderDragonAPI::RenderDragonAPI()
    : initialized_(false) {
}

RenderDragonAPI::~RenderDragonAPI() {
}

RenderDragonAPI& RenderDragonAPI::GetInstance() {
    static RenderDragonAPI instance;
    return instance;
}

bool RenderDragonAPI::Initialize() {
#ifdef _WIN32
    return InitializeWindows();
#elif defined(__linux__)
    return InitializeLinux();
#elif defined(__ANDROID__)
    return InitializeAndroid();
#else
    return false;
#endif
}

#ifdef _WIN32
bool RenderDragonAPI::InitializeWindows() {
    // Windows平台：从minecraftpe.dll获取函数指针
    HMODULE hMod = GetModuleHandleA("minecraftpe.dll");
    if (!hMod) {
        return false;
    }
    
    createMaterialFunc_ = reinterpret_cast<decltype(createMaterialFunc_)>(
        GetProcAddress(hMod, "RenderDragon_CreateMaterial"));
    setShaderStageFunc_ = reinterpret_cast<decltype(setShaderStageFunc_)>(
        GetProcAddress(hMod, "RenderDragon_SetShaderStage"));
    addUniformFunc_ = reinterpret_cast<decltype(addUniformFunc_)>(
        GetProcAddress(hMod, "RenderDragon_AddUniform"));
    setUniformValueFunc_ = reinterpret_cast<decltype(setUniformValueFunc_)>(
        GetProcAddress(hMod, "RenderDragon_SetUniformValue"));
    bindMaterialFunc_ = reinterpret_cast<decltype(bindMaterialFunc_)>(
        GetProcAddress(hMod, "RenderDragon_BindMaterial"));
    destroyMaterialFunc_ = reinterpret_cast<decltype(destroyMaterialFunc_)>(
        GetProcAddress(hMod, "RenderDragon_DestroyMaterial"));
    
    initialized_ = true;
    return true;
}
#endif

#ifdef __linux__
bool RenderDragonAPI::InitializeLinux() {
    // Linux平台：从libminecraftpe.so获取函数指针
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) {
        return false;
    }
    
    createMaterialFunc_ = reinterpret_cast<decltype(createMaterialFunc_)>(
        dlsym(handle, "RenderDragon_CreateMaterial"));
    setShaderStageFunc_ = reinterpret_cast<decltype(setShaderStageFunc_)>(
        dlsym(handle, "RenderDragon_SetShaderStage"));
    addUniformFunc_ = reinterpret_cast<decltype(addUniformFunc_)>(
        dlsym(handle, "RenderDragon_AddUniform"));
    setUniformValueFunc_ = reinterpret_cast<decltype(setUniformValueFunc_)>(
        dlsym(handle, "RenderDragon_SetUniformValue"));
    bindMaterialFunc_ = reinterpret_cast<decltype(bindMaterialFunc_)>(
        dlsym(handle, "RenderDragon_BindMaterial"));
    destroyMaterialFunc_ = reinterpret_cast<decltype(destroyMaterialFunc_)>(
        dlsym(handle, "RenderDragon_DestroyMaterial"));
    
    initialized_ = true;
    return true;
}
#endif

#ifdef __ANDROID__
bool RenderDragonAPI::InitializeAndroid() {
    // Android平台：从libminecraftpe.so获取函数指针
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) {
        return false;
    }
    
    createMaterialFunc_ = reinterpret_cast<decltype(createMaterialFunc_)>(
        dlsym(handle, "RenderDragon_CreateMaterial"));
    setShaderStageFunc_ = reinterpret_cast<decltype(setShaderStageFunc_)>(
        dlsym(handle, "RenderDragon_SetShaderStage"));
    addUniformFunc_ = reinterpret_cast<decltype(addUniformFunc_)>(
        dlsym(handle, "RenderDragon_AddUniform"));
    setUniformValueFunc_ = reinterpret_cast<decltype(setUniformValueFunc_)>(
        dlsym(handle, "RenderDragon_SetUniformValue"));
    bindMaterialFunc_ = reinterpret_cast<decltype(bindMaterialFunc_)>(
        dlsym(handle, "RenderDragon_BindMaterial"));
    destroyMaterialFunc_ = reinterpret_cast<decltype(destroyMaterialFunc_)>(
        dlsym(handle, "RenderDragon_DestroyMaterial"));
    
    initialized_ = true;
    return true;
}
#endif

void* RenderDragonAPI::CreateMaterial(const char* name) {
    if (!initialized_ || !createMaterialFunc_) {
        return nullptr;
    }
    return createMaterialFunc_(name);
}

bool RenderDragonAPI::SetShaderStage(void* material, int stage, const uint32_t* spirv, size_t size) {
    if (!initialized_ || !setShaderStageFunc_) {
        return false;
    }
    return setShaderStageFunc_(material, stage, spirv, size);
}

bool RenderDragonAPI::AddUniform(void* material, const char* name, int location) {
    if (!initialized_ || !addUniformFunc_) {
        return false;
    }
    return addUniformFunc_(material, name, location);
}

bool RenderDragonAPI::SetUniformValue(void* material, const char* name, const float* value, int count) {
    if (!initialized_ || !setUniformValueFunc_) {
        return false;
    }
    return setUniformValueFunc_(material, name, value, count);
}

bool RenderDragonAPI::BindMaterial(void* material) {
    if (!initialized_ || !bindMaterialFunc_) {
        return false;
    }
    return bindMaterialFunc_(material);
}

bool RenderDragonAPI::DestroyMaterial(void* material) {
    if (!initialized_ || !destroyMaterialFunc_) {
        return false;
    }
    return destroyMaterialFunc_(material);
}

// ==================== ShaderCache ====================

ShaderCache::ShaderCache() {
    cachePath_ = fs::temp_directory_path().string() + "/shader_cache";
    fs::create_directories(cachePath_);
}

ShaderCache::~ShaderCache() {
    SaveToDisk(cachePath_ + "/cache.bin");
}

bool ShaderCache::HasCached(const std::string& key) {
    return cache_.find(key) != cache_.end();
}

std::vector<uint32_t> ShaderCache::GetCached(const std::string& key) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second;
    }
    return {};
}

void ShaderCache::SetCached(const std::string& key, const std::vector<uint32_t>& spirv) {
    cache_[key] = spirv;
}

void ShaderCache::Clear() {
    cache_.clear();
}

bool ShaderCache::SaveToDisk(const std::string& cachePath) {
    std::ofstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // 写入缓存数量
    uint32_t count = cache_.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    
    // 写入每个缓存项
    for (const auto& [key, spirv] : cache_) {
        // 写入key长度
        uint32_t keyLen = key.size();
        file.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
        
        // 写入key
        file.write(key.c_str(), keyLen);
        
        // 写入SPIR-V大小
        uint32_t spirvSize = spirv.size();
        file.write(reinterpret_cast<const char*>(&spirvSize), sizeof(spirvSize));
        
        // 写入SPIR-V数据
        file.write(reinterpret_cast<const char*>(spirv.data()), spirvSize * sizeof(uint32_t));
    }
    
    file.close();
    return true;
}

bool ShaderCache::LoadFromDisk(const std::string& cachePath) {
    std::ifstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // 读取缓存数量
    uint32_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));
    
    // 读取每个缓存项
    for (uint32_t i = 0; i < count; i++) {
        // 读取key长度
        uint32_t keyLen;
        file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
        
        // 读取key
        std::string key(keyLen, '\0');
        file.read(&key[0], keyLen);
        
        // 读取SPIR-V大小
        uint32_t spirvSize;
        file.read(reinterpret_cast<char*>(&spirvSize), sizeof(spirvSize));
        
        // 读取SPIR-V数据
        std::vector<uint32_t> spirv(spirvSize);
        file.read(reinterpret_cast<char*>(spirv.data()), spirvSize * sizeof(uint32_t));
        
        cache_[key] = spirv;
    }
    
    file.close();
    return true;
}

} // namespace render
} // namespace core
} // namespace mcu
