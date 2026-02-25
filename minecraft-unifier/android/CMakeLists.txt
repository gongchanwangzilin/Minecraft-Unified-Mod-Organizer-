cmake_minimum_required(VERSION 3.18.1)
project("minecraft-unifier-android")

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Android API级别
set(ANDROID_PLATFORM android-24)
set(ANDROID_STL c++_shared)

# 查找依赖包
find_package(ZLIB REQUIRED)
find_package(log REQUIRED)
find_package(JNI REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/injector
    ${CMAKE_SOURCE_DIR}/packer
    ${CMAKE_SOURCE_DIR}/gui
)

# 通用库
add_library(cmc_lib SHARED
    common/cmc_format.cpp
    common/cmc_format.h
)
target_link_libraries(cmc_lib ZLIB::ZLIB)

# 核心库
add_library(core_lib SHARED
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
    log
)

# APK注入器
add_library(apk_injector_lib SHARED
    injector/android/apk_injector.cpp
    injector/android/apk_injector.h
)
target_link_libraries(apk_injector_lib core_lib log)

# Android GUI
add_library(android_gui_lib SHARED
    gui/android/android_gui.cpp
    gui/android/android_gui.h
)
target_link_libraries(android_gui_lib core_lib JNI::JNI)

# 统一器主库
add_library(minecraft-unifier SHARED
    android_main.cpp
)
target_link_libraries(minecraft-unifier 
    android_gui_lib 
    apk_injector_lib 
    core_lib 
    cmc_lib
    log
    android
)

# 设置输出目录
set_target_properties(cmc_lib PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/libs/${ANDROID_ABI}
)

set_target_properties(core_lib PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/libs/${ANDROID_ABI}
)

set_target_properties(apk_injector_lib PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/libs/${ANDROID_ABI}
)

set_target_properties(android_gui_lib PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/libs/${ANDROID_ABI}
)

set_target_properties(minecraft-unifier PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/libs/${ANDROID_ABI}
)
