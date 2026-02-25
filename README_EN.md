# Minecraft Unifier v2.0

A cross-platform Minecraft mod compatibility tool that enables resource interoperability between Java Edition, Bedrock Edition (International), and Netease China Edition.

## Project Overview

This project aims to break down the resource barriers between Minecraft Java Edition, Bedrock Edition (International), and Netease China Edition, enabling Bedrock Edition to fully support Java Edition shader packs, various mods, and resource packs.

### Core Features

- **Static Injection Technology**: All modifications are completed during installation/build phase; only necessary compatibility modules are loaded during game runtime
- **Unified Codebase Across Three Platforms**: Windows, Linux, and Android share core compatibility code (approximately 80%)
- **Cross-Platform Compatibility**: Supports Windows, Linux, and Android
- **Multiple Injection Methods**:
  - Windows: PE file import table hijacking
  - Linux: ELF file PT_NOTE injection
  - Android: APK repackaging + so library preloading

## Project Structure

```
minecraft-unifier/
├── common/              # Common code library
│   ├── cmc_format.h    # .cmc format definition
│   └── cmc_format.cpp  # .cmc format implementation
├── injector/           # Injector
│   ├── android/        # Android platform injector
│   │   ├── apk_injector.h
│   │   └── apk_injector.cpp
│   ├── windows/        # Windows platform injector
│   │   ├── pe_injector.h
│   │   └── pe_injector.cpp
│   └── linux/          # Linux platform injector
│       ├── elf_injector.h
│       └── elf_injector.cpp
├── packer/             # Packer
│   └── windows/        # Windows platform packer (priority)
│       ├── netease_packer.h
│       └── netease_packer.cpp
├── core/               # Unifier core
│   ├── render/         # Render compatibility module
│   │   ├── shader_converter.h
│   │   └── shader_converter.cpp
│   ├── mods/           # Mod compatibility module
│   │   ├── java_runtime.h      # Java mod runtime
│   │   ├── java_runtime.cpp
│   │   ├── netease_runtime.h   # Netease mod runtime
│   │   └── netease_runtime.cpp
│   └── resources/      # Resource management module
│       ├── resource_manager.h
│       └── resource_manager.cpp
├── gui/                # GUI components
│   ├── desktop/        # Desktop GUI
│   ├── ingame/         # In-game GUI
│   └── android/        # Android GUI
└── CMakeLists.txt      # Build configuration
```

## Core Components

### 1. .cmc Format (Cross-Platform Mod Packaging Format)

A unified mod packaging format supporting Java mods, Netease mods, shader packs, resource packs, etc.

**Features**:
- Supports compression (zlib)
- Supports encryption (AES)
- Contains manifest metadata
- Cross-platform compatible

### 2. Netease Project Packer (Windows Priority)

A tool that converts Java mods, Netease mods, and shader packs to .cmc format.

**Supported Conversions**:
- Java mods → .cmc format
- Netease mods → .cmc format
- Shader packs → .cmc format

### 3. Original Plugin Injector (Android Priority)

Injects the loader into the game binary files.

**Injection Methods**:
- **Android**: APK repackaging + so library preloading + xHook framework
- **Windows**: PE file import table hijacking + Detours framework
- **Linux**: ELF file PT_NOTE injection + PLT Hook

### 4. Unifier Core

#### Render Compatibility Module
- GLSL shader conversion (Java Edition → Render Dragon)
- SPIR-V compilation and caching
- Render Dragon API wrapper

#### Mod Compatibility Module
- **Java Mod Runtime**: Embedded JVM + API mapping
- **Netease Mod Runtime**: Embedded Python + SDK simulation

#### Resource Management Module
- Cross-platform file system Hook
- Resource format conversion (textures, models, sounds)
- Resource pack management

## Build Instructions

### Dependencies

- CMake 3.16+
- C++17 compiler (GCC/Clang/MSVC)
- zlib (compression library)
- Python 3 (Netease mod runtime)
- Platform-specific dependencies:
  - Windows: Detours library
  - Linux: dl library
  - Android: Android NDK, xHook library

### Build Steps

```bash
# Create build directory
mkdir build && cd build

# Configure CMake
cmake ..

# Build
cmake --build .

# Install
cmake --install .
```

### Platform-Specific Builds

#### Windows
```bash
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
```

#### Linux
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### Android
```bash
# Set Android NDK path
export ANDROID_NDK=/path/to/ndk

# Configure CMake
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 ..

# Build
cmake --build .
```

## Usage

### 1. Pack Mods

```cpp
#include "packer/windows/netease_packer.h"

using namespace mcu::packer::windows;

// Create unified packer
UnifiedPacker packer;
packer.SetOutputDir("./output");

// Pack Java mod
packer.Pack("mod.jar", "mod.cmc");

// Pack shader pack
packer.Pack("shaderpack/", "shader.cmc");
```

### 2. Inject into Game

#### Android
```cpp
#include "injector/android/apk_injector.h"

using namespace mcu::injector::android;

UnifiedAndroidInjector injector;
injector.AddCompatModule("libRenderCompat.so");
injector.AddCompatModule("libModRuntime.so");
injector.InjectToApk("input.apk", "output.apk");
```

#### Windows
```cpp
#include "injector/windows/pe_injector.h"

using namespace mcu::injector::windows;

UnifiedWindowsInjector injector;
injector.AddCompatModule("RenderCompat.dll");
injector.AddCompatModule("ModRuntime.dll");
injector.InjectToPE("Minecraft.exe", "Minecraft.Unified.exe");
```

#### Linux
```cpp
#include "injector/linux/elf_injector.h"

using namespace mcu::injector::linux;

UnifiedLinuxInjector injector;
injector.AddCompatModule("libRenderCompat.so");
injector.AddCompatModule("libModRuntime.so");
injector.InjectToELF("minecraft-pe", "minecraft-pe.unified");
```

### 3. Runtime Usage

```cpp
#include "core/render/shader_converter.h"
#include "core/mods/java_runtime.h"
#include "core/resources/resource_manager.h"

using namespace mcu::core;

// Initialize render compatibility module
render::ShaderConverter shaderConverter;
shaderConverter.Initialize();
shaderConverter.LoadJavaShaderpack("/path/to/shaderpack");
shaderConverter.CompileToRenderDragon("gbuffers_terrain");

// Initialize Java mod runtime
mods::JavaModRuntime javaRuntime;
javaRuntime.Initialize();
javaRuntime.LoadMod("/path/to/mod.cmc");

// Initialize resource manager
resources::ResourceManager resourceManager;
resourceManager.Initialize();
resourceManager.InstallFileHooks();
```

## Development Roadmap

### Phase 1: Basic Framework and Toolchain (Completed)
- ✅ Create project structure
- ✅ Implement .cmc format
- ✅ Develop three-platform injectors
- ✅ Develop Windows platform packer

### Phase 2: Render Compatibility Module (Completed)
- ✅ GLSL parsing and conversion framework
- ✅ Render Dragon API wrapper
- ✅ Shader caching system

### Phase 3: Mod Compatibility Module (Completed)
- ✅ Embedded JVM implementation
- ✅ Java API mapping
- ✅ Embedded Python implementation
- ✅ Netease SDK simulation

### Phase 4: Resource Management and GUI (Pending)
- ⏳ Cross-platform file system Hook
- ⏳ Resource format converter
- ⏳ ImGui in-game control panel
- ⏳ Qt6 management tool

### Phase 5: Integration and Testing (Pending)
- ⏳ Complete three-platform integration
- ⏳ Compatibility testing
- ⏳ Performance optimization

## Legal Disclaimer

This project is an open-source learning project for technical research purposes only.

**Important Notice**:
1. Use of this tool requires you to legally own a legitimate copy of Minecraft
2. This tool does not contain any copyrighted assets from Mojang Studios
3. All mods, shaders, and resource packs must be obtained by users themselves and belong to their respective copyright holders
4. Developers are not legally responsible for any issues caused by using this tool
5. I'm just a junior high school student，thanks

## License

Unauthorized commercial and illegal use is strictly prohibited. For commercial use, please contact the author.

## Contributing

Issues and Pull Requests are welcome!

## Contact

- Email: jqyh1026@outlook.com

---

**Note**: This project is still under development, and some features may be unstable. Please use with caution.
