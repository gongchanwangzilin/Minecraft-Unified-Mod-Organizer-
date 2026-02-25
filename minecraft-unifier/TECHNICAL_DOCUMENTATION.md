# 我的世界统一器 - 技术文档

## 目录

- [项目概述](#项目概述)
- [技术架构](#技术架构)
- [核心模块详解](#核心模块详解)
- [API参考](#api参考)
- [开发指南](#开发指南)
- [部署指南](#部署指南)
- [贡献指南](#贡献指南)
- [常见问题](#常见问题)

---

## 项目概述

### 项目简介

"我的世界统一器"（Minecraft Unifier）是一个跨平台的Minecraft模组兼容工具，旨在打破Java版、基岩版（国际版）、网易中国版之间的资源壁垒，实现基岩版对Java版光影包、各类模组、资源包的完整兼容。

### 核心特性

- **静态注入技术**：所有修改在安装/构建阶段完成，游戏运行时仅加载必要的兼容模块
- **三端统一代码基**：Windows、Linux、Android共享核心兼容代码（约80%）
- **跨平台兼容**：支持Windows、Linux、Android三大平台
- **多种注入方式**：
  - Windows：PE文件导入表劫持
  - Linux：ELF文件PT_NOTE注入
  - Android：APK重打包 + so库预加载

### 技术栈

- **核心语言**：C++17
- **构建系统**：CMake 3.16+
- **依赖库**：
  - zlib（压缩）
  - Python 3（网易模组运行时）
  - Detours（Windows Hook）
  - xHook（Android Hook）
  - glslang（着色器编译）

### 项目结构

```
minecraft-unifier/
├── common/              # 通用代码库
│   ├── cmc_format.h    # .cmc格式定义
│   └── cmc_format.cpp  # .cmc格式实现
├── injector/           # 注入器
│   ├── android/        # Android平台注入器
│   ├── windows/        # Windows平台注入器
│   └── linux/          # Linux平台注入器
├── packer/             # 打包器
│   └── windows/        # Windows平台打包器
├── core/               # 统一器核心
│   ├── render/         # 渲染兼容模块
│   ├── mods/           # 模组兼容模块
│   └── resources/      # 资源管理模块
├── gui/                # GUI组件
│   ├── desktop/        # 桌面端GUI
│   ├── ingame/         # 游戏内GUI
│   └── android/        # Android GUI
└── CMakeLists.txt      # 构建配置
```

---

## 技术架构

### 总体架构设计

项目采用分层架构设计，从底层到上层分为：

1. **注入层**：负责将加载器植入原版游戏二进制文件
2. **加载器层**：动态加载兼容模块、事件分发
3. **兼容层**：渲染引擎、模组运行时、资源管理器
4. **工具链层**：Qt6跨平台GUI、.cmc打包器、逆向辅助工具
5. **GUI层**：ImGui运行时面板 + Qt6管理工具

### 数据流

```
用户操作 → 管理工具(GUI) → 修改目标游戏文件 → 静态注入完成
        ↓
启动游戏 → 加载器初始化 → 解析.cmc包 → 加载兼容模块
        ↓
渲染循环 → 光影模块拦截 → GLSL编译 → Render Dragon执行
        ↓
游戏逻辑 → 模组模块拦截 → JVM/Python执行 → API映射 → 返回结果
        ↓
资源加载 → 文件Hook → 重定向/转换 → 返回游戏
```

### 平台特定实现

#### Windows平台

**注入方式**：PE文件导入表劫持

```cpp
// 通过修改PE文件的导入表，在游戏加载系统DLL之前
// 强制加载我们的MinecraftUnifier.dll
bool InjectToPE(const std::wstring& targetPath, const std::wstring& outputPath) {
    // 1. 打开PE文件
    HANDLE hFile = CreateFileW(targetPath.c_str(), GENERIC_READ | GENERIC_WRITE, ...);
    
    // 2. 定位导入表
    PIMAGE_IMPORT_DESCRIPTOR pImport = GetImportTable(pBase);
    
    // 3. 在kernel32.dll的导入表中插入我们的DLL
    InsertNewDll(pImport, "MinecraftUnifier.dll");
    
    // 4. 写入修改后的文件
    FlushViewOfFile(pBase, 0);
}
```

**Hook框架**：使用Detours库

```cpp
// 安装API Hook
void InstallHooks() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    
    // Hook文件操作
    DetourAttach(&(PVOID&)original_CreateFileW, Hooked_CreateFileW);
    
    // Hook OpenGL
    DetourAttach(&(PVOID&)original_glShaderSource, Hooked_glShaderSource);
    
    DetourTransactionCommit();
}
```

#### Linux平台

**注入方式**：ELF文件PT_NOTE注入 + LD_PRELOAD持久化

```cpp
// 通过修改ELF文件的动态段，添加NEEDED条目
bool InjectToELF(const char* targetPath, const char* outputPath) {
    // 1. 打开ELF文件
    int fd = open(targetPath, O_RDWR);
    void* map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    // 2. 查找动态段
    Elf64_Dyn* dyn = FindDynamicSection(map);
    
    // 3. 在动态段中添加NEEDED条目
    InsertNeededLibrary(dyn, "libMinecraftUnifier.so");
    
    // 4. 写入修改后的文件
    msync(map, st.st_size, MS_SYNC);
}
```

**Hook框架**：PLT Hook

```cpp
// 修改PLT表，替换函数地址
void InstallPltHooks() {
    struct link_map* map;
    dlinfo(RTLD_SELF, RTLD_DI_LINKMAP, &map);
    
    // 遍历所有已加载的共享库
    while (map) {
        if (strstr(map->l_name, "libminecraftpe.so")) {
            // 修改该库的PLT表
            ElfW(Rel)* rel = GetRelocationTable(map);
            
            // 替换函数地址
            for (int i = 0; i < num_rel; i++) {
                if (IsHookedSymbol(rel[i])) {
                    void* addr = (void*)(map->l_addr + rel[i].r_offset);
                    mprotect(addr, 0x1000, PROT_READ | PROT_WRITE);
                    *(void**)addr = hook_func;
                }
            }
        }
        map = map->l_next;
    }
}
```

#### Android平台

**注入方式**：APK重打包 + so库预加载

```python
# 修改APK的AndroidManifest.xml和smali代码
class APKModifier:
    def modify_smali(self):
        # 在MainActivity.smali的onCreate方法开头插入加载代码
        insert_code = '''
    const-string v0, "MinecraftUnifier"
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V
'''
        content = content.replace(
            ".method protected onCreate(Landroid/os/Bundle;)V",
            ".method protected onCreate(Landroid/os/Bundle;)V\n" + insert_code
        )
```

**Hook框架**：使用xHook库

```cpp
// 注册Hook
void InstallHooks(void* libHandle) {
    xhook_register(".*libminecraftpe\\.so$", "fopen",
                   (void*)hooked_fopen, (void**)&orig_fopen);
    
    xhook_register(".*libc\\.so$", "open",
                   (void*)hooked_open, (void**)&orig_open);
    
    xhook_refresh(0);
}
```

---

## 核心模块详解

### 1. .cmc格式（跨平台模组封装格式）

#### 格式定义

```cpp
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
```

#### 支持的模组类型

```cpp
enum class ModType {
    UNKNOWN,
    JAVA_MOD,        // Java版模组
    NETEASE_MOD,     // 网易版模组
    SHADER_PACK,     // 光影包
    RESOURCE_PACK,   // 资源包
    BEHAVIOR_PACK    // 行为包
};
```

#### 使用示例

```cpp
#include "common/cmc_format.h"

using namespace mcu::cmc;

// 打包模组
CMCPacker packer;
packer.SetCompression(true, 6);
packer.Pack("./mod_directory", "./output.cmc");

// 解包模组
CMCPacker unpacker;
unpacker.Unpack("./input.cmc", "./output_directory");

// 获取manifest信息
CMCManifest manifest;
packer.GetManifest("./input.cmc", manifest);
std::cout << "Mod name: " << manifest.name << std::endl;
std::cout << "Mod version: " << manifest.version << std::endl;
```

### 2. 网易项目打包器

#### Java模组转换器

```cpp
class JavaModConverter {
public:
    // 转换Java模组为.cmc格式
    bool Convert(const std::string& inputJarPath, const std::string& outputCmcPath);
    
    // 设置API映射配置
    void SetApiMappings(const std::string& configPath);
    
private:
    // 解压JAR文件
    bool ExtractJar(const std::string& jarPath, const std::string& extractDir);
    
    // 转换Java类
    bool ConvertJavaClasses(const std::string& classesDir);
    
    // 生成manifest
    bool GenerateManifest(const std::string& jarPath, CMCManifest& manifest);
};
```

#### 网易模组转换器

```cpp
class NeteaseModConverter {
public:
    // 转换网易模组为.cmc格式
    bool Convert(const std::string& inputModPath, const std::string& outputCmcPath);
    
private:
    // 转换Python脚本
    bool ConvertPythonScripts(const std::string& scriptsDir);
    
    // 转换资源
    bool ConvertResources(const std::string& resourcesDir);
};
```

#### 光影包转换器

```cpp
class ShaderPackConverter {
public:
    // 转换Java版光影包为.cmc格式
    bool Convert(const std::string& inputShaderPath, const std::string& outputCmcPath);
    
private:
    // 转换GLSL着色器
    bool ConvertGlslShaders(const std::string& shadersDir);
    
    // 转换配置文件
    bool ConvertProperties(const std::string& configDir);
};
```

### 3. 渲染兼容模块

#### GLSL到Render Dragon转换

```cpp
class ShaderConverter {
public:
    // 从Java版光影包加载
    bool LoadJavaShaderpack(const std::string& shaderpackPath);
    
    // 编译为Render Dragon材质
    bool CompileToRenderDragon(const std::string& materialName);
    
    // 更新Uniform值
    void UpdateUniforms(void* material, const std::unordered_map<std::string, float>& values);
    
private:
    // 编译GLSL到SPIR-V
    bool CompileGLSLToSPIRV(ShaderInfo& shader);
    
    // 解析Uniform变量
    bool ParseUniforms(ShaderInfo& shader);
};
```

#### Render Dragon API封装

```cpp
class RenderDragonAPI {
public:
    static RenderDragonAPI& GetInstance();
    
    // 创建材质
    void* CreateMaterial(const char* name);
    
    // 设置着色器阶段
    bool SetShaderStage(void* material, int stage, const uint32_t* spirv, size_t size);
    
    // 添加Uniform
    bool AddUniform(void* material, const char* name, int location);
    
    // 设置Uniform值
    bool SetUniformValue(void* material, const char* name, const float* value, int count);
};
```

### 4. 模组兼容模块

#### Java模组运行时

```cpp
class JavaModRuntime {
public:
    // 初始化JVM
    bool Initialize();
    
    // 加载模组
    bool LoadMod(const std::string& jarPath);
    
    // 调用Java方法
    bool CallJavaMethod(const std::string& className,
                       const std::string& methodName,
                       const std::string& signature, ...);
    
    // 注册本地方法
    bool RegisterNativeMethod(const std::string& className,
                             const std::string& methodName,
                             const std::string& signature,
                             void* func);
};
```

#### 网易模组运行时

```cpp
class NeteaseModRuntime {
public:
    // 初始化Python解释器
    bool Initialize();
    
    // 加载模组
    bool LoadMod(const std::string& modPath);
    
    // 调用Python函数
    bool CallPythonFunction(const std::string& module,
                           const std::string& function,
                           PyObject* args);
    
    // 注册C函数到Python
    bool RegisterCFunction(const std::string& moduleName,
                          const std::string& functionName,
                          PyCFunction func, const char* doc);
};
```

### 5. 资源管理模块

#### 文件系统Hook

```cpp
class ResourceManager {
public:
    // 初始化
    bool Initialize();
    
    // 添加重定向规则
    void AddRedirectRule(const std::string& from, const std::string& to, int priority = 0);
    
    // 应用重定向
    std::string ApplyRedirect(const std::string& originalPath);
    
    // 安装文件系统Hook
    bool InstallFileHooks();
    
private:
    // Hook回调函数
#ifdef _WIN32
    static HANDLE WINAPI Hooked_CreateFileW(LPCWSTR lpFileName, ...);
#else
    static FILE* Hooked_fopen(const char* path, const char* mode);
#endif
};
```

#### 资源转换器

```cpp
class ResourceConverter {
public:
    // 转换纹理
    static bool ConvertTexture(const std::string& inputPath, const std::string& outputPath);
    
    // 转换模型
    static bool ConvertModel(const std::string& inputPath, const std::string& outputPath);
    
    // 转换声音
    static bool ConvertSound(const std::string& inputPath, const std::string& outputPath);
    
    // 检测资源类型
    static ResourceType DetectType(const std::string& path);
};
```

---

## API参考

### 通用API

#### CMCPacker

```cpp
namespace mcu::cmc {

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
};

}
```

### 打包器API

#### UnifiedPacker

```cpp
namespace mcu::packer::windows {

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
};

}
```

### 注入器API

#### UnifiedAndroidInjector

```cpp
namespace mcu::injector::android {

class UnifiedAndroidInjector {
public:
    UnifiedAndroidInjector();
    ~UnifiedAndroidInjector();
    
    // 注入到APK
    bool InjectToApk(const std::string& inputApk, const std::string& outputApk);
    
    // 设置配置
    void SetConfig(const std::string& configPath);
    
    // 添加兼容模块
    void AddCompatModule(const std::string& modulePath);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);
};

}
```

#### UnifiedWindowsInjector

```cpp
namespace mcu::injector::windows {

class UnifiedWindowsInjector {
public:
    UnifiedWindowsInjector();
    ~UnifiedWindowsInjector();
    
    // 注入到PE文件
    bool InjectToPE(const std::string& inputExe, const std::string& outputExe);
    
    // 设置配置
    void SetConfig(const std::string& configPath);
    
    // 添加兼容模块
    void AddCompatModule(const std::string& modulePath);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);
};

}
```

#### UnifiedLinuxInjector

```cpp
namespace mcu::injector::linux {

class UnifiedLinuxInjector {
public:
    UnifiedLinuxInjector();
    ~UnifiedLinuxInjector();
    
    // 注入到ELF文件
    bool InjectToELF(const std::string& inputElf, const std::string& outputElf);
    
    // 设置配置
    void SetConfig(const std::string& configPath);
    
    // 添加兼容模块
    void AddCompatModule(const std::string& modulePath);
    
    // 进度回调
    using ProgressCallback = std::function<void(int percent, const std::string& message)>;
    void SetProgressCallback(ProgressCallback callback);
};

}
```

### 核心API

#### ShaderConverter

```cpp
namespace mcu::core::render {

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
};

}
```

#### JavaModRuntime

```cpp
namespace mcu::core::mods {

class JavaModRuntime {
public:
    JavaModRuntime();
    ~JavaModRuntime();
    
    // 初始化JVM
    bool Initialize();
    
    // 关闭JVM
    void Shutdown();
    
    // 加载模组
    bool LoadMod(const std::string& jarPath);
    
    // 卸载模组
    bool UnloadMod(const std::string& modId);
    
    // 调用Java方法
    bool CallJavaMethod(const std::string& className,
                       const std::string& methodName,
                       const std::string& signature, ...);
    
    // 注册本地方法
    bool RegisterNativeMethod(const std::string& className,
                             const std::string& methodName,
                             const std::string& signature,
                             void* func);
    
    // 获取已加载的模组列表
    std::vector<JavaModInfo> GetLoadedMods() const;
};

}
```

#### NeteaseModRuntime

```cpp
namespace mcu::core::mods {

class NeteaseModRuntime {
public:
    NeteaseModRuntime();
    ~NeteaseModRuntime();
    
    // 初始化Python解释器
    bool Initialize();
    
    // 关闭Python解释器
    void Shutdown();
    
    // 加载模组
    bool LoadMod(const std::string& modPath);
    
    // 卸载模组
    bool UnloadMod(const std::string& modId);
    
    // 调用Python函数
    bool CallPythonFunction(const std::string& module,
                           const std::string& function,
                           PyObject* args);
    
    // 注册C函数到Python
    bool RegisterCFunction(const std::string& moduleName,
                          const std::string& functionName,
                          PyCFunction func, const char* doc);
    
    // 获取已加载的模组列表
    std::vector<NeteaseModInfo> GetLoadedMods() const;
};

}
```

#### ResourceManager

```cpp
namespace mcu::core::resources {

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    
    // 初始化
    bool Initialize();
    
    // 关闭
    void Shutdown();
    
    // 添加重定向规则
    void AddRedirectRule(const std::string& from, const std::string& to, int priority = 0);
    
    // 应用重定向
    std::string ApplyRedirect(const std::string& originalPath);
    
    // 添加资源
    bool AddResource(const std::string& originalPath, const std::string& redirectPath);
    
    // 转换资源
    bool ConvertResource(const std::string& originalPath, const std::string& outputPath);
    
    // 批量转换资源
    bool BatchConvert(const std::string& inputDir, const std::string& outputDir);
    
    // 安装文件系统Hook
    bool InstallFileHooks();
};

}
```

---

## 开发指南

### 环境配置

#### Windows

```bash
# 安装Visual Studio 2019或更高版本
# 安装CMake 3.16+
# 安装Python 3.8+
# 安装Detours库

# 克隆项目
git clone https://github.com/your-repo/minecraft-unifier.git
cd minecraft-unifier

# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake -G "Visual Studio 16 2019" -A x64 ..

# 编译
cmake --build . --config Release
```

#### Linux

```bash
# 安装依赖
sudo apt-get update
sudo apt-get install build-essential cmake zlib1g-dev libpython3-dev

# 克隆项目
git clone https://github.com/your-repo/minecraft-unifier.git
cd minecraft-unifier

# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# 编译
make -j$(nproc)
```

#### Android

```bash
# 安装Android NDK
# 设置环境变量
export ANDROID_NDK=/path/to/ndk

# 克隆项目
git clone https://github.com/your-repo/minecraft-unifier.git
cd minecraft-unifier

# 创建构建目录
mkdir build && cd build

# 配置CMake
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 ..

# 编译
cmake --build .
```

### 代码规范

#### 命名约定

- **类名**：PascalCase（如 `ShaderConverter`）
- **函数名**：PascalCase（如 `LoadJavaShaderpack`）
- **变量名**：camelCase（如 `shaderpackPath`）
- **常量名**：UPPER_CASE（如 `MAX_SHADER_COUNT`）
- **命名空间**：lowercase（如 `mcu::core::render`）

#### 文件组织

- 每个类一个头文件和一个实现文件
- 头文件使用 `.h` 扩展名
- 实现文件使用 `.cpp` 扩展名
- 头文件包含保护使用 `#pragma once`

#### 注释规范

```cpp
/**
 * 类的简要描述
 * 
 * 详细描述（可选）
 */
class MyClass {
public:
    /**
     * 函数的简要描述
     * 
     * @param param1 参数1的描述
     * @param param2 参数2的描述
     * @return 返回值的描述
     */
    bool MyFunction(int param1, const std::string& param2);
    
private:
    int privateVar_;  // 私有变量的描述
};
```

### 测试指南

#### 测试框架

项目使用Google Test（gtest）作为测试框架，支持单元测试、集成测试、端到端测试、跨平台兼容性测试和性能测试。

#### 单元测试

单元测试覆盖项目的各个模块，包括打包器、注入器、核心功能等。

**打包器测试**（`tests/packer/packer_test.cpp`）：
- Java模组打包测试
- 网易模组打包测试
- 光影包打包测试
- 统一打包器测试
- CMC文件解包测试
- 批量打包测试

**注入器测试**（`tests/injector/injector_test.cpp`）：
- PE文件解析测试
- ELF文件解析测试
- APK文件解析测试
- Windows导入表劫持测试
- Linux PT_NOTE段注入测试
- Android APK注入测试
- Windows Detours Hook测试
- Linux PLT Hook测试
- Android xHook测试
- 注入安全性验证测试
- 注入回滚测试

**核心功能测试**（`tests/core/core_test.cpp`）：
- GLSL着色器转换测试
- SPIR-V编译测试
- 着色器缓存测试
- Java模组运行时初始化/加载/卸载/方法调用测试
- 网易模组运行时初始化/加载/卸载/函数调用测试
- 资源管理器初始化/重定向规则/资源类型检测/资源包加载/卸载/启用/禁用/排序测试
- CMC文件格式测试

**运行单元测试**：

```bash
# 编译测试
cd build
cmake --build . --target tests

# 运行所有测试
ctest

# 运行特定测试
./tests/packer_test
./tests/injector_test
./tests/core_test

# 运行特定测试用例
./tests/packer_test --gtest_filter=PackerTest.PackJavaMod
```

#### 集成测试

集成测试覆盖项目的完整工作流程，包括端到端测试、跨平台兼容性测试和性能测试。

**端到端测试**（`tests/e2e/e2e_test.cpp`）：
- 完整的Java模组工作流程
- 完整的网易模组工作流程
- 完整的光影包工作流程
- 批量打包多个模组
- 资源包管理工作流程
- 多模组依赖管理
- 完整的错误处理流程
- 完整的配置和设置流程
- 完整的日志记录流程
- 完整的备份和恢复流程

**跨平台兼容性测试**（`tests/cross_platform/cross_platform_test.cpp`）：
- 平台检测
- 文件路径兼容性
- 文件权限兼容性
- CMC格式跨平台兼容性
- Java模组运行时跨平台兼容性
- 网易模组运行时跨平台兼容性
- 资源管理器跨平台兼容性
- Windows/Linux/Android平台特定功能
- 跨平台文件系统操作
- 跨平台目录操作
- 跨平台路径操作
- 跨平台编码兼容性
- 跨平台时间戳兼容性

**性能测试**（`tests/performance/performance_test.cpp`）：
- 单个模组打包性能
- 批量打包性能
- CMC文件读取/解包性能
- 着色器转换性能
- 批量着色器转换性能
- 资源管理器初始化/加载性能
- 批量资源包加载性能
- 重定向规则查询性能
- 大文件处理性能
- 内存使用测试
- 并发性能测试
- 缓存性能测试
- 长时间运行稳定性测试

**运行集成测试**：

```bash
# 运行端到端测试
./tests/e2e_test

# 运行跨平台兼容性测试
./tests/cross_platform_test

# 运行性能测试
./tests/performance_test

# 运行所有集成测试
ctest --label integration
```

#### 测试覆盖率

项目要求达到以下测试覆盖率目标：

- **单元测试覆盖率**：≥ 80%
- **集成测试覆盖率**：≥ 70%
- **端到端测试覆盖率**：≥ 60%

**生成覆盖率报告**：

```bash
# 使用gcov生成覆盖率报告
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON ..
cmake --build .
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

#### 持续集成测试

项目支持持续集成（CI）测试，在每次代码提交时自动运行测试。

**CI测试流程**：

1. 代码提交到仓库
2. CI系统自动触发构建
3. 运行所有单元测试
4. 运行所有集成测试
5. 生成测试覆盖率报告
6. 如果测试失败，阻止合并

**CI配置示例**（`.github/workflows/test.yml`）：

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install -y cmake g++ libgtest-dev
      - name: Build
        run: |
          mkdir build && cd build
          cmake ..
          cmake --build .
      - name: Run tests
        run: |
          cd build
          ctest --output-on-failure
      - name: Generate coverage
        run: |
          cd build
          lcov --capture --directory . --output-file coverage.info
          lcov --list coverage.info
```

#### 测试最佳实践

1. **测试命名**：使用描述性的测试名称，如`PackJavaMod`、`InjectToApk`
2. **测试隔离**：每个测试用例应该独立运行，不依赖其他测试
3. **测试清理**：在`TearDown()`中清理测试资源
4. **测试断言**：使用明确的断言，如`ASSERT_TRUE`、`EXPECT_EQ`
5. **测试数据**：使用测试数据文件，避免硬编码测试数据
6. **测试超时**：为长时间运行的测试设置超时时间
7. **测试日志**：在测试失败时输出详细的日志信息

**示例测试用例**：

```cpp
#include <gtest/gtest.h>
#include "packer/windows/java_packer.h"

namespace mcu::packer::windows::test {

class JavaPackerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录
        test_dir_ = "/tmp/minecraft-unifier-test";
        std::filesystem::create_directories(test_dir_);
    }
    
    void TearDown() override {
        // 清理测试目录
        std::filesystem::remove_all(test_dir_);
    }
    
    std::string test_dir_;
};

TEST_F(JavaPackerTest, PackJavaMod) {
    // 创建测试Java模组
    std::string mod_dir = CreateTestJavaMod("testmod", "Test Mod", "1.0.0");
    
    // 创建Java模组打包器
    JavaPacker packer;
    
    // 打包为.cmc格式
    std::string cmc_path = test_dir_ + "/testmod.cmc";
    bool result = packer.Pack(mod_dir, cmc_path, "java");
    
    // 验证结果
    ASSERT_TRUE(result) << "Failed to pack Java mod";
    ASSERT_TRUE(std::filesystem::exists(cmc_path)) << "CMC file not created";
    
    // 验证CMC文件
    CMCPacker cmc_packer;
    CMCHeader header;
    result = cmc_packer.ReadHeader(cmc_path, header);
    
    ASSERT_TRUE(result) << "Failed to read CMC header";
    EXPECT_EQ(header.magic, CMC_MAGIC) << "Invalid CMC magic number";
    EXPECT_EQ(header.mod_type, ModType::Java) << "Invalid mod type";
}

} // namespace mcu::packer::windows::test
```

### 调试技巧

#### 日志输出

```cpp
#include <iostream>

// 使用标准输出
std::cout << "Debug info: " << info << std::endl;

// 使用错误输出
std::cerr << "Error: " << error << std::endl;

// Android日志
#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "MinecraftUnifier"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif
```

#### 断言检查

```cpp
#include <cassert>

// 运行时断言
assert(condition && "Error message");

// 自定义断言
#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "Check failed: " << message << std::endl; \
            std::abort(); \
        } \
    } while (0)
```

---

## 部署指南

### Windows部署

#### 打包

```bash
# 使用NSIS创建安装程序
makensis installer.nsi

# 或使用Inno Setup
iscc installer.iss
```

#### 安装

1. 运行安装程序
2. 选择安装目录
3. 选择要安装的组件
4. 完成安装

#### 使用

```bash
# 打包模组
MinecraftUnifierPacker.exe --input mod.jar --output mod.cmc

# 注入到游戏
MinecraftUnifierInjector.exe --input "Minecraft.exe" --output "Minecraft.Unified.exe"
```

### Linux部署

#### 打包

```bash
# 创建Debian包
dpkg-buildpackage -us -uc

# 或创建RPM包
rpmbuild -bb minecraft-unifier.spec
```

#### 安装

```bash
# Debian/Ubuntu
sudo dpkg -i minecraft-unifier_2.0.0_amd64.deb

# Fedora/RHEL
sudo rpm -i minecraft-unifier-2.0.0-1.x86_64.rpm
```

#### 使用

```bash
# 打包模组
minecraft-unifier-packer --input mod.jar --output mod.cmc

# 注入到游戏
minecraft-unifier-injector --input minecraft-pe --output minecraft-pe.unified
```

### Android部署

#### 打包

```bash
# 创建APK
./gradlew assembleRelease

# 签名APK
jarsigner -keystore keystore.jks app-release-unsigned.apk unifier
zipalign -v 4 app-release-unsigned.apk app-release.apk
```

#### 安装

```bash
# 通过ADB安装
adb install app-release.apk

# 或直接安装APK文件
```

#### 使用

```bash
# 打包模组
adb shell minecraft-unifier-packer --input /sdcard/mod.jar --output /sdcard/mod.cmc

# 注入到游戏
adb shell minecraft-unifier-injector --input /sdcard/Minecraft.apk --output /sdcard/Minecraft.Unified.apk
```

---

## 贡献指南

### 如何贡献

1. Fork项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启Pull Request

### 代码审查

- 确保代码符合项目规范
- 添加必要的测试
- 更新相关文档
- 确保所有测试通过

### 问题报告

- 使用GitHub Issues报告问题
- 提供详细的错误信息
- 包含复现步骤
- 附上相关的日志和截图

---

## 常见问题

### Q1: 注入后游戏无法启动？

**A**: 请检查以下几点：
1. 确保游戏文件是正版
2. 检查注入器是否正确执行
3. 查看日志文件获取详细错误信息
4. 确保兼容模块已正确放置

### Q2: 光影包无法加载？

**A**: 请检查：
1. 光影包是否为Java版格式
2. GLSL着色器是否正确转换
3. Render Dragon API是否正确初始化
4. 查看着色器缓存是否有错误

### Q3: Java模组无法运行？

**A**: 请检查：
1. JVM是否正确初始化
2. API映射是否正确配置
3. 模组是否正确加载
4. 查看Java日志获取详细错误信息

### Q4: 网易模组无法运行？

**A**: 请检查：
1. Python解释器是否正确初始化
2. SDK模拟层是否正确注册
3. 模组脚本是否正确转换
4. 查看Python日志获取详细错误信息

### Q5: 资源包无法加载？

**A**: 请检查：
1. 文件系统Hook是否正确安装
2. 重定向规则是否正确配置
3. 资源格式是否正确转换
4. 查看文件访问日志

---

## 许可证

MIT License

Copyright (c) 2026 Minecraft Unifier Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

## 联系方式

- **邮箱**: jqyh1026@outlook.com

---

## 致谢

感谢所有为这个项目做出贡献的开发者和测试者！

特别感谢：
- Mojang Studios开发了《我的世界》
- Minecraft社区提供的各种模组和资源
- 开源社区提供的各种工具和库

---

**注意**: 本项目仍在开发中，部分功能可能不稳定。请谨慎使用。
