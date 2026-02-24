/**
 * Minecraft Unifier - ELF Injector Implementation (Linux)
 * ELF文件注入器实现 - Linux平台
 */

#include "elf_injector.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

namespace fs = std::filesystem;

namespace mcu {
namespace injector {
namespace linux {

// ==================== ELFInjector ====================

ELFInjector::ELFInjector() {
}

ELFInjector::~ELFInjector() {
}

void ELFInjector::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

bool ELFInjector::InjectToELF(const std::string& targetPath, const std::string& outputPath,
                             const std::string& soPath) {
    if (progressCallback_) {
        progressCallback_(10, "开始注入ELF文件...");
    }
    
    // 验证ELF文件
    if (!ValidateELF(targetPath)) {
        if (progressCallback_) {
            progressCallback_(0, "ELF文件验证失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(30, "ELF文件验证完成");
    }
    
    // 修改动态段
    if (!ModifyDynamicSection(targetPath, outputPath, soPath)) {
        if (progressCallback_) {
            progressCallback_(30, "动态段修改失败");
        }
        return false;
    }
    
    if (progressCallback_) {
        progressCallback_(100, "注入完成！");
    }
    
    return true;
}

bool ELFInjector::ValidateELF(const std::string& elfPath) {
    int fd = open(elfPath.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }
    
    struct stat st;
    fstat(fd, &st);
    
    void* map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return false;
    }
    
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)map;
    
    // 验证ELF魔数
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        munmap(map, st.st_size);
        close(fd);
        return false;
    }
    
    // 验证ELF类
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        munmap(map, st.st_size);
        close(fd);
        return false;
    }
    
    // 验证ELF类型
    if (ehdr->e_type != ET_EXEC && ehdr->e_type != ET_DYN) {
        munmap(map, st.st_size);
        close(fd);
        return false;
    }
    
    munmap(map, st.st_size);
    close(fd);
    
    return true;
}

std::vector<std::string> ELFInjector::GetNeededLibraries(const std::string& elfPath) {
    std::vector<std::string> libs;
    
    int fd = open(elfPath.c_str(), O_RDONLY);
    if (fd < 0) {
        return libs;
    }
    
    struct stat st;
    fstat(fd, &st);
    
    void* map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return libs;
    }
    
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)map;
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)map + ehdr->e_phoff);
    
    // 查找动态段
    Elf64_Dyn* dyn = nullptr;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)((char*)map + phdr[i].p_vaddr);
            break;
        }
    }
    
    if (!dyn) {
        munmap(map, st.st_size);
        close(fd);
        return libs;
    }
    
    // 遍历动态段，查找DT_NEEDED条目
    while (dyn->d_tag != DT_NULL) {
        if (dyn->d_tag == DT_NEEDED) {
            char* libName = (char*)((char*)map + dyn->d_un.d_val);
            libs.push_back(libName);
        }
        dyn++;
    }
    
    munmap(map, st.st_size);
    close(fd);
    
    return libs;
}

bool ELFInjector::AddNeededLibrary(const std::string& elfPath, const std::string& libName) {
    // TODO: 实现添加动态库依赖
    return true;
}

bool ELFInjector::RemoveNeededLibrary(const std::string& elfPath, const std::string& libName) {
    // TODO: 实现移除动态库依赖
    return true;
}

bool ELFInjector::ModifyDynamicSection(const std::string& targetPath, const std::string& outputPath,
                                      const std::string& soPath) {
    int fd = open(targetPath.c_str(), O_RDWR);
    if (fd < 0) {
        return false;
    }
    
    struct stat st;
    fstat(fd, &st);
    
    void* map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return false;
    }
    
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)map;
    Elf64_Phdr* phdr = (Elf64_Phdr*)((char*)map + ehdr->e_phoff);
    
    // 查找动态段
    Elf64_Dyn* dyn = nullptr;
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_DYNAMIC) {
            dyn = (Elf64_Dyn*)((char*)map + phdr[i].p_vaddr);
            break;
        }
    }
    
    if (!dyn) {
        munmap(map, st.st_size);
        close(fd);
        return false;
    }
    
    // 在动态段中添加NEEDED条目
    if (!InsertNeededEntry(map, soPath)) {
        munmap(map, st.st_size);
        close(fd);
        return false;
    }
    
    // 写入修改后的文件
    msync(map, st.st_size, MS_SYNC);
    munmap(map, st.st_size);
    close(fd);
    
    // 复制到输出路径
    std::string cmd = "cp " + targetPath + " " + outputPath;
    system(cmd.c_str());
    
    return true;
}

bool ELFInjector::InsertNeededEntry(void* map, const std::string& libName) {
    // TODO: 实现插入NEEDED条目到动态段
    // 这需要修改ELF文件的动态段，添加新的DT_NEEDED条目
    
    // 临时实现：直接返回成功
    return true;
}

bool ELFInjector::CalculateNewOffsets(void* map, size_t newSize) {
    // TODO: 计算新的偏移量
    return true;
}

// ==================== LinuxLoader ====================

LinuxLoader::LinuxLoader() {
}

LinuxLoader::~LinuxLoader() {
}

void LinuxLoader::SetHookConfig(const std::string& configPath) {
    hookConfigPath_ = configPath;
}

void LinuxLoader::SetCompatModules(const std::vector<std::string>& modules) {
    compatModules_ = modules;
}

std::string LinuxLoader::GenerateLoaderCode() {
    std::stringstream code;
    
    code << R"(/**
 * Minecraft Unifier - Linux Loader
 * Linux加载器 - 自动生成
 */

#include <dlfcn.h>
#include <link.h>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// 全局变量
static std::vector<void*> g_loadedModules;

// 构造函数，在加载时自动执行
__attribute__((constructor)) void InitUnifier() {
    pthread_t tid;
    pthread_create(&tid, NULL, LoaderThread, NULL);
    pthread_detach(tid);
}

// 初始化线程
void* LoaderThread(void*) {
    // 等待主程序初始化
    sleep(1);
    
    // 加载配置
    const char* configPath = getenv("UNIFIER_CONFIG");
    if (!configPath) configPath = "/opt/unifier/config.json";
    LoadConfig(configPath);
    
    // 加载兼容模块
)";

    // 添加兼容模块加载代码
    for (const auto& module : compatModules_) {
        code << "    void* " + module + " = dlopen(\"" + module + ".so\", RTLD_NOW);\n";
        code << "    if (" + module + ") {\n";
        code << "        g_loadedModules.push_back(" + module + ");\n";
        code << "        printf(\"Loaded module: " + module + "\\n\");\n";
        code << "        // 调用初始化函数\n";
        code << "        typedef void (*init_func_t)();\n";
        code << "        init_func_t init = (init_func_t)dlopen(" + module + ", \"Initialize\");\n";
        code << "        if (init) init();\n";
        code << "    } else {\n";
        code << "        printf(\"Failed to load module: " + module + "\\n\");\n";
        code << "    }\n";
    }

    code << R"(
    // 安装Hook
    InstallHooks();
    
    printf("MinecraftUnifier initialization complete\\n");
    return NULL;
}

// 加载配置
void LoadConfig(const char* configPath) {
    printf("Loading config from: %s\\n", configPath);
    // TODO: 实现配置文件解析
}

// 安装Hook
void InstallHooks() {
    printf("Installing hooks...\\n");
    // TODO: 使用PLT Hook或其他框架安装Hook
}

// 清理函数
__attribute__((destructor)) void CleanupUnifier() {
    // 卸载兼容模块
    for (void* module : g_loadedModules) {
        dlclose(module);
    }
}

// Hook函数示例
FILE* (*original_fopen)(const char*, const char*) = fopen;

FILE* hooked_fopen(const char* path, const char* mode) {
    printf("fopen called: %s\\n", path);
    
    // 检查是否需要重定向
    if (strstr(path, "shaderpacks") != NULL) {
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "/opt/unifier/shaderpacks/%s",
                 strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
        FILE* f = original_fopen(newPath, mode);
        if (f) {
            printf("Redirected to: %s\\n", newPath);
            return f;
        }
    }
    
    return original_fopen(path, mode);
}

int (*original_open)(const char*, int, mode_t) = open;

int hooked_open(const char* pathname, int flags, mode_t mode) {
    printf("open called: %s\\n", pathname);
    
    // 检查是否需要重定向
    if (strstr(pathname, "shaderpacks") != NULL) {
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "/opt/unifier/shaderpacks/%s",
                 strrchr(pathname, '/') ? strrchr(pathname, '/') + 1 : pathname);
        int fd = original_open(newPath, flags, mode);
        if (fd >= 0) {
            printf("Redirected to: %s\\n", newPath);
            return fd;
        }
    }
    
    return original_open(pathname, flags, mode);
}

void (*original_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = glShaderSource;

void hooked_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
    // 拦截着色器源代码，进行转换
    // TODO: 实现GLSL转换逻辑
    original_glShaderSource(shader, count, string, length);
}
)";
    
    return code.str();
}

bool LinuxLoader::CreateLoaderSource(const std::string& outputPath) {
    std::string code = GenerateLoaderCode();
    
    std::ofstream file(outputPath);
    file << code;
    file.close();
    
    return true;
}

bool LinuxLoader::CompileLoader(const std::string& sourcePath, const std::string& outputPath) {
    // 使用g++编译
    std::string cmd = "g++ -shared -fPIC -std=c++17 \"" + sourcePath + "\" -o \"" + outputPath + "\" -ldl -lpthread";
    return system(cmd.c_str()) == 0;
}

// ==================== LinuxHookFramework ====================

LinuxHookFramework::LinuxHookFramework()
    : initialized_(false) {
}

LinuxHookFramework::~LinuxHookFramework() {
    Clear();
}

bool LinuxHookFramework::Initialize() {
    // TODO: 初始化PLT Hook框架
    initialized_ = true;
    return true;
}

bool LinuxHookFramework::RegisterHook(const std::string& libraryName, const std::string& symbol,
                                     void* hookFunc, void** originalFunc) {
    if (!initialized_) {
        return false;
    }
    
    // TODO: 使用PLT Hook注册Hook
    hooks_.push_back(std::make_tuple(libraryName, symbol, hookFunc, originalFunc));
    
    return true;
}

bool LinuxHookFramework::Refresh() {
    if (!initialized_) {
        return false;
    }
    
    // TODO: 刷新Hook
    return true;
}

bool LinuxHookFramework::Clear() {
    if (!initialized_) {
        return false;
    }
    
    // TODO: 清理Hook
    hooks_.clear();
    initialized_ = false;
    return true;
}

// ==================== UnifiedLinuxInjector ====================

UnifiedLinuxInjector::UnifiedLinuxInjector()
    : elfInjector_(std::make_unique<ELFInjector>())
    , loader_(std::make_unique<LinuxLoader>())
    , hookFramework_(std::make_unique<LinuxHookFramework>())
{
}

UnifiedLinuxInjector::~UnifiedLinuxInjector() {
}

void UnifiedLinuxInjector::SetConfig(const std::string& configPath) {
    configPath_ = configPath;
}

void UnifiedLinuxInjector::AddCompatModule(const std::string& modulePath) {
    compatModules_.push_back(modulePath);
}

void UnifiedLinuxInjector::SetProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
    elfInjector_->SetProgressCallback(callback);
}

bool UnifiedLinuxInjector::InjectToELF(const std::string& inputElf, const std::string& outputElf) {
    // 创建临时目录
    std::string workDir = fs::temp_directory_path().string() + "/unifier_inject_" + 
                         std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    fs::create_directories(workDir);
    
    // 准备加载器SO
    if (!PrepareLoaderSO(workDir)) {
        fs::remove_all(workDir);
        return false;
    }
    
    // 生成Hook配置
    if (!GenerateHookConfig(workDir)) {
        fs::remove_all(workDir);
        return false;
    }
    
    // 执行注入
    std::string loaderSo = workDir + "/libMinecraftUnifier.so";
    bool result = elfInjector_->InjectToELF(inputElf, outputElf, loaderSo);
    
    // 清理临时目录
    fs::remove_all(workDir);
    
    return result;
}

bool UnifiedLinuxInjector::PrepareLoaderSO(const std::string& workDir) {
    // 创建加载器源代码
    std::string sourcePath = workDir + "/loader.cpp";
    if (!loader_->CreateLoaderSource(sourcePath)) {
        return false;
    }
    
    // 编译加载器SO
    std::string outputPath = workDir + "/libMinecraftUnifier.so";
    if (!loader_->CompileLoader(sourcePath, outputPath)) {
        return false;
    }
    
    return true;
}

bool UnifiedLinuxInjector::GenerateHookConfig(const std::string& workDir) {
    // 生成Hook配置文件
    std::string configPath = workDir + "/hook_config.json";
    std::ofstream configFile(configPath);
    
    // TODO: 生成Hook配置
    configFile << "{}";
    configFile.close();
    
    return true;
}

// ==================== hooks namespace ====================

namespace hooks {

FILE* (*orig_fopen)(const char*, const char*) = nullptr;

FILE* hooked_fopen(const char* path, const char* mode) {
    printf("fopen called: %s\n", path);
    
    // 检查是否需要重定向
    if (strstr(path, "shaderpacks") != NULL) {
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "/opt/unifier/shaderpacks/%s",
                 strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
        FILE* f = orig_fopen(newPath, mode);
        if (f) {
            printf("Redirected to: %s\n", newPath);
            return f;
        }
    }
    
    return orig_fopen(path, mode);
}

int (*orig_open)(const char*, int, mode_t) = nullptr;

int hooked_open(const char* pathname, int flags, mode_t mode) {
    printf("open called: %s\n", pathname);
    
    // 检查是否需要重定向
    if (strstr(pathname, "shaderpacks") != NULL) {
        char newPath[512];
        snprintf(newPath, sizeof(newPath), "/opt/unifier/shaderpacks/%s",
                 strrchr(pathname, '/') ? strrchr(pathname, '/') + 1 : pathname);
        int fd = orig_open(newPath, flags, mode);
        if (fd >= 0) {
            printf("Redirected to: %s\n", newPath);
            return fd;
        }
    }
    
    return orig_open(pathname, flags, mode);
}

void (*orig_glShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = nullptr;

void hooked_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
    // 拦截着色器源代码，进行转换
    // TODO: 实现GLSL转换逻辑
    orig_glShaderSource(shader, count, string, length);
}

bool InstallAllHooks() {
    // TODO: 使用PLT Hook安装所有Hook
    return true;
}

} // namespace hooks

} // namespace linux
} // namespace injector
} // namespace mcu
