/**
 * Minecraft Unifier - Java Mod Runtime Implementation
 * Java模组运行时实现 - 嵌入式JVM
 */

#include "java_runtime.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <dlfcn.h>

namespace fs = std::filesystem;

namespace mcu {
namespace core {
namespace mods {

// ==================== JavaModRuntime ====================

JavaModRuntime::JavaModRuntime()
    : jvm_(nullptr)
    , env_(nullptr)
    , initialized_(false)
    , classPath_(".")
    , modsDirectory_("./mods")
{
}

JavaModRuntime::~JavaModRuntime() {
    Shutdown();
}

bool JavaModRuntime::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // 创建JVM
    if (!CreateJVM()) {
        return false;
    }
    
    // 注册本地方法
    RegisterNativeMethods();
    
    // 加载API映射
    LoadApiMappings("./api_mappings.json");
    
    initialized_ = true;
    return true;
}

void JavaModRuntime::Shutdown() {
    if (jvm_) {
        jvm_->DestroyJavaVM();
        jvm_ = nullptr;
        env_ = nullptr;
    }
    initialized_ = false;
}

bool JavaModRuntime::CreateJVM() {
    JavaVMInitArgs args;
    JavaVMOption options[10];
    int optionCount = 0;
    
    // 设置类路径
    std::string classpath = "-Djava.class.path=" + classPath_;
    options[optionCount].optionString = const_cast<char*>(classpath.c_str());
    optionCount++;
    
    // 设置模组目录
    std::string modsPath = "-Dminecraft.mods.dir=" + modsDirectory_;
    options[optionCount].optionString = const_cast<char*>(modsPath.c_str());
    optionCount++;
    
    // 设置JVM参数
    options[optionCount].optionString = const_cast<char*>("-Xmx512M");
    optionCount++;
    
    options[optionCount].optionString = const_cast<char*>("-Xms128M");
    optionCount++;
    
    // 禁用严格模式
    options[optionCount].optionString = const_cast<char*>("-Djava.compiler=NONE");
    optionCount++;
    
    args.version = JNI_VERSION_1_8;
    args.nOptions = optionCount;
    args.options = options;
    args.ignoreUnrecognized = JNI_FALSE;
    
    // 创建JVM
    jint res = JNI_CreateJavaVM(&jvm_, reinterpret_cast<void**>(&env_), &args);
    if (res != JNI_OK) {
        return false;
    }
    
    return true;
}

bool JavaModRuntime::LoadMod(const std::string& jarPath) {
    if (!initialized_) {
        return false;
    }
    
    JavaModInfo info;
    if (!LoadModFromJar(jarPath, info)) {
        return false;
    }
    
    // 检查是否已加载
    if (loadedMods_.find(info.modId) != loadedMods_.end()) {
        return false;
    }
    
    // 检查依赖关系
    for (const auto& dep : info.dependencies) {
        if (loadedMods_.find(dep) == loadedMods_.end()) {
            // 依赖未满足，记录警告
            continue;
        }
    }
    
    // 添加到类路径
    std::string cmd = "System.getProperty(\"java.class.path\") + \":\" + \"" + jarPath + "\"";
    jstring newClasspath = env_->NewStringUTF(cmd.c_str());
    
    // 调用System.setProperty
    jclass systemClass = env_->FindClass("java/lang/System");
    jmethodID setPropertyMethod = env_->GetStaticMethodID(systemClass, "setProperty",
                                                           "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    env_->CallStaticObjectMethod(systemClass, setPropertyMethod,
                                 env_->NewStringUTF("java.class.path"),
                                 newClasspath);
    
    // 尝试加载模组主类
    std::string mainClass = info.modId;
    std::replace(mainClass.begin(), mainClass.end(), '.', '/');
    
    // 尝试常见的模组主类命名模式
    std::vector<std::string> possibleMainClasses = {
        mainClass,
        mainClass + "/ModMain",
        mainClass + "/Main",
        mainClass + "/Core",
        "com/" + info.modId + "/ModMain",
        "net/" + info.modId + "/ModMain",
        "io/github/" + info.modId + "/ModMain"
    };
    
    bool modLoaded = false;
    for (const auto& className : possibleMainClasses) {
        jclass modClass = env_->FindClass(className.c_str());
        if (modClass) {
            // 尝试调用初始化方法
            jmethodID initMethod = env_->GetStaticMethodID(modClass, "init", "()V");
            if (initMethod) {
                env_->CallStaticVoidMethod(modClass, initMethod);
                modLoaded = true;
                break;
            }
            
            // 尝试调用onLoad方法
            jmethodID loadMethod = env_->GetStaticMethodID(modClass, "onLoad", "()V");
            if (loadMethod) {
                env_->CallStaticVoidMethod(modClass, loadMethod);
                modLoaded = true;
                break;
            }
            
            // 尝试调用构造函数
            jmethodID constructor = env_->GetMethodID(modClass, "<init>", "()V");
            if (constructor) {
                jobject modInstance = env_->NewObject(modClass, constructor);
                if (modInstance) {
                    modLoaded = true;
                    env_->DeleteLocalRef(modInstance);
                    break;
                }
            }
        }
    }
    
    if (!modLoaded) {
        // 没有找到主类或初始化方法，但仍然标记为已加载
        // 某些模组可能通过事件系统初始化
    }
    
    loadedMods_[info.modId] = info;
    
    // 触发模组加载事件
    TriggerEvent("mod_loaded", &info);
    
    return true;
}

bool JavaModRuntime::UnloadMod(const std::string& modId) {
    auto it = loadedMods_.find(modId);
    if (it == loadedMods_.end()) {
        return false;
    }
    
    // 检查是否有其他模组依赖此模组
    for (const auto& [id, info] : loadedMods_) {
        if (id != modId) {
            for (const auto& dep : info.dependencies) {
                if (dep == modId) {
                    // 有其他模组依赖此模组，不能卸载
                    return false;
                }
            }
        }
    }
    
    // 触发模组卸载事件
    TriggerEvent("mod_unloaded", &it->second);
    
    // 尝试调用模组的卸载方法
    std::string mainClass = it->second.modId;
    std::replace(mainClass.begin(), mainClass.end(), '.', '/');
    
    // 尝试常见的模组主类命名模式
    std::vector<std::string> possibleMainClasses = {
        mainClass,
        mainClass + "/ModMain",
        mainClass + "/Main",
        mainClass + "/Core",
        "com/" + it->second.modId + "/ModMain",
        "net/" + it->second.modId + "/ModMain",
        "io/github/" + it->second.modId + "/ModMain"
    };
    
    for (const auto& className : possibleMainClasses) {
        jclass modClass = env_->FindClass(className.c_str());
        if (modClass) {
            // 尝试调用onUnload方法
            jmethodID unloadMethod = env_->GetStaticMethodID(modClass, "onUnload", "()V");
            if (unloadMethod) {
                env_->CallStaticVoidMethod(modClass, unloadMethod);
                break;
            }
            
            // 尝试调用disable方法
            jmethodID disableMethod = env_->GetStaticMethodID(modClass, "disable", "()V");
            if (disableMethod) {
                env_->CallStaticVoidMethod(modClass, disableMethod);
                break;
            }
        }
    }
    
    // 从类路径中移除
    // 注意：JVM不支持动态移除类路径，这里只是逻辑上的移除
    // 实际的类卸载需要依赖垃圾回收器
    
    loadedMods_.erase(it);
    return true;
}

bool JavaModRuntime::LoadModFromJar(const std::string& jarPath, JavaModInfo& info) {
    info.jarPath = jarPath;
    
    // 解析模组manifest
    if (!ParseModManifest(jarPath, info)) {
        // 使用默认值
        fs::path path(jarPath);
        info.modId = path.stem().string();
        info.name = info.modId;
        info.version = "1.0.0";
        info.description = "Unknown mod";
    }
    
    return true;
}

bool JavaModRuntime::ParseModManifest(const std::string& jarPath, JavaModInfo& info) {
    // 尝试从JAR文件中读取mod配置
    // 可能的配置文件：mcmod.info（Forge旧版）、mods.toml（Forge新版）、fabric.mod.json（Fabric）
    
    // 使用系统命令解压JAR文件中的配置
    std::string tempDir = "/tmp/mcu_mod_" + std::to_string(std::time(nullptr));
    fs::create_directories(tempDir);
    
    // 尝试提取mcmod.info（Forge旧版）
    std::string cmd = "cd " + tempDir + " && jar xf " + jarPath + " mcmod.info 2>/dev/null";
    system(cmd.c_str());
    
    if (fs::exists(tempDir + "/mcmod.info")) {
        if (ParseMcmodInfo(tempDir + "/mcmod.info", info)) {
            fs::remove_all(tempDir);
            return true;
        }
    }
    
    // 尝试提取mods.toml（Forge新版）
    cmd = "cd " + tempDir + " && jar xf " + jarPath + " META-INF/mods.toml 2>/dev/null";
    system(cmd.c_str());
    
    if (fs::exists(tempDir + "/META-INF/mods.toml")) {
        if (ParseModsToml(tempDir + "/META-INF/mods.toml", info)) {
            fs::remove_all(tempDir);
            return true;
        }
    }
    
    // 尝试提取fabric.mod.json（Fabric）
    cmd = "cd " + tempDir + " && jar xf " + jarPath + " fabric.mod.json 2>/dev/null";
    system(cmd.c_str());
    
    if (fs::exists(tempDir + "/fabric.mod.json")) {
        if (ParseFabricModJson(tempDir + "/fabric.mod.json", info)) {
            fs::remove_all(tempDir);
            return true;
        }
    }
    
    fs::remove_all(tempDir);
    
    // 如果没有找到配置文件，使用文件名作为模组ID
    fs::path path(jarPath);
    info.modId = path.stem().string();
    info.name = info.modId;
    info.version = "1.0.0";
    info.description = "Loaded from JAR (no manifest found)";
    
    return true;
}

bool JavaModRuntime::CallJavaMethod(const std::string& className,
                                   const std::string& methodName,
                                   const std::string& signature,
                                   ...) {
    if (!initialized_ || !env_) {
        return false;
    }
    
    // 查找类
    jclass clazz = env_->FindClass(className.c_str());
    if (!clazz) {
        return false;
    }
    
    // 查找方法
    jmethodID method = env_->GetMethodID(clazz, methodName.c_str(), signature.c_str());
    if (!method) {
        return false;
    }
    
    // 准备参数
    va_list args;
    va_start(args, signature);
    
    // 根据返回类型调用不同类型的方法
    char returnType = signature[signature.find(')') + 1];
    
    switch (returnType) {
        case 'V': // void
            env_->CallVoidMethodV(nullptr, method, args);
            break;
        case 'Z': // boolean
            env_->CallBooleanMethodV(nullptr, method, args);
            break;
        case 'B': // byte
            env_->CallByteMethodV(nullptr, method, args);
            break;
        case 'C': // char
            env_->CallCharMethodV(nullptr, method, args);
            break;
        case 'S': // short
            env_->CallShortMethodV(nullptr, method, args);
            break;
        case 'I': // int
            env_->CallIntMethodV(nullptr, method, args);
            break;
        case 'J': // long
            env_->CallLongMethodV(nullptr, method, args);
            break;
        case 'F': // float
            env_->CallFloatMethodV(nullptr, method, args);
            break;
        case 'D': // double
            env_->CallDoubleMethodV(nullptr, method, args);
            break;
        case 'L': // object
        case '[': // array
            env_->CallObjectMethodV(nullptr, method, args);
            break;
        default:
            va_end(args);
            return false;
    }
    
    va_end(args);
    
    // 检查异常
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        return false;
    }
    
    return true;
}

bool JavaModRuntime::CallStaticJavaMethod(const std::string& className,
                                         const std::string& methodName,
                                         const std::string& signature,
                                         ...) {
    if (!initialized_ || !env_) {
        return false;
    }
    
    // 查找类
    jclass clazz = env_->FindClass(className.c_str());
    if (!clazz) {
        return false;
    }
    
    // 查找方法
    jmethodID method = env_->GetStaticMethodID(clazz, methodName.c_str(), signature.c_str());
    if (!method) {
        return false;
    }
    
    // 准备参数
    va_list args;
    va_start(args, signature);
    
    // 根据返回类型调用不同类型的静态方法
    char returnType = signature[signature.find(')') + 1];
    
    switch (returnType) {
        case 'V': // void
            env_->CallStaticVoidMethodV(clazz, method, args);
            break;
        case 'Z': // boolean
            env_->CallStaticBooleanMethodV(clazz, method, args);
            break;
        case 'B': // byte
            env_->CallStaticByteMethodV(clazz, method, args);
            break;
        case 'C': // char
            env_->CallStaticCharMethodV(clazz, method, args);
            break;
        case 'S': // short
            env_->CallStaticShortMethodV(clazz, method, args);
            break;
        case 'I': // int
            env_->CallStaticIntMethodV(clazz, method, args);
            break;
        case 'J': // long
            env_->CallStaticLongMethodV(clazz, method, args);
            break;
        case 'F': // float
            env_->CallStaticFloatMethodV(clazz, method, args);
            break;
        case 'D': // double
            env_->CallStaticDoubleMethodV(clazz, method, args);
            break;
        case 'L': // object
        case '[': // array
            env_->CallStaticObjectMethodV(clazz, method, args);
            break;
        default:
            va_end(args);
            return false;
    }
    
    va_end(args);
    
    // 检查异常
    if (env_->ExceptionCheck()) {
        env_->ExceptionDescribe();
        env_->ExceptionClear();
        return false;
    }
    
    return true;
}

bool JavaModRuntime::RegisterNativeMethod(const std::string& className,
                                         const std::string& methodName,
                                         const std::string& signature,
                                         void* func) {
    if (!initialized_ || !env_) {
        return false;
    }
    
    // 查找类
    jclass clazz = env_->FindClass(className.c_str());
    if (!clazz) {
        return false;
    }
    
    // 注册本地方法
    JNINativeMethod method;
    method.name = const_cast<char*>(methodName.c_str());
    method.signature = const_cast<char*>(signature.c_str());
    method.fnPtr = func;
    
    jint res = env_->RegisterNatives(clazz, &method, 1);
    return res == JNI_OK;
}

void JavaModRuntime::AddApiMapping(const ApiMapping& mapping) {
    apiMappings_.push_back(mapping);
}

bool JavaModRuntime::LoadApiMappings(const std::string& configPath) {
    // TODO: 从JSON文件加载API映射配置
    return true;
}

void JavaModRuntime::RegisterNativeMethods() {
    // 注册所有桥接函数
    bridge::RegisterAllBridges(this);
}

void* JavaModRuntime::FindNativeFunction(const std::string& className, const std::string& methodName) {
    for (const auto& mapping : apiMappings_) {
        if (mapping.javaClass == className && mapping.javaMethod == methodName) {
            return mapping.nativeFunc;
        }
    }
    return nullptr;
}

std::vector<JavaModInfo> JavaModRuntime::GetLoadedMods() const {
    std::vector<JavaModInfo> mods;
    for (const auto& [id, info] : loadedMods_) {
        mods.push_back(info);
    }
    return mods;
}

const JavaModInfo* JavaModRuntime::GetModInfo(const std::string& modId) const {
    auto it = loadedMods_.find(modId);
    if (it != loadedMods_.end()) {
        return &it->second;
    }
    return nullptr;
}

void JavaModRuntime::SetEventCallback(EventCallback callback) {
    eventCallback_ = callback;
}

void JavaModRuntime::TriggerEvent(const std::string& event, void* data) {
    if (eventCallback_) {
        eventCallback_(event, data);
    }
}

bool JavaModRuntime::ParseMcmodInfo(const std::string& filePath, JavaModInfo& info) {
    // 解析Forge旧版mcmod.info格式（JSON格式）
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // 简单的JSON解析（实际项目应使用JSON库）
    // 查找modid
    size_t pos = content.find("\"modid\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 8);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.modId = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    // 查找name
    pos = content.find("\"name\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 6);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.name = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    // 查找version
    pos = content.find("\"version\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 10);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.version = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    // 查找description
    pos = content.find("\"description\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 14);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.description = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    return !info.modId.empty();
}

bool JavaModRuntime::ParseModsToml(const std::string& filePath, JavaModInfo& info) {
    // 解析Forge新版mods.toml格式（TOML格式）
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    bool inModSection = false;
    
    while (std::getline(file, line)) {
        // 检查是否进入[[mods]]部分
        if (line.find("[[mods]]") != std::string::npos) {
            inModSection = true;
            continue;
        }
        
        if (!inModSection) {
            continue;
        }
        
        // 检查是否离开mods部分
        if (line.find("[[") != std::string::npos && line.find("[[mods]]") == std::string::npos) {
            break;
        }
        
        // 解析modId
        if (line.find("modId") != std::string::npos) {
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                // 去除引号和空格
                size_t start = value.find("\"");
                if (start != std::string::npos) {
                    size_t end = value.find("\"", start + 1);
                    if (end != std::string::npos) {
                        info.modId = value.substr(start + 1, end - start - 1);
                    }
                }
            }
        }
        
        // 解析displayName
        if (line.find("displayName") != std::string::npos) {
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                size_t start = value.find("\"");
                if (start != std::string::npos) {
                    size_t end = value.find("\"", start + 1);
                    if (end != std::string::npos) {
                        info.name = value.substr(start + 1, end - start - 1);
                    }
                }
            }
        }
        
        // 解析version
        if (line.find("version") != std::string::npos) {
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                size_t start = value.find("\"");
                if (start != std::string::npos) {
                    size_t end = value.find("\"", start + 1);
                    if (end != std::string::npos) {
                        info.version = value.substr(start + 1, end - start - 1);
                    }
                }
            }
        }
        
        // 解析description
        if (line.find("description") != std::string::npos) {
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                std::string value = line.substr(pos + 1);
                size_t start = value.find("\"");
                if (start != std::string::npos) {
                    size_t end = value.find("\"", start + 1);
                    if (end != std::string::npos) {
                        info.description = value.substr(start + 1, end - start - 1);
                    }
                }
            }
        }
    }
    
    file.close();
    return !info.modId.empty();
}

bool JavaModRuntime::ParseFabricModJson(const std::string& filePath, JavaModInfo& info) {
    // 解析Fabric fabric.mod.json格式（JSON格式）
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // 查找id
    size_t pos = content.find("\"id\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 4);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.modId = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    // 查找name
    pos = content.find("\"name\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 6);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.name = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    // 查找version
    pos = content.find("\"version\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 10);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.version = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    // 查找description
    pos = content.find("\"description\"");
    if (pos != std::string::npos) {
        pos = content.find("\"", pos + 14);
        if (pos != std::string::npos) {
            size_t end = content.find("\"", pos + 1);
            if (end != std::string::npos) {
                info.description = content.substr(pos + 1, end - pos - 1);
            }
        }
    }
    
    return !info.modId.empty();
}

// ==================== BedrockAPI ====================

BedrockAPI::BedrockAPI() {
}

BedrockAPI::~BedrockAPI() {
}

BedrockAPI& BedrockAPI::GetInstance() {
    static BedrockAPI instance;
    return instance;
}

void BedrockAPI::SetBlock(int x, int y, int z, int blockId) {
    // TODO: 调用基岩版内部函数设置方块
    // 需要通过逆向分析找到相关函数地址
}

int BedrockAPI::GetBlock(int x, int y, int z) {
    // TODO: 调用基岩版内部函数获取方块
    return 0;
}

void BedrockAPI::GetPlayerPosition(const char* playerId, float* x, float* y, float* z) {
    // TODO: 调用基岩版内部函数获取玩家位置
    *x = 0.0f;
    *y = 0.0f;
    *z = 0.0f;
}

void BedrockAPI::SetPlayerPosition(const char* playerId, float x, float y, float z) {
    // TODO: 调用基岩版内部函数设置玩家位置
}

void BedrockAPI::SpawnEntity(const char* entityType, float x, float y, float z) {
    // TODO: 调用基岩版内部函数生成实体
}

void BedrockAPI::RemoveEntity(int entityId) {
    // TODO: 调用基岩版内部函数移除实体
}

void BedrockAPI::GetTime(int* dayTime) {
    // TODO: 调用基岩版内部函数获取时间
    *dayTime = 0;
}

void BedrockAPI::SetTime(int dayTime) {
    // TODO: 调用基岩版内部函数设置时间
}

void BedrockAPI::GetGameVersion(char* version, int maxLen) {
    // TODO: 调用基岩版内部函数获取游戏版本
    strncpy(version, "1.19.0", maxLen);
}

// ==================== bridge namespace ====================

namespace bridge {

void Java_SetBlock(JNIEnv* env, jclass clazz, jint x, jint y, jint z, jint blockId) {
    BedrockAPI::SetBlock(x, y, z, blockId);
}

jint Java_GetBlock(JNIEnv* env, jclass clazz, jint x, jint y, jint z) {
    return BedrockAPI::GetBlock(x, y, z);
}

void Java_GetPlayerPosition(JNIEnv* env, jclass clazz, jstring playerId, jfloatArray pos) {
    const char* playerIdStr = env->GetStringUTFChars(playerId, nullptr);
    
    float x, y, z;
    BedrockAPI::GetPlayerPosition(playerIdStr, &x, &y, &z);
    
    jfloat* posArray = env->GetFloatArrayElements(pos, nullptr);
    posArray[0] = x;
    posArray[1] = y;
    posArray[2] = z;
    env->ReleaseFloatArrayElements(pos, posArray, 0);
    
    env->ReleaseStringUTFChars(playerId, playerIdStr);
}

void Java_SetPlayerPosition(JNIEnv* env, jclass clazz, jstring playerId,
                           jfloat x, jfloat y, jfloat z) {
    const char* playerIdStr = env->GetStringUTFChars(playerId, nullptr);
    BedrockAPI::SetPlayerPosition(playerIdStr, x, y, z);
    env->ReleaseStringUTFChars(playerId, playerIdStr);
}

void Java_SpawnEntity(JNIEnv* env, jclass clazz, jstring entityType,
                     jfloat x, jfloat y, jfloat z) {
    const char* entityTypeStr = env->GetStringUTFChars(entityType, nullptr);
    BedrockAPI::SpawnEntity(entityTypeStr, x, y, z);
    env->ReleaseStringUTFChars(entityType, entityTypeStr);
}

void Java_RemoveEntity(JNIEnv* env, jclass clazz, jint entityId) {
    BedrockAPI::RemoveEntity(entityId);
}

void Java_GetTime(JNIEnv* env, jclass clazz, jintArray time) {
    jint dayTime;
    BedrockAPI::GetTime(&dayTime);
    
    jint* timeArray = env->GetIntArrayElements(time, nullptr);
    timeArray[0] = dayTime;
    env->ReleaseIntArrayElements(time, timeArray, 0);
}

void Java_SetTime(JNIEnv* env, jclass clazz, jint time) {
    BedrockAPI::SetTime(time);
}

void RegisterAllBridges(JavaModRuntime* runtime) {
    // 注册方块相关方法
    runtime->RegisterNativeMethod("net/minecraft/world/World", "setBlock", "(IIII)V",
                                  reinterpret_cast<void*>(Java_SetBlock));
    runtime->RegisterNativeMethod("net/minecraft/world/World", "getBlock", "(III)I",
                                  reinterpret_cast<void*>(Java_GetBlock));
    
    // 注册玩家相关方法
    runtime->RegisterNativeMethod("net/minecraft/entity/player/PlayerEntity", "getPosition",
                                  "([F)V", reinterpret_cast<void*>(Java_GetPlayerPosition));
    runtime->RegisterNativeMethod("net/minecraft/entity/player/PlayerEntity", "setPosition",
                                  "(FFF)V", reinterpret_cast<void*>(Java_SetPlayerPosition));
    
    // 注册实体相关方法
    runtime->RegisterNativeMethod("net/minecraft/world/World", "spawnEntity",
                                  "(Ljava/lang/String;FFF)V", reinterpret_cast<void*>(Java_SpawnEntity));
    runtime->RegisterNativeMethod("net/minecraft/world/World", "removeEntity",
                                  "(I)V", reinterpret_cast<void*>(Java_RemoveEntity));
    
    // 注册世界相关方法
    runtime->RegisterNativeMethod("net/minecraft/world/World", "getWorldTime",
                                  "([I)V", reinterpret_cast<void*>(Java_GetTime));
    runtime->RegisterNativeMethod("net/minecraft/world/World", "setWorldTime",
                                  "(I)V", reinterpret_cast<void*>(Java_SetTime));
}

} // namespace bridge

} // namespace mods
} // namespace core
} // namespace mcu
