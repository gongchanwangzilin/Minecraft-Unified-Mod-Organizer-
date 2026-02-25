/**
 * Minecraft Unifier - In-Game GUI Implementation
 * 游戏内GUI实现 - ImGui覆盖界面
 */

#include "ingame_gui.h"
#include <chrono>
#include <ctime>

namespace mcu {
namespace gui {
namespace ingame {

// ==================== InGameGUI ====================

InGameGUI::InGameGUI()
    : window_(nullptr)
    , visible_(false)
    , initialized_(false)
    , currentTab_(0)
    , autoScroll_(true)
    , showFPS_(true)
    , showMemory_(true)
    , showDebugInfo_(false) {
    
    // 设置ImGui样式颜色
    colorPrimary_ = ImVec4(0.13f, 0.59f, 0.95f, 1.0f);  // 蓝色
    colorSecondary_ = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);   // 深灰色
    colorSuccess_ = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);    // 绿色
    colorWarning_ = ImVec4(0.9f, 0.7f, 0.2f, 1.0f);    // 黄色
    colorError_ = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);      // 红色
}

InGameGUI::~InGameGUI() {
    Shutdown();
}

bool InGameGUI::Initialize(GLFWwindow* window) {
    if (initialized_) {
        return true;
    }
    
    window_ = window;
    
    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    // 初始化ImGui GLFW绑定
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // 设置样式
    SetupStyle();
    
    initialized_ = true;
    return true;
}

void InGameGUI::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    initialized_ = false;
}

void InGameGUI::Render() {
    if (!visible_ || !initialized_) {
        return;
    }
    
    // 新帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // 渲染主菜单
    RenderMainMenu();
    
    // 渲染
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void InGameGUI::ToggleVisibility() {
    visible_ = !visible_;
    
    if (toggleCallback_) {
        toggleCallback_();
    }
}

void InGameGUI::SetVisible(bool visible) {
    visible_ = visible;
}

bool InGameGUI::IsVisible() const {
    return visible_;
}

void InGameGUI::SetToggleCallback(ToggleCallback callback) {
    toggleCallback_ = callback;
}

void InGameGUI::AddMod(const ModInfo& mod) {
    mods_.push_back(mod);
}

void InGameGUI::RemoveMod(const std::string& modId) {
    mods_.erase(
        std::remove_if(mods_.begin(), mods_.end(),
                      [&modId](const ModInfo& mod) {
                          return mod.id == modId;
                      }),
        mods_.end());
}

void InGameGUI::EnableMod(const std::string& modId) {
    for (auto& mod : mods_) {
        if (mod.id == modId) {
            mod.enabled = true;
            break;
        }
    }
}

void InGameGUI::DisableMod(const std::string& modId) {
    for (auto& mod : mods_) {
        if (mod.id == modId) {
            mod.enabled = false;
            break;
        }
    }
}

const std::vector<ModInfo>& InGameGUI::GetMods() const {
    return mods_;
}

void InGameGUI::AddResourcePack(const ResourcePackInfo& pack) {
    resourcePacks_.push_back(pack);
}

void InGameGUI::RemoveResourcePack(const std::string& packId) {
    resourcePacks_.erase(
        std::remove_if(resourcePacks_.begin(), resourcePacks_.end(),
                      [&packId](const ResourcePackInfo& pack) {
                          return pack.id == packId;
                      }),
        resourcePacks_.end());
}

void InGameGUI::EnableResourcePack(const std::string& packId) {
    for (auto& pack : resourcePacks_) {
        if (pack.id == packId) {
            pack.enabled = true;
            break;
        }
    }
}

void InGameGUI::DisableResourcePack(const std::string& packId) {
    for (auto& pack : resourcePacks_) {
        if (pack.id == packId) {
            pack.enabled = false;
            break;
        }
    }
}

const std::vector<ResourcePackInfo>& InGameGUI::GetResourcePacks() const {
    return resourcePacks_;
}

void InGameGUI::AddLog(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tm);
    
    std::string logMessage = "[" + std::string(timeStr) + "] " + message;
    logs_.push_back(logMessage);
    
    // 限制日志数量
    if (logs_.size() > 1000) {
        logs_.erase(logs_.begin());
    }
}

void InGameGUI::ClearLog() {
    logs_.clear();
}

void InGameGUI::SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // 设置深色主题
    ImGui::StyleColorsDark();
    
    // 自定义颜色
    style.Colors[ImGuiCol_Header] = colorPrimary_;
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(colorPrimary_.x + 0.1f, colorPrimary_.y + 0.1f, colorPrimary_.z + 0.1f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(colorPrimary_.x + 0.2f, colorPrimary_.y + 0.2f, colorPrimary_.z + 0.2f, 1.0f);
    style.Colors[ImGuiCol_Button] = colorPrimary_;
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(colorPrimary_.x + 0.1f, colorPrimary_.y + 0.1f, colorPrimary_.z + 0.1f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(colorPrimary_.x + 0.2f, colorPrimary_.y + 0.2f, colorPrimary_.z + 0.2f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = colorSuccess_;
    
    // 设置窗口圆角
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    
    // 设置间距
    style.ItemSpacing = ImVec2(8, 4);
    style.FramePadding = ImVec2(4, 3);
}

void InGameGUI::RenderMainMenu() {
    // 设置窗口位置和大小
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    
    // 创建主窗口
    if (ImGui::Begin("我的世界统一器", &visible_, ImGuiWindowFlags_MenuBar)) {
        // 菜单栏
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("文件")) {
                if (ImGui::MenuItem("关闭", "F1")) {
                    visible_ = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("视图")) {
                ImGui::MenuItem("显示FPS", nullptr, &showFPS_);
                ImGui::MenuItem("显示内存", nullptr, &showMemory_);
                ImGui::MenuItem("显示调试信息", nullptr, &showDebugInfo_);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("帮助")) {
                if (ImGui::MenuItem("关于")) {
                    currentTab_ = 4; // 关于Tab
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        // Tab切换
        ImGui::BeginTabBar("MainTabBar");
        
        if (ImGui::BeginTabItem("模组管理")) {
            currentTab_ = 0;
            RenderModsTab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("资源包")) {
            currentTab_ = 1;
            RenderResourcePacksTab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("设置")) {
            currentTab_ = 2;
            RenderSettingsTab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("日志")) {
            currentTab_ = 3;
            RenderLogTab();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("关于")) {
            currentTab_ = 4;
            RenderAboutTab();
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
        
        // 显示FPS
        if (showFPS_) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        
        // 显示内存
        if (showMemory_) {
            ImGui::SameLine();
            ImGui::Text(" | 内存: %.2f MB", ImGui::GetIO().MetricsAllocs / 1024.0f / 1024.0f);
        }
    }
    
    ImGui::End();
}

void InGameGUI::RenderModsTab() {
    ImGui::Text("已加载的模组 (%zu)", mods_.size());
    ImGui::Separator();
    
    if (ImGui::BeginChild("ModsList", ImVec2(0, 300), true)) {
        for (size_t i = 0; i < mods_.size(); i++) {
            const ModInfo& mod = mods_[i];
            
            ImGui::PushID(i);
            
            // 模组名称
            ImGui::Text("%s", mod.name.c_str());
            ImGui::SameLine();
            
            // 模组版本
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "v%s", mod.version.c_str());
            
            // 模组ID
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "ID: %s", mod.id.c_str());
            
            // 状态标签
            if (mod.loaded) {
                ImGui::SameLine();
                ImGui::TextColored(colorSuccess_, "[已加载]");
            } else {
                ImGui::SameLine();
                ImGui::TextColored(colorWarning_, "[未加载]");
            }
            
            // 启用/禁用按钮
            if (mod.enabled) {
                if (ImGui::SmallButton("禁用")) {
                    DisableMod(mod.id);
                }
            } else {
                if (ImGui::SmallButton("启用")) {
                    EnableMod(mod.id);
                }
            }
            
            ImGui::Separator();
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    
    // 添加模组按钮
    if (ImGui::Button("加载模组")) {
        // TODO: 实现模组加载对话框
    }
}

void InGameGUI::RenderResourcePacksTab() {
    ImGui::Text("已加载的资源包 (%zu)", resourcePacks_.size());
    ImGui::Separator();
    
    if (ImGui::BeginChild("ResourcePacksList", ImVec2(0, 300), true)) {
        for (size_t i = 0; i < resourcePacks_.size(); i++) {
            const ResourcePackInfo& pack = resourcePacks_[i];
            
            ImGui::PushID(i);
            
            // 资源包名称
            ImGui::Text("%s", pack.name.c_str());
            
            // 状态标签
            if (pack.applied) {
                ImGui::SameLine();
                ImGui::TextColored(colorSuccess_, "[已应用]");
            } else {
                ImGui::SameLine();
                ImGui::TextColored(colorWarning_, "[未应用]");
            }
            
            // 启用/禁用按钮
            if (pack.enabled) {
                if (ImGui::SmallButton("禁用")) {
                    DisableResourcePack(pack.id);
                }
            } else {
                if (ImGui::SmallButton("启用")) {
                    EnableResourcePack(pack.id);
                }
            }
            
            ImGui::Separator();
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    
    // 添加资源包按钮
    if (ImGui::Button("加载资源包")) {
        // TODO: 实现资源包加载对话框
    }
}

void InGameGUI::RenderSettingsTab() {
    ImGui::Text("显示设置");
    ImGui::Separator();
    
    ImGui::Checkbox("显示FPS", &showFPS_);
    ImGui::Checkbox("显示内存", &showMemory_);
    ImGui::Checkbox("显示调试信息", &showDebugInfo_);
    
    ImGui::Separator();
    
    ImGui::Text("日志设置");
    ImGui::Separator();
    
    ImGui::Checkbox("自动滚动", &autoScroll_);
    
    ImGui::Separator();
    
    ImGui::Text("调试信息");
    ImGui::Separator();
    
    if (showDebugInfo_) {
        ImGui::Text("ImGui版本: %s", ImGui::GetVersion());
        ImGui::Text("窗口大小: %.0f x %.0f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
        ImGui::Text("鼠标位置: %.0f, %.0f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);
    }
}

void InGameGUI::RenderLogTab() {
    ImGui::Text("日志 (%zu条)", logs_.size());
    ImGui::Separator();
    
    if (ImGui::Button("清空日志")) {
        ClearLog();
    }
    
    ImGui::SameLine();
    ImGui::Checkbox("自动滚动", &autoScroll_);
    
    ImGui::Separator();
    
    if (ImGui::BeginChild("LogList", ImVec2(0, 350), true)) {
        for (const auto& log : logs_) {
            ImGui::Text("%s", log.c_str());
        }
        
        if (autoScroll_) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
}

void InGameGUI::RenderAboutTab() {
    ImGui::Text("我的世界统一器 v2.0.0");
    ImGui::Separator();
    
    ImGui::Text("一个跨平台的我的世界模组统一工具，支持Java版、基岩版和网易版之间的资源互通。");
    ImGui::Separator();
    
    ImGui::Text("功能特性：");
    ImGui::BulletText("模组打包：将Java版/网易版模组打包为.cmc格式");
    ImGui::BulletText("游戏注入：将统一器注入到游戏中");
    ImGui::BulletText("资源转换：转换不同版本之间的资源格式");
    ImGui::BulletText("资源包管理：加载和管理资源包");
    ImGui::Separator();
    
    ImGui::Text("联系方式：jqyh1026@outlook.com");
    ImGui::Separator();
    
    ImGui::TextColored(colorWarning_, "版权声明：");
    ImGui::Text("Mojang Studios 拥有《我的世界》的版权，本项目未经授权。");
    ImGui::Text("个人使用可直接使用，非个人使用、商用需获得授权。");
}

// ==================== InGameGUIRenderer ====================

InGameGUIRenderer::InGameGUIRenderer()
    : initialized_(false) {
}

InGameGUIRenderer::~InGameGUIRenderer() {
    Shutdown();
}

InGameGUIRenderer& InGameGUIRenderer::GetInstance() {
    static InGameGUIRenderer instance;
    return instance;
}

bool InGameGUIRenderer::Initialize(GLFWwindow* window) {
    if (initialized_) {
        return true;
    }
    
    if (!gui_.Initialize(window)) {
        return false;
    }
    
    initialized_ = true;
    return true;
}

void InGameGUIRenderer::Shutdown() {
    if (!initialized_) {
        return;
    }
    
    gui_.Shutdown();
    initialized_ = false;
}

void InGameGUIRenderer::NewFrame() {
    if (!initialized_) {
        return;
    }
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void InGameGUIRenderer::Render() {
    if (!initialized_) {
        return;
    }
    
    gui_.Render();
}

void InGameGUIRenderer::HandleInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!initialized_) {
        return;
    }
    
    // F1键切换GUI可见性
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        gui_.ToggleVisibility();
    }
}

InGameGUI& InGameGUIRenderer::GetGUI() {
    return gui_;
}

} // namespace ingame
} // namespace gui
} // namespace mcu
