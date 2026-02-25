# 我的世界统一器 - API文档

## 目录

- [通用API](#通用api)
- [打包器API](#打包器api)
- [注入器API](#注入器api)
- [核心API](#核心api)
- [GUI API](#gui-api)

---

## 通用API

### CMCPacker

跨平台模组封装（.cmc格式）打包器。

#### 头文件

```cpp
#include "common/cmc_format.h"
```

#### 命名空间

```cpp
namespace mcu::cmc {
```

#### 类定义

```cpp
class CMCPacker {
public:
    CMCPacker();
    ~CMCPacker();
    
    // 打包目录为.cmc文件
    bool Pack(const std::string& inputDir, const std::string& outputCmcPath);
    
    // 解包.cmc文件到目录
    bool Unpack(const std::string& inputCmcPath, const std::string& outputDir);
    
    // 验证.cmc文件
    bool Validate(const std::string& cmcPath);
    
    // 读取CMC头
    bool ReadHeader(const std::string& cmcPath, CMCHeader& header);
    
    // 写入CMC头
    bool WriteHeader(const std::string& cmcPath, const CMCHeader& header);
    
    // 获取manifest
    bool GetManifest(const std::string& cmcPath, CMCManifest& manifest);
    
    // 设置压缩
    void SetCompression(bool enable, int level = 6);
    
    // 设置加密
    void SetEncryption(bool enable, const std::string& password = "");
    
private:
    bool compressionEnabled_;
    int compressionLevel_;
    bool encryptionEnabled_;
    std::string encryptionPassword_;
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

---

## 打包器API

### JavaPacker

Java模组打包器，将Java模组转换为.cmc格式。

#### 头文件

```cpp
#include "packer/windows/java_packer.h"
```

#### 命名空间

```cpp
namespace mcu::packer::windows {
```

#### 类定义

```cpp
class JavaPacker {
public:
    JavaPacker();
    ~JavaPacker();
    
    // 打包Java模组
    bool Pack(const std::string& inputJarPath, const std::string& outputCmcPath, const std::string& modType = "java");
    
    // 设置API映射配置
    void SetApiMappings(const std::string& configPath);
    
    // 设置输出目录
    void SetOutputDir(const std::string& outputDir);
    
private:
    std::string outputDir_;
    std::string apiMappingsPath_;
};
```

#### 使用示例

```cpp
#include "packer/windows/java_packer.h"

using namespace mcu::packer::windows;

// 创建Java模组打包器
JavaPacker packer;
packer.SetOutputDir("./output");

// 打包Java模组
bool result = packer.Pack("mod.jar", "mod.cmc", "java");
if (result) {
    std::cout << "Java mod packed successfully" << std::endl;
}
```

### NeteasePacker

网易模组打包器，将网易模组转换为.cmc格式。

#### 头文件

```cpp
#include "packer/windows/netease_packer.h"
```

#### 命名空间

```cpp
namespace mcu::packer::windows {
```

#### 类定义

```cpp
class NeteasePacker {
public:
    NeteasePacker();
    ~NeteasePacker();
    
    // 打包网易模组
    bool Pack(const std::string& inputModPath, const std::string& outputCmcPath, const std::string& modType = "netease");
    
    // 设置输出目录
    void SetOutputDir(const std::string& outputDir);
    
private:
    std::string outputDir_;
};
```

#### 使用示例

```cpp
#include "packer/windows/netease_packer.h"

using namespace mcu::packer::windows;

// 创建网易模组打包器
NeteasePacker packer;
packer.SetOutputDir("./output");

// 打包网易模组
bool result = packer.Pack("netease_mod_project/", "mod.cmc", "netease");
if (result) {
    std::cout << "Netease mod packed successfully" << std::endl;
}
```

### ShaderPacker

光影包打包器，将光影包转换为.cmc格式。

#### 头文件

```cpp
#include "packer/windows/shader_packer.h"
```

#### 命名空间

```cpp
namespace mcu::packer::windows {
```

#### 类定义

```cpp
class ShaderPacker {
public:
    ShaderPacker();
    ~ShaderPacker();
    
    // 打包光影包
    bool Pack(const std::string& inputShaderPath, const std::string& outputCmcPath, const std::string& modType = "shader");
    
    // 设置输出目录
    void SetOutputDir(const std::string& outputDir);
    
private:
    std::string outputDir_;
};
```

#### 使用示例

```cpp
#include "packer/windows/shader_packer.h"

using namespace mcu::packer::windows;

// 创建光影包打包器
ShaderPacker packer;
packer.SetOutputDir("./output");

// 打包光影包
bool result = packer.Pack("shaderpack/", "shader.cmc", "shader");
if (result) {
    std::cout << "Shader pack packed successfully" << std::endl;
}
```

### UnifiedPacker

统一打包器，支持所有类型的模组打包。

#### 头文件

```cpp
#include "packer/windows/unified_packer.h"
```

#### 命名空间

```cpp
namespace mcu::packer::windows {
```

#### 类定义

```cpp
class UnifiedPacker {
public:
    UnifiedPacker();
    ~UnifiedPacker();
    
    // 打包模组（自动检测类型）
    bool Pack(const std::string& inputPath, const std::string& outputCmcPath, const std::string& modType = "auto");
    
    // 批量打包
    bool BatchPack(const std::vector<std::string>& inputPaths, const std::string& outputDir, const std::string& modType = "auto");
    
    // 设置输出目录
    void SetOutputDir(const std::string& outputDir);
    
    // 设置API映射配置
    void SetApiMappings(const std::string& configPath);
    
private:
    std::string outputDir_;
    std::string apiMappingsPath_;
};
```

#### 使用示例

```cpp
#include "packer/windows/unified_packer.h"

using namespace mcu::packer::windows;

// 创建统一打包器
UnifiedPacker packer;
packer.SetOutputDir("./output");

// 打包模组（自动检测类型）
bool result = packer.Pack("mod.jar", "mod.cmc");
if (result) {
    std::cout << "Mod packed successfully" << std::endl;
}

// 批量打包
std::vector<std::string> modPaths = {"mod1.jar", "mod2.jar", "mod3.jar"};
result = packer.BatchPack(modPaths, "./output", "java");
if (result) {
    std::cout << "Batch packing completed successfully" << std::endl;
}
```

---

## 注入器API

### UnifiedWindowsInjector

Windows平台注入器，支持PE文件导入表劫持和Detours Hook。

#### 头文件

```cpp
#include "injector/windows/pe_injector.h"
```

#### 命名空间

```cpp
namespace mcu::injector::windows {
```

#### 类定义

```cpp
class UnifiedWindowsInjector {
public:
    UnifiedWindowsInjector();
    ~UnifiedWindowsInjector();
    
    // 解析PE文件
    bool ParsePE(const std::string& pePath, PEInfo& info);
    
    // 注入DLL到PE文件
    bool InjectDLL(const std::string& targetPath, const std::string& dllPath);
    
    // 验证文件
    bool ValidateFile(const std::string& filePath);
    
    // 备份文件
    bool BackupFile(const std::string& filePath, const std::string& backupPath);
    
    // 恢复文件
    bool RestoreFile(const std::string& backupPath, const std::string& filePath);
    
    // 获取Hook框架
    WindowsHookFramework& GetHookFramework();
    
private:
    WindowsHookFramework hookFramework_;
};
```

#### 使用示例

```cpp
#include "injector/windows/pe_injector.h"

using namespace mcu::injector::windows;

// 创建Windows注入器
UnifiedWindowsInjector injector;

// 解析PE文件
PEInfo info;
bool result = injector.ParsePE("Minecraft.exe", info);
if (result) {
    std::cout << "PE file parsed successfully" << std::endl;
}

// 注入DLL
result = injector.InjectDLL("Minecraft.exe", "MinecraftUnifier.dll");
if (result) {
    std::cout << "DLL injected successfully" << std::endl;
}
```

### UnifiedLinuxInjector

Linux平台注入器，支持ELF文件PT_NOTE注入和PLT Hook。

#### 头文件

```cpp
#include "injector/linux/elf_injector.h"
```

#### 命名空间

```cpp
namespace mcu::injector::linux {
```

#### 类定义

```cpp
class UnifiedLinuxInjector {
public:
    UnifiedLinuxInjector();
    ~UnifiedLinuxInjector();
    
    // 解析ELF文件
    bool ParseELF(const std::string& elfPath, ELFInfo& info);
    
    // 注入库到ELF文件
    bool InjectLibrary(const std::string& targetPath, const std::string& libPath);
    
    // 验证文件
    bool ValidateFile(const std::string& filePath);
    
    // 备份文件
    bool BackupFile(const std::string& filePath, const std::string& backupPath);
    
    // 恢复文件
    bool RestoreFile(const std::string& backupPath, const std::string& filePath);
    
    // 获取Hook框架
    LinuxHookFramework& GetHookFramework();
    
private:
    LinuxHookFramework hookFramework_;
};
```

#### 使用示例

```cpp
#include "injector/linux/elf_injector.h"

using namespace mcu::injector::linux;

// 创建Linux注入器
UnifiedLinuxInjector injector;

// 解析ELF文件
ELFInfo info;
bool result = injector.ParseELF("minecraft-pe", info);
if (result) {
    std::cout << "ELF file parsed successfully" << std::endl;
}

// 注入库
result = injector.InjectLibrary("minecraft-pe", "libMinecraftUnifier.so");
if (result) {
    std::cout << "Library injected successfully" << std::endl;
}
```

### UnifiedAndroidInjector

Android平台注入器，支持APK重打包和xHook。

#### 头文件

```cpp
#include "injector/android/apk_injector.h"
```

#### 命名空间

```cpp
namespace mcu::injector::android {
```

#### 类定义

```cpp
class UnifiedAndroidInjector {
public:
    UnifiedAndroidInjector();
    ~UnifiedAndroidInjector();
    
    // 解析APK文件
    bool ParseAPK(const std::string& apkPath, APKInfo& info);
    
    // 注入so库到APK文件
    bool InjectSO(const std::string& inputApkPath, const std::string& soPath, const std::string& outputApkPath);
    
    // 验证文件
    bool ValidateFile(const std::string& filePath);
    
    // 备份文件
    bool BackupFile(const std::string& filePath, const std::string& backupPath);
    
    // 恢复文件
    bool RestoreFile(const std::string& backupPath, const std::string& filePath);
    
    // 获取Hook框架
    AndroidHookFramework& GetHookFramework();
    
private:
    AndroidHookFramework hookFramework_;
};
```

#### 使用示例

```cpp
#include "injector/android/apk_injector.h"

using namespace mcu::injector::android;

// 创建Android注入器
UnifiedAndroidInjector injector;

// 解析APK文件
APKInfo info;
bool result = injector.ParseAPK("Minecraft.apk", info);
if (result) {
    std::cout << "APK file parsed successfully" << std::endl;
}

// 注入so库
result = injector.InjectSO("Minecraft.apk", "libMinecraftUnifier.so", "Minecraft.Unified.apk");
if (result) {
    std::cout << "SO injected successfully" << std::endl;
}
```

---

## 核心API

### ShaderConverter

着色器转换器，将Java版GLSL着色器转换为Render Dragon兼容格式。

#### 头文件

```cpp
#include "core/render/shader_converter.h"
```

#### 命名空间

```cpp
namespace mcu::core::render {
```

#### 类定义

```cpp
class ShaderConverter {
public:
    ShaderConverter();
    ~ShaderConverter();
    
    // 转换GLSL着色器为Render Dragon格式
    bool ConvertGLSLToRenderDragon(const std::string& inputPath, const std::string& outputPath);
    
    // 编译为SPIR-V
    bool CompileToSPIRV(const std::string& inputPath, const std::string& outputPath);
    
    // 清除缓存
    void ClearCache();
    
    // 设置缓存目录
    void SetCacheDir(const std::string& cacheDir);
    
private:
    std::string cacheDir_;
    std::unordered_map<std::string, std::string> shaderCache_;
};
```

#### 使用示例

```cpp
#include "core/render/shader_converter.h"

using namespace mcu::core::render;

// 创建着色器转换器
ShaderConverter converter;

// 转换GLSL着色器
bool result = converter.ConvertGLSLToRenderDragon("terrain.vsh", "terrain_rd.vsh");
if (result) {
    std::cout << "Shader converted successfully" << std::endl;
}

// 编译为SPIR-V
result = converter.CompileToSPIRV("terrain.vsh", "terrain.spv");
if (result) {
    std::cout << "Shader compiled to SPIR-V successfully" << std::endl;
}
```

### JavaModRuntime

Java模组运行时，支持嵌入式JVM和Java模组加载。

#### 头文件

```cpp
#include "core/mods/java_runtime.h"
```

#### 命名空间

```cpp
namespace mcu::core::mods {
```

#### 类定义

```cpp
class JavaModRuntime {
public:
    JavaModRuntime();
    ~JavaModRuntime();
    
    // 初始化运行时
    bool Initialize();
    
    // 关闭运行时
    void Shutdown();
    
    // 加载模组
    bool LoadMod(const std::string& modPath);
    
    // 卸载模组
    bool UnloadMod(const std::string& modId);
    
    // 调用Java方法
    bool CallJavaMethod(const std::string& modId, const std::string& className, 
                       const std::string& methodName, const std::string& signature, ...);
    
    // 获取已加载的模组列表
    std::vector<ModInfo> GetLoadedMods() const;
    
    // 注册C函数到Java
    bool RegisterCFunction(const std::string& className, const std::string& methodName,
                          const std::string& signature, void* func);
    
private:
    JavaVM* jvm_;
    JNIEnv* env_;
    std::unordered_map<std::string, ModInfo> loadedMods_;
};
```

#### 使用示例

```cpp
#include "core/mods/java_runtime.h"

using namespace mcu::core::mods;

// 创建Java模组运行时
JavaModRuntime runtime;

// 初始化运行时
bool result = runtime.Initialize();
if (result) {
    std::cout << "Java mod runtime initialized successfully" << std::endl;
    
    // 加载模组
    result = runtime.LoadMod("mod.cmc");
    if (result) {
        std::cout << "Mod loaded successfully" << std::endl;
        
        // 调用Java方法
        result = runtime.CallJavaMethod("mod", "com.example.ModMain", "init", "()V");
        if (result) {
            std::cout << "Java method called successfully" << std::endl;
        }
        
        // 卸载模组
        runtime.UnloadMod("mod");
    }
    
    // 关闭运行时
    runtime.Shutdown();
}
```

### NeteaseModRuntime

网易模组运行时，支持嵌入式Python和网易模组加载。

#### 头文件

```cpp
#include "core/mods/netease_runtime.h"
```

#### 命名空间

```cpp
namespace mcu::core::mods {
```

#### 类定义

```cpp
class NeteaseModRuntime {
public:
    NeteaseModRuntime();
    ~NeteaseModRuntime();
    
    // 初始化运行时
    bool Initialize();
    
    // 关闭运行时
    void Shutdown();
    
    // 加载模组
    bool LoadMod(const std::string& modPath);
    
    // 卸载模组
    bool UnloadMod(const std::string& modId);
    
    // 调用Python函数
    bool CallPythonFunction(const std::string& modId, const std::string& functionName, PyObject* args = nullptr);
    
    // 获取已加载的模组列表
    std::vector<ModInfo> GetLoadedMods() const;
    
    // 注册C函数到Python
    bool RegisterCFunction(const std::string& moduleName, const std::string& functionName,
                          PyCFunction func, const char* doc = nullptr);
    
private:
    std::unordered_map<std::string, ModInfo> loadedMods_;
};
```

#### 使用示例

```cpp
#include "core/mods/netease_runtime.h"

using namespace mcu::core::mods;

// 创建网易模组运行时
NeteaseModRuntime runtime;

// 初始化运行时
bool result = runtime.Initialize();
if (result) {
    std::cout << "Netease mod runtime initialized successfully" << std::endl;
    
    // 加载模组
    result = runtime.LoadMod("mod.cmc");
    if (result) {
        std::cout << "Mod loaded successfully" << std::endl;
        
        // 调用Python函数
        result = runtime.CallPythonFunction("mod", "init");
        if (result) {
            std::cout << "Python function called successfully" << std::endl;
        }
        
        // 卸载模组
        runtime.UnloadMod("mod");
    }
    
    // 关闭运行时
    runtime.Shutdown();
}
```

### ResourceManager

资源管理器，支持跨平台文件系统Hook和资源包管理。

#### 头文件

```cpp
#include "core/resources/resource_manager.h"
```

#### 命名空间

```cpp
namespace mcu::core::resources {
```

#### 类定义

```cpp
class ResourceManager {
public:
    // 获取单例
    static ResourceManager& GetInstance();
    
    // 初始化
    bool Initialize(const std::string& modsDir, const std::string& resourcePacksDir);
    
    // 关闭
    void Shutdown();
    
    // 添加重定向规则
    void AddRedirectRule(const std::string& from, const std::string& to, int priority = 0);
    
    // 获取重定向路径
    std::string GetRedirectedPath(const std::string& originalPath);
    
    // 加载资源包
    bool LoadResourcePack(const std::string& packPath);
    
    // 卸载资源包
    bool UnloadResourcePack(const std::string& packId);
    
    // 设置资源包启用状态
    bool SetResourcePackEnabled(const std::string& packId, bool enabled);
    
    // 设置资源包顺序
    bool SetResourcePackOrder(const std::vector<std::string>& order);
    
    // 获取已加载的资源包列表
    std::vector<ResourcePackInfo> GetLoadedResourcePacks() const;
    
    // 转换资源
    bool ConvertResource(const std::string& originalPath, const std::string& outputPath);
    
    // 批量转换资源
    bool BatchConvert(const std::string& inputDir, const std::string& outputDir);
    
private:
    ResourceManager();
    ~ResourceManager();
    
    std::unordered_map<std::string, std::string> redirectRules_;
    std::vector<ResourcePackInfo> loadedResourcePacks_;
    std::string modsDir_;
    std::string resourcePacksDir_;
};
```

#### 使用示例

```cpp
#include "core/resources/resource_manager.h"

using namespace mcu::core::resources;

// 获取资源管理器单例
ResourceManager& manager = ResourceManager::GetInstance();

// 初始化资源管理器
bool result = manager.Initialize("./mods", "./resource_packs");
if (result) {
    std::cout << "Resource manager initialized successfully" << std::endl;
    
    // 添加重定向规则
    manager.AddRedirectRule("assets/minecraft/textures/blocks/stone.png", 
                           "mods/testmod/textures/stone.png");
    
    // 获取重定向路径
    std::string redirectedPath = manager.GetRedirectedPath("assets/minecraft/textures/blocks/stone.png");
    std::cout << "Redirected path: " << redirectedPath << std::endl;
    
    // 加载资源包
    result = manager.LoadResourcePack("resourcepack.cmc");
    if (result) {
        std::cout << "Resource pack loaded successfully" << std::endl;
        
        // 获取已加载的资源包列表
        std::vector<ResourcePackInfo> packs = manager.GetLoadedResourcePacks();
        std::cout << "Loaded resource packs: " << packs.size() << std::endl;
        
        // 卸载资源包
        manager.UnloadResourcePack("resourcepack");
    }
    
    // 关闭资源管理器
    manager.Shutdown();
}
```

---

## GUI API

### MainWindow

桌面端主窗口，使用Qt6实现。

#### 头文件

```cpp
#include "gui/desktop/main_window.h"
```

#### 命名空间

```cpp
namespace mcu::gui::desktop {
```

#### 类定义

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    // 打包模组
    void PackMod(const std::string& inputPath, const std::string& outputPath);
    
    // 注入游戏
    void InjectGame(const std::string& inputPath, const std::string& outputPath);
    
    // 转换资源
    void ConvertResource(const std::string& inputPath, const std::string& outputPath);
    
    // 加载资源包
    void LoadResourcePack(const std::string& packPath);
    
    // 添加日志
    void AddLog(const std::string& log);
    
private slots:
    void OnPackModClicked();
    void OnInjectGameClicked();
    void OnConvertResourceClicked();
    void OnLoadResourcePackClicked();
    
private:
    Ui::MainWindow* ui_;
    WorkerThread* workerThread_;
};
```

#### 使用示例

```cpp
#include "gui/desktop/main_window.h"
#include <QApplication>

using namespace mcu::gui::desktop;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // 创建主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

### InGameGUI

游戏内GUI，使用ImGui实现。

#### 头文件

```cpp
#include "gui/ingame/ingame_gui.h"
```

#### 命名空间

```cpp
namespace mcu::gui::ingame {
```

#### 类定义

```cpp
class InGameGUI {
public:
    InGameGUI();
    ~InGameGUI();
    
    // 初始化
    bool Initialize();
    
    // 关闭
    void Shutdown();
    
    // 渲染
    void Render();
    
    // 处理输入
    void HandleInput();
    
    // 切换可见性
    void ToggleVisibility();
    
    // 设置可见性
    void SetVisible(bool visible);
    
    // 获取可见性
    bool IsVisible() const;
    
private:
    bool visible_;
    InGameGUIRenderer* renderer_;
};
```

#### 使用示例

```cpp
#include "gui/ingame/ingame_gui.h"

using namespace mcu::gui::ingame;

// 创建游戏内GUI
InGameGUI gui;

// 初始化GUI
bool result = gui.Initialize();
if (result) {
    std::cout << "In-game GUI initialized successfully" << std::endl;
    
    // 渲染循环
    while (running) {
        // 处理输入
        gui.HandleInput();
        
        // 渲染GUI
        gui.Render();
    }
    
    // 关闭GUI
    gui.Shutdown();
}
```

### AndroidGUI

Android GUI，使用Android原生API实现。

#### 头文件

```cpp
#include "gui/android/android_gui.h"
```

#### 命名空间

```cpp
namespace mcu::gui::android {
```

#### 类定义

```cpp
class AndroidGUI {
public:
    AndroidGUI();
    ~AndroidGUI();
    
    // 初始化
    bool Initialize(JNIEnv* env, jobject context);
    
    // 关闭
    void Shutdown();
    
    // 获取模组列表
    std::vector<ModInfo> GetMods();
    
    // 切换模组启用状态
    bool ToggleMod(const std::string& modId);
    
    // 获取资源包列表
    std::vector<ResourcePackInfo> GetResourcePacks();
    
    // 切换资源包启用状态
    bool ToggleResourcePack(const std::string& packId);
    
    // 获取日志
    std::vector<std::string> GetLogs();
    
    // 清空日志
    void ClearLogs();
    
    // 添加日志
    void AddLog(const std::string& log);
    
private:
    JNIEnv* env_;
    jobject context_;
    std::vector<ModInfo> mods_;
    std::vector<ResourcePackInfo> resourcePacks_;
    std::vector<std::string> logs_;
};
```

#### JNI接口

```cpp
extern "C" {
    // 初始化
    JNIEXPORT jboolean JNICALL Java_com_minecraftunifier_AndroidGUI_nativeInitialize(
        JNIEnv* env, jobject thiz, jobject context);
    
    // 关闭
    JNIEXPORT void JNICALL Java_com_minecraftunifier_AndroidGUI_nativeShutdown(
        JNIEnv* env, jobject thiz);
    
    // 获取模组列表
    JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_AndroidGUI_nativeGetMods(
        JNIEnv* env, jobject thiz);
    
    // 切换模组启用状态
    JNIEXPORT jboolean JNICALL Java_com_minecraftunifier_AndroidGUI_nativeToggleMod(
        JNIEnv* env, jobject thiz, jstring modId);
    
    // 获取资源包列表
    JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_AndroidGUI_nativeGetResourcePacks(
        JNIEnv* env, jobject thiz);
    
    // 切换资源包启用状态
    JNIEXPORT jboolean JNICALL Java_com_minecraftunifier_AndroidGUI_nativeToggleResourcePack(
        JNIEnv* env, jobject thiz, jstring packId);
    
    // 获取日志
    JNIEXPORT jobjectArray JNICALL Java_com_minecraftunifier_AndroidGUI_nativeGetLogs(
        JNIEnv* env, jobject thiz);
    
    // 清空日志
    JNIEXPORT void JNICALL Java_com_minecraftunifier_AndroidGUI_nativeClearLogs(
        JNIEnv* env, jobject thiz);
    
    // 添加日志
    JNIEXPORT void JNICALL Java_com_minecraftunifier_AndroidGUI_nativeAddLog(
        JNIEnv* env, jobject thiz, jstring log);
}
```

#### 使用示例

```cpp
#include "gui/android/android_gui.h"
#include <android/log.h>

using namespace mcu::gui::android;

// 创建Android GUI
AndroidGUI gui;

// 初始化GUI
bool result = gui.Initialize(env, context);
if (result) {
    __android_log_print(ANDROID_LOG_INFO, "MinecraftUnifier", "Android GUI initialized successfully");
    
    // 获取模组列表
    std::vector<ModInfo> mods = gui.GetMods();
    __android_log_print(ANDROID_LOG_INFO, "MinecraftUnifier", "Loaded mods: %d", mods.size());
    
    // 切换模组启用状态
    result = gui.ToggleMod("testmod");
    if (result) {
        __android_log_print(ANDROID_LOG_INFO, "MinecraftUnifier", "Mod toggled successfully");
    }
    
    // 关闭GUI
    gui.Shutdown();
}
```

---

## 错误处理

所有API函数都返回布尔值表示成功或失败。如果函数返回false，可以通过以下方式获取错误信息：

```cpp
// 使用标准错误输出
std::cerr << "Error: " << error_message << std::endl;

// 使用日志系统
#include <iostream>
std::cout << "Error: " << error_message << std::endl;

// Android日志
#ifdef __ANDROID__
#include <android/log.h>
__android_log_print(ANDROID_LOG_ERROR, "MinecraftUnifier", "Error: %s", error_message.c_str());
#endif
```

---

## 线程安全

大多数API都是线程安全的，但需要注意以下几点：

1. **单例模式**：ResourceManager使用单例模式，所有线程共享同一个实例
2. **运行时**：JavaModRuntime和NeteaseModRuntime不是线程安全的，应该在同一个线程中使用
3. **GUI**：所有GUI操作必须在主线程中执行

---

## 性能优化

1. **缓存**：ShaderConverter使用缓存来避免重复转换
2. **批量操作**：使用BatchPack和BatchConvert来批量处理多个文件
3. **异步操作**：使用WorkerThread来执行耗时操作

---

## 许可证

本项目采用MIT许可证。详见LICENSE文件。

---

## 联系方式

如有问题或建议，请联系：jqyh1026@outlook.com
