cmake_minimum_required(VERSION 3.16)
project(MinecraftUnifier VERSION 2.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
else()
    add_compile_options(-O3 -DNDEBUG)
endif()

# 查找依赖包
find_package(ZLIB REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# 查找Qt6（桌面端GUI）
find_package(Qt6 QUIET COMPONENTS Core Widgets)

# 查找GLFW和ImGui（游戏内GUI）
find_package(glfw3 QUIET)

# 查找JNI（Android）
if(ANDROID)
    find_package(JNI REQUIRED)
endif()

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/injector
    ${CMAKE_SOURCE_DIR}/packer
)

# 通用库
add_library(cmc_lib STATIC
    common/cmc_format.cpp
    common/cmc_format.h
)
target_link_libraries(cmc_lib ZLIB::ZLIB)

# 核心库
add_library(core_lib STATIC
    core/render/shader_converter.cpp
    core/render/shader_converter.h
    core/mods/java_runtime.cpp
    core/mods/java_runtime.h
    core/mods/netease_runtime.cpp
    core/mods/netease_runtime.h
    core/resources/resource_manager.cpp
    core/resources/resource_manager.h
)
target_link_libraries(core_lib
    cmc_lib
    ZLIB::ZLIB
    Python3::Python
)

# Windows平台特定
if(WIN32)
    # PE注入器
    add_library(pe_injector_lib STATIC
        injector/windows/pe_injector.cpp
        injector/windows/pe_injector.h
    )
    target_link_libraries(pe_injector_lib core_lib)
    
    # 网易打包器
    add_library(netease_packer_lib STATIC
        packer/windows/netease_packer.cpp
        packer/windows/netease_packer.h
    )
    target_link_libraries(netease_packer_lib core_lib)
endif()

# Linux平台特定
if(UNIX AND NOT ANDROID)
    # ELF注入器
    add_library(elf_injector_lib STATIC
        injector/linux/elf_injector.cpp
        injector/linux/elf_injector.h
    )
    target_link_libraries(elf_injector_lib core_lib dl)
endif()

# Android平台特定
if(ANDROID)
    # APK注入器
    add_library(apk_injector_lib STATIC
        injector/android/apk_injector.cpp
        injector/android/apk_injector.h
    )
    target_link_libraries(apk_injector_lib core_lib log)
    
    # Android GUI
    add_library(android_gui_lib STATIC
        gui/android/android_gui.cpp
        gui/android/android_gui.h
    )
    target_link_libraries(android_gui_lib core_lib JNI::JNI)
endif()

# 桌面端GUI（Qt6）
if(Qt6_FOUND AND (WIN32 OR (UNIX AND NOT ANDROID)))
    # 桌面端GUI
    add_library(desktop_gui_lib STATIC
        gui/desktop/main_window.cpp
        gui/desktop/main_window.h
    )
    target_link_libraries(desktop_gui_lib 
        core_lib 
        Qt6::Core 
        Qt6::Widgets
    )
    
    # 桌面端可执行文件
    add_executable(minecraft-unifier-desktop
        gui/desktop/main.cpp
    )
    target_link_libraries(minecraft-unifier-desktop 
        desktop_gui_lib 
        core_lib 
        cmc_lib
    )
endif()

# 游戏内GUI（ImGui + GLFW）
if(glfw3_FOUND AND (WIN32 OR (UNIX AND NOT ANDROID)))
    # 游戏内GUI
    add_library(ingame_gui_lib STATIC
        gui/ingame/ingame_gui.cpp
        gui/ingame/ingame_gui.h
    )
    target_link_libraries(ingame_gui_lib 
        core_lib 
        glfw
    )
    
    # 添加ImGui包含目录
    target_include_directories(ingame_gui_lib PRIVATE ${CMAKE_SOURCE_DIR}/third_party/imgui)
endif()

# 安装规则
install(TARGETS cmc_lib core_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES
    common/cmc_format.h
    core/render/shader_converter.h
    core/mods/java_runtime.h
    core/mods/netease_runtime.h
    core/resources/resource_manager.h
    DESTINATION include/minecraft-unifier
)

# Windows平台特定安装
if(WIN32)
    install(TARGETS minecraft-unifier-desktop
        RUNTIME DESTINATION bin
    )
    
    # 安装Windows依赖
    if(Qt6_FOUND)
        install(FILES ${Qt6_DIR}/../../../bin/Qt6Core.dll ${Qt6_DIR}/../../../bin/Qt6Widgets.dll
            DESTINATION bin
        )
    endif()
endif()

# Linux平台特定安装
if(UNIX AND NOT ANDROID)
    install(TARGETS minecraft-unifier-desktop
        RUNTIME DESTINATION bin
    )
    
    # 安装桌面文件
    install(FILES minecraft-unifier.desktop
        DESTINATION share/applications
    )
    
    # 安装图标
    install(FILES icons/minecraft-unifier.png
        DESTINATION share/icons/hicolor/256x256/apps
    )
endif()

# Android平台特定安装
if(ANDROID)
    # 安装Android库
    install(TARGETS android_gui_lib apk_injector_lib core_lib cmc_lib
        ARCHIVE DESTINATION lib/${ANDROID_ABI}
    )
    
    # 安装JNI头文件
    install(FILES gui/android/android_gui.h
        DESTINATION include/minecraft-unifier
    )
endif()

# 打包配置
include(CPack)

set(CPACK_PACKAGE_NAME "MinecraftUnifier")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Minecraft Unifier - Cross-platform mod unification tool")
set(CPACK_PACKAGE_VENDOR "Minecraft Unifier Team")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(UNIX AND NOT ANDROID)
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
elseif(ANDROID)
    set(CPACK_GENERATOR "TGZ")
endif()
