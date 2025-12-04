# VMOverlay

虚拟机 Overlay 管理工具 - 一个运行在 Linux (Wayland + KDE Plasma) 环境下的 Qt6 托盘常驻程序，用于与 libvirt 虚拟机的关机 hook 交互。

## 功能特性

- 🖥️ 系统托盘图标，常驻后台运行
- 📊 点击托盘图标查看虚拟机运行状态
- 🔄 自动检测虚拟机关机事件
- 💾 虚拟机关机后询问是否提交 overlay 到 base 镜像
- 🔨 自动重建 overlay 镜像

## 技术栈

- Qt6 (Core, Widgets, DBus, Svg)
- C++17
- CMake 构建系统
- libvirt/qemu-img

## 编译要求

- Qt6 (>= 6.0)
- CMake (>= 3.16)
- C++ 编译器支持 C++17
- libvirt
- qemu-img

### Ubuntu/Debian 安装依赖

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-svg-dev libqt6dbus6 libvirt-clients qemu-utils
```

### Arch Linux 安装依赖

```bash
sudo pacman -S base-devel cmake qt6-base qt6-svg libvirt qemu-img
```

## 编译步骤

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make

# 可选：安装
sudo make install
```

## 使用方法

### 基本用法

```bash
./VMOverlay [虚拟机名称] [选项]
```

### 命令行参数

- 第一个参数：虚拟机名称 (默认: win10)
- `--overlay <路径>`: Overlay 镜像路径 (默认: /var/lib/libvirt/images/overlay.qcow2)
- `--base <路径>`: Base 镜像路径 (默认: /var/lib/libvirt/images/base.qcow2)

### 示例

```bash
# 使用默认配置监控名为 win10 的虚拟机
./VMOverlay win10

# 指定完整路径
./VMOverlay myvm \
  --overlay /path/to/overlay.qcow2 \
  --base /path/to/base.qcow2
```

## 工作流程

1. **启动程序**：VMOverlay 作为系统托盘应用启动
2. **监控状态**：每 5 秒检查一次虚拟机运行状态
3. **检测关机**：当检测到虚拟机从运行状态变为停止状态时
4. **询问提交**：弹出对话框询问是否要提交 overlay 到 base
   - **选择"是"**：执行 `qemu-img commit` 将 overlay 合并到 base，然后重建 overlay
   - **选择"否"**：直接重建 overlay
5. **重建完成**：显示操作结果通知

## 托盘菜单

- **查看虚拟机状态**：显示当前虚拟机运行状态
- **关于**：显示程序信息
- **退出**：退出程序

## 配置 libvirt Hook（可选）

如果希望通过 libvirt hook 触发，可以创建 hook 脚本：

```bash
sudo mkdir -p /etc/libvirt/hooks
sudo nano /etc/libvirt/hooks/qemu
```

添加以下内容：

```bash
#!/bin/bash
VM_NAME="$1"
OPERATION="$2"

if [ "$OPERATION" = "stopped" ]; then
    # 通知 VMOverlay（通过 DBus 或其他机制）
    logger "VM $VM_NAME stopped"
fi
```

设置可执行权限：

```bash
sudo chmod +x /etc/libvirt/hooks/qemu
sudo systemctl restart libvirtd
```

## 注意事项

- 需要有足够的权限访问 libvirt 和执行 qemu-img 命令
- 确保 overlay 和 base 镜像路径正确
- 建议在执行 commit 操作前备份重要数据

## 许可证

本项目使用 MIT 许可证。

## 贡献

欢迎提交 Issue 和 Pull Request！
