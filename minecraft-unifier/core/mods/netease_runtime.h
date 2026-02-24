/**
 * Minecraft Unifier - Netease Mod Runtime
 * 网易模组运行时 - 嵌入式Python
 */

#pragma once
#include <Python.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace mcu {
namespace core {
namespace mods {

// 网易模组信息
struct NeteaseModInfo {
    std::string modId;          // 模组ID
    std::string name;           // 模组名称
    std::string version;        // 版本
    std::string description;    // 描述
    std::string modPath;        // 模组路径
    std::vector<std::string> dependencies; // 依赖项
};

// 网易模组运行时
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
    
    // 调用Python函数（带返回值）
    PyObject* CallPythonFunctionWithReturn(const std::string& module,
                                          const std::string& function,
                                          PyObject* args);
    
    // 注册C函数到Python
    bool RegisterCFunction(const std::string& moduleName,
                          const std::string& functionName,
                          PyCFunction func,
                          const char* doc);
    
    // 获取已加载的模组列表
    std::vector<NeteaseModInfo> GetLoadedMods() const;
    
    // 获取模组信息
    const NeteaseModInfo* GetModInfo(const std::string& modId) const;
    
    // 事件回调
    using EventCallback = std::function<void(const std::string& event, void* data)>;
    void SetEventCallback(EventCallback callback);
    
    // 触发事件
    void TriggerEvent(const std::string& event, void* data);

private:
    bool initialized_;
    std::unordered_map<std::string, NeteaseModInfo> loadedMods_;
    EventCallback eventCallback_;
    
    std::string modsDirectory_;
    
    // 内部处理函数
    bool InitPythonInterpreter();
    bool LoadModFromPath(const std::string& modPath, NeteaseModInfo& info);
    bool ParseModManifest(const std::string& modPath, NeteaseModInfo& info);
    void RegisterNeteaseSDKSimulation();
};

// 网易SDK模拟层
class NeteaseSDKSimulation {
public:
    static NeteaseSDKSimulation& GetInstance();
    
    // 初始化SDK模拟
    bool Initialize();
    
    // 注册所有SDK模块
    void RegisterAllModules();

private:
    NeteaseSDKSimulation();
    ~NeteaseSDKSimulation();
    
    bool initialized_;
};

// Python到基岩版的桥接函数
namespace bridge {
    // 客户端API
    PyObject* client_GetPlayerPos(PyObject* self, PyObject* args);
    PyObject* client_SetPlayerPos(PyObject* self, PyObject* args);
    PyObject* client_GetBlock(PyObject* self, PyObject* args);
    PyObject* client_SetBlock(PyObject* self, PyObject* args);
    PyObject* client_SpawnEntity(PyObject* self, PyObject* args);
    PyObject* client_RemoveEntity(PyObject* self, PyObject* args);
    
    // 服务器API
    PyObject* server_BroadcastMessage(PyObject* self, PyObject* args);
    PyObject* server_GetPlayerList(PyObject* self, PyObject* args);
    PyObject* server_GetLevelName(PyObject* self, PyObject* args);
    PyObject* server_SetTime(PyObject* self, PyObject* args);
    PyObject* server_GetTime(PyObject* self, PyObject* args);
    
    // 注册所有桥接函数
    void RegisterAllBridges(NeteaseModRuntime* runtime);
}

} // namespace mods
} // namespace core
} // namespace mcu
