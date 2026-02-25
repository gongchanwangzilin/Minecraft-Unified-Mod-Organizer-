/**
 * Minecraft Unifier - In-Game GUI
 * 游戏内GUI - ImGui覆盖界面
 */

#pragma once
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <functional>

namespace mcu {
namespace gui {
namespace ingame {

// 模组信息
struct ModInfo {
    std::string id;
    std::string name;
    std::string version;
    bool enabled;
    bool loaded;
};

// 资源包信息
struct ResourcePackInfo {
    std::string id;
    std::string name;
    bool enabled;
    bool applied;
};

// 游戏内GUI管理器
class InGameGUI {
public:
    InGameGUI();
    ~InGameGUI();
    
    // 初始化
    bool Initialize(GLFWwindow* window);
    
    // 关闭
    void Shutdown();
    
    // 渲染
    void Render();
    
    // 切换可见性
    void ToggleVisibility();
    
    // 设置可见性
    void SetVisible(bool visible);
    
    // 获取可见性
    bool IsVisible() const;
    
    // 事件回调
    using ToggleCallback = std::function<void()>;
    void SetToggleCallback(ToggleCallback callback);
    
    // 模组管理
    void AddMod(const ModInfo& mod);
    void RemoveMod(const std::string& modId);
    void EnableMod(const std::string& modId);
    void DisableMod(const std::string& modId);
    const std::vector<ModInfo>& GetMods() const;
    
    // 资源包管理
    void AddResourcePack(const ResourcePackInfo& pack);
    void RemoveResourcePack(const std::string& packId);
    void EnableResourcePack(const std::string& packId);
    void DisableResourcePack(const std::string& packId);
    const std::vector<ResourcePackInfo>& GetResourcePacks() const;
    
    // 日志
    void AddLog(const std::string& message);
    void ClearLog();
    
private:
    void SetupStyle();
    void RenderMainMenu();
    void RenderModsTab();
    void RenderResourcePacksTab();
    void RenderSettingsTab();
    void RenderLogTab();
    void RenderAboutTab();
    
    GLFWwindow* window_;
    bool visible_;
    bool initialized_;
    
    // 当前选中的Tab
    int currentTab_;
    
    // 模组列表
    std::vector<ModInfo> mods_;
    
    // 资源包列表
    std::vector<ResourcePackInfo> resourcePacks_;
    
    // 日志
    std::vector<std::string> logs_;
    bool autoScroll_;
    
    // 设置
    bool showFPS_;
    bool showMemory_;
    bool showDebugInfo_;
    
    // 回调
    ToggleCallback toggleCallback_;
    
    // ImGui样式
    ImVec4 colorPrimary_;
    ImVec4 colorSecondary_;
    ImVec4 colorSuccess_;
    ImVec4 colorWarning_;
    ImVec4 colorError_;
};

// 游戏内GUI渲染器
class InGameGUIRenderer {
public:
    static InGameGUIRenderer& GetInstance();
    
    // 初始化
    bool Initialize(GLFWwindow* window);
    
    // 关闭
    void Shutdown();
    
    // 新帧
    void NewFrame();
    
    // 渲染
    void Render();
    
    // 处理输入
    void HandleInput(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    // 获取GUI管理器
    InGameGUI& GetGUI();
    
private:
    InGameGUIRenderer();
    ~InGameGUIRenderer();
    
    InGameGUI gui_;
    bool initialized_;
};

} // namespace ingame
} // namespace gui
} // namespace mcu
