/**
 * Minecraft Unifier - Netease Mod Runtime Implementation
 * 网易模组运行时实现 - 嵌入式Python
 */

#include "netease_runtime.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace mcu {
namespace core {
namespace mods {

// ==================== NeteaseModRuntime ====================

NeteaseModRuntime::NeteaseModRuntime()
    : initialized_(false)
    , modsDirectory_("./mods")
{
}

NeteaseModRuntime::~NeteaseModRuntime() {
    Shutdown();
}

bool NeteaseModRuntime::Initialize() {
    if (initialized_) {
        return true;
    }
    
    // 初始化Python解释器
    if (!InitPythonInterpreter()) {
        return false;
    }
    
    // 注册网易SDK模拟层
    RegisterNeteaseSDKSimulation();
    
    initialized_ = true;
    return true;
}

void NeteaseModRuntime::Shutdown() {
    if (initialized_) {
        Py_Finalize();
        initialized_ = false;
    }
}

bool NeteaseModRuntime::InitPythonInterpreter() {
    // 初始化Python解释器
    Py_Initialize();
    
    if (!Py_IsInitialized()) {
        return false;
    }
    
    // 设置模块搜索路径
    PyRun_SimpleString("import sys");
    std::string cmd = "sys.path.append('" + modsDirectory_ + "')";
    PyRun_SimpleString(cmd.c_str());
    
    return true;
}

bool NeteaseModRuntime::LoadMod(const std::string& modPath) {
    if (!initialized_) {
        return false;
    }
    
    NeteaseModInfo info;
    if (!LoadModFromPath(modPath, info)) {
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
            std::string warnMsg = "Warning: Dependency '" + dep + "' not satisfied for mod '" + info.modId + "'";
            PyRun_SimpleString(("print('" + warnMsg + "')").c_str());
            continue;
        }
    }
    
    // 添加模组路径到Python搜索路径
    std::string pathCmd = "sys.path.insert(0, '" + modPath + "')";
    PyRun_SimpleString(pathCmd.c_str());
    
    // 导入Python模块
    std::string importCmd = "import " + info.modId;
    if (PyRun_SimpleString(importCmd.c_str()) != 0) {
        PyErr_Print();
        return false;
    }
    
    // 尝试调用模组的初始化函数
    std::vector<std::string> initFunctions = {"init", "on_load", "onLoad", "initialize"};
    bool initCalled = false;
    
    for (const auto& func : initFunctions) {
        std::string initCmd = info.modId + "." + func + "()";
        if (PyRun_SimpleString(initCmd.c_str()) == 0) {
            initCalled = true;
            break;
        }
    }
    
    if (!initCalled) {
        // 没有找到初始化函数，但仍然标记为已加载
        // 某些模组可能通过事件系统初始化
    }
    
    loadedMods_[info.modId] = info;
    
    // 触发模组加载事件
    TriggerEvent("mod_loaded", &info);
    
    return true;
}

bool NeteaseModRuntime::UnloadMod(const std::string& modId) {
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
                    std::string errorMsg = "Error: Cannot unload mod '" + modId + "', it is required by mod '" + id + "'";
                    PyRun_SimpleString(("print('" + errorMsg + "')").c_str());
                    return false;
                }
            }
        }
    }
    
    // 触发模组卸载事件
    TriggerEvent("mod_unloaded", &it->second);
    
    // 尝试调用模组的卸载函数
    std::vector<std::string> unloadFunctions = {"cleanup", "on_unload", "onUnload", "disable", "deinitialize"};
    bool unloadCalled = false;
    
    for (const auto& func : unloadFunctions) {
        std::string unloadCmd = modId + "." + func + "()";
        if (PyRun_SimpleString(unloadCmd.c_str()) == 0) {
            unloadCalled = true;
            break;
        }
    }
    
    // 从Python中移除模块
    std::string removeCmd = "if '" + modId + "' in sys.modules: del sys.modules['" + modId + "']";
    PyRun_SimpleString(removeCmd.c_str());
    
    // 清理模块的所有子模块
    std::string cleanupCmd = "import sys\n";
    cleanupCmd += "for key in list(sys.modules.keys()):\n";
    cleanupCmd += "    if key.startswith('" + modId + ".'):\n";
    cleanupCmd += "        del sys.modules[key]\n";
    PyRun_SimpleString(cleanupCmd.c_str());
    
    loadedMods_.erase(it);
    return true;
}

bool NeteaseModRuntime::LoadModFromPath(const std::string& modPath, NeteaseModInfo& info) {
    info.modPath = modPath;
    
    // 解析模组manifest
    if (!ParseModManifest(modPath, info)) {
        // 使用默认值
        fs::path path(modPath);
        info.modId = path.stem().string();
        info.name = info.modId;
        info.version = "1.0.0";
        info.description = "Unknown mod";
    }
    
    return true;
}

bool NeteaseModRuntime::ParseModManifest(const std::string& modPath, NeteaseModInfo& info) {
    // 查找mod.json（网易标准格式）
    std::string manifestPath = modPath + "/mod.json";
    if (fs::exists(manifestPath)) {
        return ParseModJson(manifestPath, info);
    }
    
    // 查找mod.py（Python脚本格式）
    manifestPath = modPath + "/mod.py";
    if (fs::exists(manifestPath)) {
        return ParseModPy(manifestPath, info);
    }
    
    // 查找__init__.py（Python包格式）
    manifestPath = modPath + "/__init__.py";
    if (fs::exists(manifestPath)) {
        return ParseModPy(manifestPath, info);
    }
    
    return false;
}

bool NeteaseModRuntime::CallPythonFunction(const std::string& module,
                                          const std::string& function,
                                          PyObject* args) {
    if (!initialized_) {
        return false;
    }
    
    // 获取模块
    PyObject* moduleName = PyUnicode_FromString(module.c_str());
    PyObject* moduleObj = PyImport_Import(moduleName);
    Py_DECREF(moduleName);
    
    if (!moduleObj) {
        return false;
    }
    
    // 获取函数
    PyObject* funcObj = PyObject_GetAttrString(moduleObj, function.c_str());
    Py_DECREF(moduleObj);
    
    if (!funcObj || !PyCallable_Check(funcObj)) {
        Py_XDECREF(funcObj);
        return false;
    }
    
    // 调用函数
    PyObject* result = PyObject_CallObject(funcObj, args);
    Py_DECREF(funcObj);
    
    if (!result) {
        PyErr_Print();
        return false;
    }
    
    Py_DECREF(result);
    return true;
}

PyObject* NeteaseModRuntime::CallPythonFunctionWithReturn(const std::string& module,
                                                         const std::string& function,
                                                         PyObject* args) {
    if (!initialized_) {
        return nullptr;
    }
    
    // 获取模块
    PyObject* moduleName = PyUnicode_FromString(module.c_str());
    PyObject* moduleObj = PyImport_Import(moduleName);
    Py_DECREF(moduleName);
    
    if (!moduleObj) {
        return nullptr;
    }
    
    // 获取函数
    PyObject* funcObj = PyObject_GetAttrString(moduleObj, function.c_str());
    Py_DECREF(moduleObj);
    
    if (!funcObj || !PyCallable_Check(funcObj)) {
        Py_XDECREF(funcObj);
        return nullptr;
    }
    
    // 调用函数
    PyObject* result = PyObject_CallObject(funcObj, args);
    Py_DECREF(funcObj);
    
    if (!result) {
        PyErr_Print();
        return nullptr;
    }
    
    return result;
}

bool NeteaseModRuntime::RegisterCFunction(const std::string& moduleName,
                                         const std::string& functionName,
                                         PyCFunction func,
                                         const char* doc) {
    if (!initialized_) {
        return false;
    }
    
    // 创建方法定义
    static std::vector<PyMethodDef> methodDefs;
    PyMethodDef methodDef;
    methodDef.ml_name = const_cast<char*>(functionName.c_str());
    methodDef.ml_meth = func;
    methodDef.ml_flags = METH_VARARGS;
    methodDef.ml_doc = const_cast<char*>(doc);
    methodDefs.push_back(methodDef);
    
    // 添加结束标记
    PyMethodDef endDef = {nullptr, nullptr, 0, nullptr};
    methodDefs.push_back(endDef);
    
    // 创建模块
    PyImport_AppendInittab(const_cast<char*>(moduleName.c_str()), 
                           PyInit_def);
    
    return true;
}

std::vector<NeteaseModInfo> NeteaseModRuntime::GetLoadedMods() const {
    std::vector<NeteaseModInfo> mods;
    for (const auto& [id, info] : loadedMods_) {
        mods.push_back(info);
    }
    return mods;
}

const NeteaseModInfo* NeteaseModRuntime::GetModInfo(const std::string& modId) const {
    auto it = loadedMods_.find(modId);
    if (it != loadedMods_.end()) {
        return &it->second;
    }
    return nullptr;
}

void NeteaseModRuntime::SetEventCallback(EventCallback callback) {
    eventCallback_ = callback;
}

void NeteaseModRuntime::TriggerEvent(const std::string& event, void* data) {
    if (eventCallback_) {
        eventCallback_(event, data);
    }
}

void NeteaseModRuntime::RegisterNeteaseSDKSimulation() {
    // 注册所有桥接函数
    bridge::RegisterAllBridges(this);
}

bool NeteaseModRuntime::ParseModJson(const std::string& filePath, NeteaseModInfo& info) {
    // 解析网易模组的mod.json格式
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // 简单的JSON解析（实际项目应使用JSON库）
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
    
    // 查找dependencies
    pos = content.find("\"dependencies\"");
    if (pos != std::string::npos) {
        pos = content.find("[", pos);
        if (pos != std::string::npos) {
            size_t end = content.find("]", pos);
            if (end != std::string::npos) {
                std::string deps = content.substr(pos + 1, end - pos - 1);
                // 解析依赖列表
                size_t start = 0;
                while (true) {
                    size_t quoteStart = deps.find("\"", start);
                    if (quoteStart == std::string::npos) break;
                    
                    size_t quoteEnd = deps.find("\"", quoteStart + 1);
                    if (quoteEnd == std::string::npos) break;
                    
                    std::string dep = deps.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    info.dependencies.push_back(dep);
                    
                    start = quoteEnd + 1;
                }
            }
        }
    }
    
    return !info.modId.empty();
}

bool NeteaseModRuntime::ParseModPy(const std::string& filePath, NeteaseModInfo& info) {
    // 解析Python脚本中的模组信息
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // 查找MOD_ID
        if (line.find("MOD_ID") != std::string::npos && line.find("=") != std::string::npos) {
            size_t pos = line.find("=");
            std::string value = line.substr(pos + 1);
            // 去除引号和空格
            size_t start = value.find("\"");
            if (start != std::string::npos) {
                size_t end = value.find("\"", start + 1);
                if (end != std::string::npos) {
                    info.modId = value.substr(start + 1, end - start - 1);
                }
            } else {
                // 去除空格
                start = value.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    size_t end = value.find_last_not_of(" \t\n\r");
                    if (end != std::string::npos) {
                        info.modId = value.substr(start, end - start + 1);
                    }
                }
            }
        }
        
        // 查找MOD_NAME
        if (line.find("MOD_NAME") != std::string::npos && line.find("=") != std::string::npos) {
            size_t pos = line.find("=");
            std::string value = line.substr(pos + 1);
            size_t start = value.find("\"");
            if (start != std::string::npos) {
                size_t end = value.find("\"", start + 1);
                if (end != std::string::npos) {
                    info.name = value.substr(start + 1, end - start - 1);
                }
            }
        }
        
        // 查找MOD_VERSION
        if (line.find("MOD_VERSION") != std::string::npos && line.find("=") != std::string::npos) {
            size_t pos = line.find("=");
            std::string value = line.substr(pos + 1);
            size_t start = value.find("\"");
            if (start != std::string::npos) {
                size_t end = value.find("\"", start + 1);
                if (end != std::string::npos) {
                    info.version = value.substr(start + 1, end - start - 1);
                }
            }
        }
        
        // 查找MOD_DESCRIPTION
        if (line.find("MOD_DESCRIPTION") != std::string::npos && line.find("=") != std::string::npos) {
            size_t pos = line.find("=");
            std::string value = line.substr(pos + 1);
            size_t start = value.find("\"");
            if (start != std::string::npos) {
                size_t end = value.find("\"", start + 1);
                if (end != std::string::npos) {
                    info.description = value.substr(start + 1, end - start - 1);
                }
            }
        }
        
        // 查找DEPENDENCIES
        if (line.find("DEPENDENCIES") != std::string::npos && line.find("=") != std::string::npos) {
            size_t pos = line.find("=");
            std::string value = line.substr(pos + 1);
            size_t start = value.find("[");
            if (start != std::string::npos) {
                size_t end = value.find("]");
                if (end != std::string::npos) {
                    std::string deps = value.substr(start + 1, end - start - 1);
                    // 解析依赖列表
                    size_t depStart = 0;
                    while (true) {
                        size_t quoteStart = deps.find("\"", depStart);
                        if (quoteStart == std::string::npos) break;
                        
                        size_t quoteEnd = deps.find("\"", quoteStart + 1);
                        if (quoteEnd == std::string::npos) break;
                        
                        std::string dep = deps.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                        info.dependencies.push_back(dep);
                        
                        depStart = quoteEnd + 1;
                    }
                }
            }
        }
    }
    
    file.close();
    
    // 如果没有找到MOD_ID，使用文件名
    if (info.modId.empty()) {
        fs::path path(filePath);
        info.modId = path.stem().string();
    }
    
    // 如果没有找到MOD_NAME，使用MOD_ID
    if (info.name.empty()) {
        info.name = info.modId;
    }
    
    // 如果没有找到MOD_VERSION，使用默认值
    if (info.version.empty()) {
        info.version = "1.0.0";
    }
    
    // 如果没有找到MOD_DESCRIPTION，使用默认值
    if (info.description.empty()) {
        info.description = "Loaded from Python script";
    }
    
    return true;
}

// ==================== NeteaseSDKSimulation ====================

NeteaseSDKSimulation::NeteaseSDKSimulation()
    : initialized_(false) {
}

NeteaseSDKSimulation::~NeteaseSDKSimulation() {
}

NeteaseSDKSimulation& NeteaseSDKSimulation::GetInstance() {
    static NeteaseSDKSimulation instance;
    return instance;
}

bool NeteaseSDKSimulation::Initialize() {
    if (initialized_) {
        return true;
    }
    
    RegisterAllModules();
    initialized_ = true;
    return true;
}

void NeteaseSDKSimulation::RegisterAllModules() {
    // 注册客户端API模块
    static PyMethodDef clientMethods[] = {
        {"GetPlayerPos", bridge::client_GetPlayerPos, METH_VARARGS,
         "获取玩家位置"},
        {"SetPlayerPos", bridge::client_SetPlayerPos, METH_VARARGS,
         "设置玩家位置"},
        {"GetBlock", bridge::client_GetBlock, METH_VARARGS,
         "获取方块"},
        {"SetBlock", bridge::client_SetBlock, METH_VARARGS,
         "设置方块"},
        {"SpawnEntity", bridge::client_SpawnEntity, METH_VARARGS,
         "生成实体"},
        {"RemoveEntity", bridge::client_RemoveEntity, METH_VARARGS,
         "移除实体"},
        {nullptr, nullptr, 0, nullptr}
    };
    
    static PyModuleDef clientModule = {
        PyModuleDef_HEAD_INIT,
        "client.extraClientApi",
        "网易客户端API模拟",
        -1,
        clientMethods
    };
    
    PyObject* clientModuleObj = PyModule_Create(&clientModule);
    PyImport_AddModule("client.extraClientApi");
    PyDict_SetItemString(PyImport_GetModuleDict(), "client.extraClientApi", clientModuleObj);
    
    // 注册服务器API模块
    static PyMethodDef serverMethods[] = {
        {"BroadcastMessage", bridge::server_BroadcastMessage, METH_VARARGS,
         "广播消息"},
        {"GetPlayerList", bridge::server_GetPlayerList, METH_VARARGS,
         "获取玩家列表"},
        {"GetLevelName", bridge::server_GetLevelName, METH_VARARGS,
         "获取世界名称"},
        {"SetTime", bridge::server_SetTime, METH_VARARGS,
         "设置时间"},
        {"GetTime", bridge::server_GetTime, METH_VARARGS,
         "获取时间"},
        {nullptr, nullptr, 0, nullptr}
    };
    
    static PyModuleDef serverModule = {
        PyModuleDef_HEAD_INIT,
        "server.extraServerApi",
        "网易服务器API模拟",
        -1,
        serverMethods
    };
    
    PyObject* serverModuleObj = PyModule_Create(&serverModule);
    PyImport_AddModule("server.extraServerApi");
    PyDict_SetItemString(PyImport_GetModuleDict(), "server.extraServerApi", serverModuleObj);
    
    // 导入到主命名空间
    PyRun_SimpleString(
        "import client.extraClientApi\n"
        "import server.extraServerApi\n"
    );
}

// ==================== bridge namespace ====================

namespace bridge {

PyObject* client_GetPlayerPos(PyObject* self, PyObject* args) {
    const char* playerId;
    if (!PyArg_ParseTuple(args, "s", &playerId)) {
        return nullptr;
    }
    
    float x, y, z;
    // TODO: 调用基岩版内部函数获取玩家位置
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    
    return Py_BuildValue("(fff)", x, y, z);
}

PyObject* client_SetPlayerPos(PyObject* self, PyObject* args) {
    const char* playerId;
    float x, y, z;
    if (!PyArg_ParseTuple(args, "sfff", &playerId, &x, &y, &z)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数设置玩家位置
    
    Py_RETURN_NONE;
}

PyObject* client_GetBlock(PyObject* self, PyObject* args) {
    int x, y, z;
    if (!PyArg_ParseTuple(args, "iii", &x, &y, &z)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数获取方块
    int blockId = 0;
    
    return Py_BuildValue("i", blockId);
}

PyObject* client_SetBlock(PyObject* self, PyObject* args) {
    int x, y, z, blockId;
    if (!PyArg_ParseTuple(args, "iiii", &x, &y, &z, &blockId)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数设置方块
    
    Py_RETURN_NONE;
}

PyObject* client_SpawnEntity(PyObject* self, PyObject* args) {
    const char* entityType;
    float x, y, z;
    if (!PyArg_ParseTuple(args, "sfff", &entityType, &x, &y, &z)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数生成实体
    
    Py_RETURN_NONE;
}

PyObject* client_RemoveEntity(PyObject* self, PyObject* args) {
    int entityId;
    if (!PyArg_ParseTuple(args, "i", &entityId)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数移除实体
    
    Py_RETURN_NONE;
}

PyObject* server_BroadcastMessage(PyObject* self, PyObject* args) {
    const char* message;
    if (!PyArg_ParseTuple(args, "s", &message)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数广播消息
    
    Py_RETURN_NONE;
}

PyObject* server_GetPlayerList(PyObject* self, PyObject* args) {
    // TODO: 调用基岩版内部函数获取玩家列表
    PyObject* playerList = PyList_New(0);
    return playerList;
}

PyObject* server_GetLevelName(PyObject* self, PyObject* args) {
    // TODO: 调用基岩版内部函数获取世界名称
    return Py_BuildValue("s", "World");
}

PyObject* server_SetTime(PyObject* self, PyObject* args) {
    int time;
    if (!PyArg_ParseTuple(args, "i", &time)) {
        return nullptr;
    }
    
    // TODO: 调用基岩版内部函数设置时间
    
    Py_RETURN_NONE;
}

PyObject* server_GetTime(PyObject* self, PyObject* args) {
    // TODO: 调用基岩版内部函数获取时间
    int time = 0;
    return Py_BuildValue("i", time);
}

void RegisterAllBridges(NeteaseModRuntime* runtime) {
    // SDK模拟层在NeteaseSDKSimulation::RegisterAllModules中注册
    NeteaseSDKSimulation::GetInstance().Initialize();
}

} // namespace bridge

} // namespace mods
} // namespace core
} // namespace mcu
