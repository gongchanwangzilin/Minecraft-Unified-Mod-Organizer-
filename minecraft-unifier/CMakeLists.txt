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
