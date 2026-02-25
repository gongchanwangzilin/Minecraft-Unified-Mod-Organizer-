/**
 * Minecraft Unifier - Java Mod Runtime
 * Java模组运行时 - 嵌入式JVM
 */

#pragma once
#include <jni.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace mcu {
namespace core {
namespace mods {

// API映射条目
struct ApiMapping {
    std::string javaClass;      // Java类名
    std::string javaMethod;     // Java方法名
    std::string javaSignature;  // Java方法签名
    void* nativeFunc;           // 原生函数指针
    std::string description;    // 描述
};

// Java模组信息
struct JavaModInfo {
    std::string modId;          // 模组ID
    std::string name;           // 模组名称
    std::string version;        // 版本
    std::string description;    // 描述
    std::string jarPath;        // JAR文件路径
    std::vector<std::string> dependencies; // 依赖项
};

// Java模组运行时
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
                       const std::string& signature,
                       ...);
    
    // 调用静态Java方法
    bool CallStaticJavaMethod(const std::string& className,
                             const std::string& methodName,
                             const std::string& signature,
                             ...);
    
    // 注册本地方法
    bool RegisterNativeMethod(const std::string& className,
                             const std::string& methodName,
                             const std::string& signature,
                             void* func);
    
    // 添加API映射
    void AddApiMapping(const ApiMapping& mapping);
    
    // 加载API映射配置
    bool LoadApiMappings(const std::string& configPath);
    
    // 获取已加载的模组列表
    std::vector<JavaModInfo> GetLoadedMods() const;
    
    // 获取模组信息
    const JavaModInfo* GetModInfo(const std::string& modId) const;
    
    // 事件回调
    using EventCallback = std::function<void(const std::string& event, void* data)>;
    void SetEventCallback(EventCallback callback);
    
    // 触发事件
    void TriggerEvent(const std::string& event, void* data);

private:
    JavaVM* jvm_;
    JNIEnv* env_;
    bool initialized_;
    
    std::unordered_map<std::string, JavaModInfo> loadedMods_;
    std::vector<ApiMapping> apiMappings_;
    EventCallback eventCallback_;
    
    std::string classPath_;
    std::string modsDirectory_;
    
    // 内部处理函数
    bool CreateJVM();
    bool LoadModFromJar(const std::string& jarPath, JavaModInfo& info);
    bool ParseModManifest(const std::string& jarPath, JavaModInfo& info);
    bool ParseMcmodInfo(const std::string& filePath, JavaModInfo& info);
    bool ParseModsToml(const std::string& filePath, JavaModInfo& info);
    bool ParseFabricModJson(const std::string& filePath, JavaModInfo& info);
    void RegisterNativeMethods();
    void* FindNativeFunction(const std::string& className, const std::string& methodName);
};

// 基岩版API封装（供Java模组调用）
class BedrockAPI {
public:
    static BedrockAPI& GetInstance();
    
    // 方块操作
    static void SetBlock(int x, int y, int z, int blockId);
    static int GetBlock(int x, int y, int z);
    
    // 玩家操作
    static void GetPlayerPosition(const char* playerId, float* x, float* y, float* z);
    static void SetPlayerPosition(const char* playerId, float x, float y, float z);
    
    // 实体操作
    static void SpawnEntity(const char* entityType, float x, float y, float z);
    static void RemoveEntity(int entityId);
    
    // 世界操作
    static void GetTime(int* dayTime);
    static void SetTime(int dayTime);
    
    // 游戏信息
    static void GetGameVersion(char* version, int maxLen);

private:
    BedrockAPI();
    ~BedrockAPI();
};

// Java到基岩版的桥接函数
namespace bridge {
    // 方块相关
    void Java_SetBlock(JNIEnv* env, jclass clazz, jint x, jint y, jint z, jint blockId);
    jint Java_GetBlock(JNIEnv* env, jclass clazz, jint x, jint y, jint z);
    
    // 玩家相关
    void Java_GetPlayerPosition(JNIEnv* env, jclass clazz, jstring playerId, 
                                jfloatArray pos);
    void Java_SetPlayerPosition(JNIEnv* env, jclass clazz, jstring playerId,
                                jfloat x, jfloat y, jfloat z);
    
    // 实体相关
    void Java_SpawnEntity(JNIEnv* env, jclass clazz, jstring entityType,
                         jfloat x, jfloat y, jfloat z);
    void Java_RemoveEntity(JNIEnv* env, jclass clazz, jint entityId);
    
    // 世界相关
    void Java_GetTime(JNIEnv* env, jclass clazz, jintArray time);
    void Java_SetTime(JNIEnv* env, jclass clazz, jint time);
    
    // 注册所有桥接函数
    void RegisterAllBridges(JavaModRuntime* runtime);
}

} // namespace mods
} // namespace core
} // namespace mcu
