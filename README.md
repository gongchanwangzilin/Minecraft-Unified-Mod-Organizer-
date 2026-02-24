# Minecraft-Unified-Mod-Organizer-
一个我的世界游戏的统一插件，希望可以打破三版本模组无法通用的壁垒，让更多开发者更轻松的开发。A unified plugin for Minecraft that aims to break the barrier of incompatible mods across three editions, making development easier for more developers.
跨平台Minecraft模组兼容工具，实现Java版、基岩版（国际版）、网易中国版之间的资源互通。
 
---
 
## 📖 目录
 
- [项目概述](#项目概述)
- [核心特性](#核心特性)
- [项目结构](#项目结构)
- [核心组件](#核心组件)
- [技术架构](#技术架构)
- [构建说明](#构建说明)
- [使用说明](#使用说明)
- [开发指南](#开发指南)
- [部署指南](#部署指南)
- [贡献指南](#贡献指南)
- [常见问题](#常见问题)
- [安全与合规](#安全与合规)
- [变更日志](#变更日志)
- [开源协议](#开源协议)
- [联系方式](#联系方式)
 
---
 
## 项目概述
 
本项目旨在打破《我的世界》Java版、基岩版（国际版）、网易中国版之间的资源壁垒，实现基岩版对Java版光影包、各类模组、资源包的完整兼容。
 
### 核心特性
### 技术栈
 
- **核心语言**：C++17
- **构建系统**：CMake 3.16+
- **依赖库**：
  - zlib（压缩）
  - Python 3（网易模组运行时）
  - Detours（Windows Hook）
  - xHook（Android Hook）
  - glslang（着色器编译）
 
---
 
## 核心特性
 
- **静态注入技术**：所有修改在安装/构建阶段完成，游戏运行时仅加载必要的兼容模块
- **三端统一代码基**：Windows、Linux、Android共享核心兼容代码（约80%）
- **跨平台兼容**：支持Windows、Linux、Android三大平台
  - Linux：ELF文件PT_NOTE注入
  - Android：APK重打包 + so库预加载
 
---
 
## 项目结构
 
```
│   └── cmc_format.cpp  # .cmc格式实现
├── injector/           # 注入器
│   ├── android/        # Android平台注入器
│   │   ├── apk_injector.h
│   │   └── apk_injector.cpp
│   ├── windows/        # Windows平台注入器
│   │   ├── pe_injector.h
│   │   └── pe_injector.cpp
│   └── linux/          # Linux平台注入器
│       ├── elf_injector.h
│       └── elf_injector.cpp
├── packer/             # 打包器
│   └── windows/        # Windows平台打包器（优先）
│       ├── netease_packer.h
│       └── netease_packer.cpp
├── core/               # 统一器核心
│   ├── render/         # 渲染兼容模块
│   │   ├── shader_converter.h
│   │   └── shader_converter.cpp
│   ├── mods/           # 模组兼容模块
│   │   ├── java_runtime.h      # Java模组运行时
│   │   ├── java_runtime.cpp
│   │   ├── netease_runtime.h   # 网易模组运行时
│   │   └── netease_runtime.cpp
│   └── resources/      # 资源管理模块
│       ├── resource_manager.h
│       └── resource_manager.cpp
├── gui/                # GUI组件
│   ├── desktop/        # 桌面端GUI
│   ├── ingame/         # 游戏内GUI
└── CMakeLists.txt      # 构建配置
```
 
---
 
## 核心组件
 
### 1. .cmc格式（跨平台模组封装格式）
- 资源格式转换（纹理、模型、声音）
- 资源包管理
 
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
 
---
 
## 构建说明
 
### 依赖项
cmake --build .
```
---
## 使用说明
### 1. 打包模组
resourceManager.InstallFileHooks();
```
 
---
 
## 开发指南
 
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
 
### 测试要求
 
#### 单元测试
 
- 为新功能添加单元测试
- 使用Google Test框架
- 测试文件放在 `tests/` 目录
 
```cpp
#include <gtest/gtest.h>
#include "common/cmc_format.h"
TEST(CMCPackerTest, PackAndUnpack) {
    mcu::cmc::CMCPacker packer;
    
    // 打包测试
    ASSERT_TRUE(packer.Pack("./test_input", "./test_output.cmc"));
    
    // 验证测试
    ASSERT_TRUE(packer.Validate("./test_output.cmc"));
    
    // 解包测试
    ASSERT_TRUE(packer.Unpack("./test_output.cmc", "./test_output"));
}
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
 
1. **Fork项目**
   ```bash
   git clone https://github.com/your-username/minecraft-unifier.git
   cd minecraft-unifier
   ```
 
2. **创建分支**
   ```bash
   git checkout -b feature/your-feature-name
   ```
 
3. **进行修改**
   - 遵循代码规范
   - 添加必要的测试
   - 更新相关文档
 
4. **提交更改**
   ```bash
   git add .
   git commit -m "feat: add your feature description"
   ```
 
5. **推送到分支**
   ```bash
   git push origin feature/your-feature-name
   ```
 
6. **创建Pull Request**
 
### 提交信息规范
 
使用[Conventional Commits](https://www.conventionalcommits.org/)格式：
 
- `feat:` 新功能
- `fix:` 修复bug
- `docs:` 文档更新
- `style:` 代码格式调整
- `refactor:` 重构
- `test:` 测试相关
- `chore:` 构建/工具链相关
 
### 行为准则
 
为了营造开放和友好的环境，我们承诺：
 
- 尊重不同的观点和经验
- 优雅地接受建设性批评
- 关注对社区最有利的事情
- 对其他社区成员表示同理心
 
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
 
### Q6: 使用这个工具安全吗？
 
**A**: 是的。我们的代码完全开源，接受社区审查。我们：
- 不收集用户数据
- 不进行网络通信
- 不包含恶意代码
- 采用静态注入技术
 
### Q7: 这个工具合法吗？
 
**A**: 在大多数地区是合法的，因为：
- 我们不分发游戏资产
- 我们不分发受版权保护的模组
- 我们仅提供兼容工具
- 用户需自行拥有正版游戏
 
但请遵守您所在地区的法律法规。
 
### Q8: 如何获取帮助？
 
**A**: 您可以通过以下方式获取帮助：
- 查看本文档
- 发送邮件到 jqyh1026@outlook.com
 
---
 
## 安全与合规
 
### 代码安全
 
#### 静态注入技术
 
我们采用静态注入技术，所有修改在安装/构建阶段完成：
 
- **无运行时修改**：游戏运行时仅加载必要的兼容模块
- **无恶意代码**：所有代码开源，接受社区审查
- **无数据收集**：不收集任何用户数据
 
#### 文件操作安全
 
- **只读操作**：仅读取用户指定的模组文件
- **备份机制**：修改前自动备份原文件
- **权限检查**：确保有足够的文件操作权限
 
### 用户隐私
 
#### 数据收集
 
- ❌ 不收集用户数据
- ❌ 不收集使用统计
- ❌ 不收集游戏数据
- ❌ 不收集模组数据
 
#### 数据存储
 
- 所有操作在本地完成
- 不上传任何数据到服务器
- 不与第三方共享数据
 
### 法律合规
 
#### 版权声明
 
- **Mojang Studios** 拥有《我的世界》的版权
- 本工具不包含任何Mojang Studios的版权资产
- 用户需自行购买正版游戏
- 模组版权归原作者所有
- 用户需自行获取模组
 
#### 使用条款
 
##### 允许的使用
 
✅ 个人学习和研究
✅ 技术探索和实验
✅ 开源贡献
✅ 非商业用途
 
##### 禁止的使用
 
❌ 分发游戏资产
❌ 分发受版权保护的模组
❌ 商业用途（需获得授权）
❌ 破坏游戏平衡
❌ 绕过游戏反作弊系统
 
#### 用户责任
 
使用本工具的风险由用户自行承担：
 
- 确保拥有正版游戏
- 确保拥有模组的使用权
- 遵守游戏服务条款
- 遵守当地法律法规
 
#### 开发者免责
 
开发者不对以下情况承担责任：
 
- 因使用本工具导致的游戏封号
- 因使用本工具导致的设备损坏
- 因使用本工具导致的法律纠纷
- 因使用本工具导致的任何损失
 
### 技术合规
 
#### 逆向工程
 
本项目涉及以下技术：
 
- PE文件格式分析
- ELF文件格式分析
- APK文件格式分析
- 函数Hook技术
 
#### 合规性
 
- 仅用于兼容性目的
- 不用于破解游戏
- 不用于绕过DRM
- 不用于作弊
 
### 安全审计
 
#### 代码审查
 
- 所有代码开源
- 接受社区审查
- 定期安全审计
- 及时修复漏洞
 
#### 漏洞报告
 
如果您发现安全漏洞，请：
 
1. 通过私有渠道报告（jqyh1026@outlook.com）
2. 详细描述漏洞信息
3. 等待确认后再公开
4. 遵守负责任的披露原则
 
---
 
## 变更日志
 
### [2.0.0] - 2026-02-24
 
#### 新增
- **核心功能**
  - 跨平台模组封装格式（.cmc）
  - 支持Java模组、网易模组、光影包、资源包
  - 压缩（zlib）和加密（AES）支持
  - JSON manifest元数据管理
 
- **网易项目打包器**
  - Java模组转换器
  - 网易模组转换器
  - 光影包转换器
  - 统一打包器接口
 
- **原插件注入器**
  - Android：APK重打包 + so库预加载 + xHook框架
  - Windows：PE文件导入表劫持 + Detours框架
  - Linux：ELF文件PT_NOTE注入 + PLT Hook
 
- **统一器核心**
  - 渲染兼容模块：GLSL→Render Dragon转换、SPIR-V编译
  - 模组兼容模块：嵌入式JVM、嵌入式Python
  - 资源管理模块：跨平台文件系统Hook、资源格式转换
 
- **构建系统**
  - CMake跨平台构建配置
  - 支持Windows、Linux、Android三平台
 
#### 技术特性
- 静态注入技术（运行时无侵入）
- 三端统一代码基（约80%代码可复用）
- 支持Java版、基岩版、网易版资源互通
- 完整的法律合规策略
 
#### 文档
- 完整的技术文档
- API参考文档
- 开发指南
- 部署指南
 
#### 已知问题
- GUI界面尚未完成
- 部分光影包可能无法完全兼容
- 性能优化空间较大
 
### [1.0.0] - 2026-01-15
 
#### 新增
- 项目初始化
- 基础架构设计
- .cmc格式初步实现
 
---
 
## 开源协议
 
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
 
## 开发计划
 
### 第一阶段：基础框架与工具链（已完成）
- ⏳ 兼容性测试
- ⏳ 性能优化
 
---
 
## 联系方式
 
- 邮箱：jqyh1026@outlook.com
 
---
 
## 法律声明
 
本项目为开源学习项目，仅供技术研究使用。
3. 所有模组、光影、资源包需用户自行获取，版权归原作者所有
4. 使用本工具造成的任何问题，开发者不承担法律责任
 
## 许可证
---
 
MIT License
**注意**：本项目仍在开发中，部分功能可能不稳定。请谨慎使用。
 
Copyright (c) 2026 Minecraft Unifier Team
 
## 贡献
 
欢迎提交Issue和Pull Request！
 
## 文档索引
 
- 📖 [README.md](README.md) - 项目概述（本文档）
- 🌐 [README_EN.md](README_EN.md) - English Version
- 📚 [TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md) - 完整技术文档
- 🌟 [OPEN_SOURCE_GUIDE.md](OPEN_SOURCE_GUIDE.md) - 开源指南
- 🤝 [CONTRIBUTING.md](CONTRIBUTING.md) - 贡献指南
- 📝 [CHANGELOG.md](CHANGELOG.md) - 变更日志
- ❓ [FAQ.md](FAQ.md) - 常见问题
- ⚖️ [SECURITY_AND_COMPLIANCE.md](SECURITY_AND_COMPLIANCE.md) - 安全与合规声明
- 📋 [DOCS_INDEX.md](DOCS_INDEX.md) - 文档导航
 
## 联系方式
 
- 邮箱：jqyh1026@outlook.com
 
---
 
**注意**：本项目仍在开发中，部分功能可能不稳定。请谨慎使用。
## English Version
 
An English version of this documentation is available at [README_EN.md](README_EN.md).
