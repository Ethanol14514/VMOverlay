# VMOverlay 快速开始指南

## 快速安装（Ubuntu/Debian）

```bash
# 1. 安装依赖
sudo apt update
sudo apt install -y build-essential cmake qt6-base-dev qt6-svg-dev libvirt-clients qemu-utils

# 2. 克隆仓库
git clone https://github.com/Ethanol14514/VMOverlay.git
cd VMOverlay

# 3. 编译
./build.sh

# 4. 测试运行
./build/VMOverlay
```

## 快速配置

### 基本配置

```bash
# 准备镜像文件
BASE_IMAGE="/var/lib/libvirt/images/base.qcow2"
OVERLAY_IMAGE="/var/lib/libvirt/images/overlay.qcow2"

# 如果 overlay 不存在，创建它
qemu-img create -f qcow2 -F qcow2 -b "$BASE_IMAGE" "$OVERLAY_IMAGE"

# 在 libvirt 中配置虚拟机使用 overlay 镜像
virsh edit your-vm-name
# 修改 <disk> 配置指向 overlay 镜像
```

### 运行 VMOverlay

```bash
# 方式 1: 直接运行
./build/VMOverlay your-vm-name \
  --overlay /var/lib/libvirt/images/overlay.qcow2 \
  --base /var/lib/libvirt/images/base.qcow2

# 方式 2: 安装为系统服务
sudo cp build/VMOverlay /usr/local/bin/
cp vmoverlay.service ~/.config/systemd/user/

# 编辑服务文件配置路径
nano ~/.config/systemd/user/vmoverlay.service

# 启用并启动
systemctl --user daemon-reload
systemctl --user enable vmoverlay.service
systemctl --user start vmoverlay.service
```

## 使用流程

1. **启动 VMOverlay**
   - 托盘区会出现 VMOverlay 图标
   - 显示欢迎通知

2. **查看虚拟机状态**
   - 点击托盘图标
   - 或右键菜单选择"查看虚拟机状态"

3. **虚拟机关机时**
   - 自动弹出提示
   - 选择是否提交 overlay
   - 自动重建 overlay

## 常见问题

### Q: 托盘图标不显示？
A: 确保您的桌面环境支持系统托盘。KDE Plasma 默认支持。

### Q: 提示找不到 virsh？
A: 安装 libvirt-clients：`sudo apt install libvirt-clients`

### Q: 提示权限错误？
A: 将用户添加到 libvirt 组：`sudo usermod -aG libvirt $USER`

### Q: qemu-img 命令失败？
A: 检查镜像路径是否正确，是否有写权限

### Q: 如何配置多个虚拟机？
A: 当前版本一个实例监控一个虚拟机。可以运行多个实例。

## 调试

### 查看日志
```bash
# 如果使用 systemd
journalctl --user -u vmoverlay.service -f

# 如果直接运行
# 输出会显示在终端
```

### 测试 virsh 连接
```bash
# 列出所有虚拟机
virsh list --all

# 测试特定虚拟机
virsh list --name | grep your-vm-name
```

### 测试 qemu-img
```bash
# 查看镜像信息
qemu-img info /path/to/overlay.qcow2

# 手动提交测试
qemu-img commit /path/to/overlay.qcow2

# 手动重建测试
qemu-img create -f qcow2 -F qcow2 -b base.qcow2 overlay.qcow2
```

## 下一步

- 阅读 [README.md](README.md) 了解详细功能
- 阅读 [IMPLEMENTATION.md](IMPLEMENTATION.md) 了解技术细节
- 查看 [CONTRIBUTING.md](CONTRIBUTING.md) 参与贡献

## 需要帮助？

在 GitHub 上提交 Issue: https://github.com/Ethanol14514/VMOverlay/issues
