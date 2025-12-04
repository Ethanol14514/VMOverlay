# 贡献指南

感谢您对 VMOverlay 项目的关注！

## 如何贡献

### 报告问题

如果您发现 bug 或有功能建议，请：

1. 检查 [Issues](https://github.com/Ethanol14514/VMOverlay/issues) 确保问题未被报告
2. 创建新的 Issue，详细描述问题或建议
3. 包含相关的系统信息（操作系统、Qt 版本等）

### 提交代码

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

### 代码规范

- 遵循现有的代码风格
- 添加必要的注释
- 更新相关文档
- 确保代码可以编译通过

### 测试

- 在提交 PR 前测试您的更改
- 包含必要的测试用例
- 确保不会破坏现有功能

## 开发环境设置

```bash
# 克隆仓库
git clone https://github.com/Ethanol14514/VMOverlay.git
cd VMOverlay

# 安装依赖（Ubuntu/Debian）
sudo apt install qt6-base-dev qt6-svg-dev libvirt-clients qemu-utils

# 构建项目
./build.sh
```

## 许可证

通过贡献代码，您同意您的贡献将在 MIT 许可证下授权。
