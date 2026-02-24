# 我的世界统一器 - 开源指南

欢迎来到"我的世界统一器"（Minecraft Unifier）开源项目！

## 项目简介

"我的世界统一器"是一个跨平台的Minecraft模组兼容工具，旨在打破Java版、基岩版（国际版）、网易中国版之间的资源壁垒，实现基岩版对Java版光影包、各类模组、资源包的完整兼容。

### 核心特性

- **静态注入技术**：所有修改在安装/构建阶段完成，游戏运行时仅加载必要的兼容模块
- **三端统一代码基**：Windows、Linux、Android共享核心兼容代码（约80%）
- **跨平台兼容**：支持Windows、Linux、Android三大平台
- **多种注入方式**：
  - Windows：PE文件导入表劫持
  - Linux：ELF文件PT_NOTE注入
  - Android：APK重打包 + so库预加载

## 开源协议

本项目采用 **MIT License** 开源协议。

### 协议要点

- ✅ 商业使用
- ✅ 修改
- ✅ 分发
- ✅ 私人使用
- ❌ 责任限制
- ❌ 担保

详见 [LICENSE](LICENSE) 文件。

## 快速开始

### 环境要求

- **编译器**：支持C++17的编译器（GCC 7+、Clang 5+、MSVC 2017+）
- **构建工具**：CMake 3.16+
- **依赖库**：
  - zlib（压缩）
  - Python 3（网易模组运行时）
  - Detours（Windows Hook）
  - xHook（Android Hook）
  - glslang（着色器编译）

### 获取代码

```bash
git clone https://github.com/your-repo/minecraft-unifier.git
cd minecraft-unifier
```

### 构建项目

#### Windows

```bash
mkdir build && cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
```

#### Linux

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### Android

```bash
export ANDROID_NDK=/path/to/ndk
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 ..
cmake --build .
```

## 项目结构

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
├── tests/              # 测试代码
├── docs/               # 文档
├── CMakeLists.txt      # 构建配置
├── README.md           # 项目说明
├── CONTRIBUTING.md     # 贡献指南
├── CHANGELOG.md        # 变更日志
├── LICENSE             # 开源协议
└── OPEN_SOURCE_GUIDE.md # 本文档
```

## 贡献方式

我们欢迎任何形式的贡献！

### 如何贡献

1. 阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详细的贡献指南
2. Fork项目到您的GitHub账号
3. 创建特性分支进行开发
4. 提交Pull Request

### 贡献类型

- 🐛 修复bug
- ✨ 添加新功能
- 📝 改进文档
- 🎨 优化代码结构
- ⚡ 性能优化
- ✅ 添加测试
- 🌍 国际化支持

## 社区

### 获取帮助

- 📖 [技术文档](TECHNICAL_DOCUMENTATION.md)
- 📧 邮箱：jqyh1026@outlook.com

## 开发路线图

### 已完成 ✅

- [x] .cmc格式实现
- [x] 三平台注入器框架
- [x] 网易项目打包器
- [x] 渲染兼容模块
- [x] 模组兼容模块
- [x] 资源管理模块

### 进行中 🚧

- [ ] GUI界面开发
- [ ] 更多光影包支持
- [ ] 性能优化

### 计划中 📋

- [ ] macOS平台支持
- [ ] iOS平台支持
- [ ] 更多模组类型支持
- [ ] 云端模组库

## 法律声明

### 重要声明

1. **正版要求**：使用本工具需要您已合法拥有《我的世界》正版游戏
2. **版权资产**：本工具不包含任何Mojang Studios的版权资产
3. **模组版权**：所有模组、光影、资源包需用户自行获取，版权归原作者所有
4. **责任限制**：使用本工具造成的任何问题，开发者不承担法律责任

### 合规策略

本项目采用以下合规策略：

- **工具-only**：仅提供兼容工具，不分发任何游戏资产
- **用户责任**：用户需自行获取正版游戏和模组
- **技术中立**：技术本身不违反任何法律
- **开源透明**：所有代码开源，接受社区审查

## 致谢

### 特别感谢

- **Mojang Studios**：开发了《我的世界》
- **Minecraft社区**：提供了各种模组和资源
- **开源社区**：提供了各种工具和库

### 依赖项目

- [zlib](https://zlib.net/) - 压缩库
- [Python](https://www.python.org/) - 编程语言
- [Detours](https://github.com/microsoft/Detours) - Windows Hook库
- [xHook](https://github.com/iqiyi/xHook) - Android Hook库
- [glslang](https://github.com/KhronosGroup/glslang) - GLSL编译器
- [nlohmann/json](https://github.com/nlohmann/json) - JSON库



## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件

---

**让我们一起打破Minecraft的版本壁垒！** 🎮✨
