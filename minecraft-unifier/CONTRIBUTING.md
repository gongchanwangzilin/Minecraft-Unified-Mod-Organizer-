# 贡献指南

感谢您对"我的世界统一器"项目的关注！我们欢迎任何形式的贡献。

## 如何贡献

### 报告问题

如果您发现了bug或有功能建议，请：

1. 在[Issues](https://github.com/your-repo/minecraft-unifier/issues)中搜索是否已有类似问题
2. 如果没有，创建新的Issue，包含：
   - 清晰的标题
   - 详细的描述
   - 复现步骤（如果是bug）
   - 预期行为
   - 实际行为
   - 环境信息（操作系统、编译器版本等）
   - 相关日志或截图

### 提交代码

1. **Fork项目**
   ```bash
   # 在GitHub上点击Fork按钮
   git clone https://github.com/your-username/minecraft-unifier.git
   cd minecraft-unifier
   ```

2. **创建分支**
   ```bash
   git checkout -b feature/your-feature-name
   # 或
   git checkout -b fix/your-bug-fix
   ```

3. **进行修改**
   - 遵循代码规范（见下方）
   - 添加必要的测试
   - 更新相关文档

4. **提交更改**
   ```bash
   git add .
   git commit -m "feat: add your feature description"
   # 或
   git commit -m "fix: describe the bug fix"
   ```

5. **推送到分支**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **创建Pull Request**
   - 在GitHub上创建Pull Request
   - 填写PR模板
   - 等待代码审查

## 代码规范

### 命名约定

- **类名**：PascalCase（如 `ShaderConverter`）
- **函数名**：PascalCase（如 `LoadJavaShaderpack`）
- **变量名**：camelCase（如 `shaderpackPath`）
- **常量名**：UPPER_CASE（如 `MAX_SHADER_COUNT`）
- **命名空间**：lowercase（如 `mcu::core::render`）

### 文件组织

- 每个类一个头文件和一个实现文件
- 头文件使用 `.h` 扩展名
- 实现文件使用 `.cpp` 扩展名
- 头文件包含保护使用 `#pragma once`

### 注释规范

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

### 提交信息规范

使用[Conventional Commits](https://www.conventionalcommits.org/)格式：

- `feat:` 新功能
- `fix:` 修复bug
- `docs:` 文档更新
- `style:` 代码格式调整（不影响功能）
- `refactor:` 重构
- `test:` 测试相关
- `chore:` 构建/工具链相关

示例：
```
feat: add support for Java mod API mapping

- Implement API mapping layer
- Add unit tests for mapping functions
- Update documentation
```

## 测试要求

### 单元测试

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

### 集成测试

- 测试跨模块功能
- 测试平台特定功能

```cpp
TEST(AndroidInjectorTest, InjectToApk) {
    mcu::injector::android::UnifiedAndroidInjector injector;
    
    injector.AddCompatModule("./libRenderCompat.so");
    injector.AddCompatModule("./libModRuntime.so");
    
    ASSERT_TRUE(injector.InjectToApk("./test_input.apk", "./test_output.apk"));
}
```

## 代码审查

### 审查清单

提交PR前，请确保：

- [ ] 代码符合项目规范
- [ ] 添加了必要的测试
- [ ] 更新了相关文档
- [ ] 所有测试通过
- [ ] 没有引入新的警告
- [ ] 提交信息清晰规范

### 审查流程

1. 自动化检查（CI/CD）
2. 维护者审查
3. 根据反馈修改
4. 审查通过后合并

## 行为准则

### 我们的承诺

为了营造开放和友好的环境，我们承诺：

- 尊重不同的观点和经验
- 优雅地接受建设性批评
- 关注对社区最有利的事情
- 对其他社区成员表示同理心

### 不可接受的行为

- 使用性化的语言或图像
- 恶意评论、人身攻击或政治攻击
- 公开或私下骚扰
- 未经许可发布他人的私人信息
- 其他不专业或不恰当的行为

## 获取帮助

如果您有任何问题：

- 查看[文档](TECHNICAL_DOCUMENTATION.md)
- 联系维护者：jqyh1026@outlook.com

## 许可证

通过贡献代码，您同意您的贡献将在MIT许可证下发布。

---

再次感谢您的贡献！
