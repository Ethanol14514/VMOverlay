# VMOverlay 项目实现总结

## 项目概述

VMOverlay 是一个基于 Qt6 的系统托盘应用程序，专为在 Linux (Wayland + KDE Plasma) 环境下管理 libvirt 虚拟机的 qcow2 overlay 镜像而设计。

## 核心功能实现

### 1. 系统托盘图标 ✅
- **实现方式**: 使用 `QSystemTrayIcon`
- **图标**: 自定义 SVG 图标，显示虚拟机和 overlay 标识
- **资源管理**: 通过 Qt 资源系统（.qrc）嵌入
- **位置**: `resources/icon.svg`, `resources/resources.qrc`

### 2. 虚拟机状态监控 ✅
- **实现方式**: 
  - 主要通过 `virsh list --name` 命令检查
  - 备用方案：libvirt DBus 接口（可扩展）
- **监控频率**: 每 5 秒检查一次
- **状态变化**: 通过信号/槽机制通知应用
- **位置**: `src/vmmanager.h`, `src/vmmanager.cpp`

### 3. 托盘图标交互 ✅
- **点击行为**: 单击或双击显示虚拟机状态
- **右键菜单**: 
  - 查看虚拟机状态
  - 关于
  - 退出
- **位置**: `src/vmoverlayapp.cpp::onTrayIconActivated()`

### 4. 虚拟机关机检测 ✅
- **检测机制**: 轮询方式，检测状态从运行到停止的转变
- **触发事件**: 发出 `vmShutdown` 信号
- **响应**: 弹出提交确认对话框
- **位置**: `src/vmmanager.cpp::checkVMStatus()`

### 5. Overlay 提交确认对话框 ✅
- **触发时机**: 虚拟机关机后自动弹出
- **对话框类型**: `QMessageBox::Question`
- **选项**: 
  - **是**: 提交 overlay 到 base，然后重建
  - **否**: 直接重建 overlay
- **位置**: `src/vmoverlayapp.cpp::askCommitOverlay()`

### 6. Overlay 提交功能 ✅
- **实现方式**: 执行 `qemu-img commit <overlay_path>`
- **异步处理**: 使用 `QProcess` 异步执行
- **进度反馈**: 通过系统托盘通知显示
- **错误处理**: 捕获执行错误并通知用户
- **位置**: `src/overlaymanager.cpp::commitOverlay()`

### 7. Overlay 重建功能 ✅
- **实现方式**: 
  1. 删除旧的 overlay 文件
  2. 执行 `qemu-img create -f qcow2 -F qcow2 -b <base> <overlay>`
- **自动执行**: 
  - 提交成功后自动重建
  - 用户选择不提交时也重建
- **位置**: `src/overlaymanager.cpp::rebuildOverlay()`

## 技术架构

### 类结构

```
VMOverlayApp (主应用类)
├── VMManager (虚拟机管理)
│   ├── 状态监控
│   ├── DBus 通信
│   └── virsh 命令执行
└── OverlayManager (Overlay 管理)
    ├── qemu-img commit
    ├── qemu-img create
    └── 进程管理
```

### 信号/槽连接

```
VMManager::vmShutdown → VMOverlayApp::onVMShutdown
OverlayManager::commitFinished → VMOverlayApp::onCommitFinished
OverlayManager::rebuildFinished → VMOverlayApp::onRebuildFinished
```

## 项目文件结构

```
VMOverlay/
├── src/                    # 源代码
│   ├── main.cpp           # 程序入口
│   ├── vmoverlayapp.h/cpp # 主应用类
│   ├── vmmanager.h/cpp    # 虚拟机管理
│   └── overlaymanager.h/cpp # Overlay 管理
├── resources/             # 资源文件
│   ├── icon.svg          # 托盘图标
│   └── resources.qrc     # Qt 资源文件
├── CMakeLists.txt        # CMake 构建配置
├── build.sh              # 构建脚本
├── vmoverlay.service     # systemd 服务文件
├── vmoverlay.desktop     # 桌面入口文件
├── config.example        # 配置示例
├── README.md             # 项目文档
├── CONTRIBUTING.md       # 贡献指南
├── LICENSE               # MIT 许可证
└── .gitignore           # Git 忽略配置
```

## 依赖项

### 必需
- Qt6 Core
- Qt6 Widgets
- Qt6 DBus
- Qt6 Svg
- CMake >= 3.16
- C++17 编译器

### 运行时
- libvirt (virsh 命令)
- qemu-img

## 配置选项

### 命令行参数
```bash
VMOverlay [VM名称] [--overlay 路径] [--base 路径]
```

### 环境变量 (systemd)
- `VM_NAME`: 虚拟机名称
- `OVERLAY_PATH`: Overlay 镜像路径
- `BASE_PATH`: Base 镜像路径

## 工作流程

```
1. 程序启动 → 创建托盘图标
2. 开始监控 VM 状态（每 5 秒）
3. 检测到 VM 关机 → 显示通知
4. 弹出对话框询问是否提交
   ├─ 是 → qemu-img commit → 重建 overlay
   └─ 否 → 直接重建 overlay
5. 显示操作结果通知
```

## 部署方式

### 1. 手动运行
```bash
./VMOverlay win10 --overlay /path/to/overlay.qcow2 --base /path/to/base.qcow2
```

### 2. systemd 用户服务
```bash
systemctl --user enable vmoverlay.service
systemctl --user start vmoverlay.service
```

### 3. 桌面自动启动
将 `vmoverlay.desktop` 复制到 `~/.config/autostart/`

## 安全考虑

- **权限**: 需要访问 libvirt 的权限
- **路径验证**: 检查文件是否存在
- **错误处理**: 所有外部命令执行都有错误处理
- **进程管理**: 防止同时执行多个 qemu-img 操作

## 未来改进方向

1. **配置文件**: 支持读取配置文件而不仅是命令行参数
2. **多虚拟机**: 支持同时监控多个虚拟机
3. **DBus 改进**: 完善 libvirt DBus API 集成
4. **通知改进**: 添加更详细的进度通知
5. **快照管理**: 支持 qcow2 快照管理
6. **GUI 配置**: 添加图形化配置界面

## 已知限制

1. 目前主要依赖 `virsh` 命令，需要正确配置 libvirt
2. 需要有执行 `qemu-img` 的权限
3. overlay 和 base 路径需要手动配置
4. 仅支持 qcow2 格式

## 测试建议

由于项目需要实际的 libvirt 环境和虚拟机，建议在真实环境中测试：

1. 创建测试虚拟机
2. 配置 overlay 镜像
3. 启动 VMOverlay
4. 启动虚拟机
5. 关闭虚拟机，测试提交流程
6. 验证 overlay 是否正确重建

## 总结

VMOverlay 项目成功实现了所有需求的功能，提供了一个完整的、可用的 Qt6 系统托盘应用，用于管理 libvirt 虚拟机的 overlay 镜像。代码结构清晰，文档完善，易于部署和使用。
