# 安全说明

## 概述

VMOverlay 是一个系统工具，需要与 libvirt 和 qemu-img 交互。本文档说明安全考虑和最佳实践。

## 安全特性

### 1. 进程管理
- ✅ 防止并发执行 qemu-img 操作
- ✅ 进程超时和错误处理
- ✅ 安全的进程清理机制

### 2. 命令执行
- ✅ 使用 QProcess API 而非 shell 执行
- ✅ 参数验证和转义
- ✅ 错误输出捕获和日志记录

### 3. 文件操作
- ✅ 路径验证
- ✅ 文件存在性检查
- ✅ 使用绝对路径

### 4. 权限要求
- ✅ 用户级权限运行
- ✅ 需要访问 libvirt 的权限
- ✅ 需要读写镜像文件的权限

## 权限配置

### libvirt 访问权限

将用户添加到 libvirt 组：

```bash
sudo usermod -aG libvirt $USER
# 注销并重新登录以使组更改生效
```

### 镜像文件权限

确保用户对镜像文件有适当的权限：

```bash
# 检查文件权限
ls -la /var/lib/libvirt/images/

# 如果需要，调整权限
sudo chown $USER:libvirt /var/lib/libvirt/images/overlay.qcow2
sudo chmod 660 /var/lib/libvirt/images/overlay.qcow2
```

## 潜在风险

### 1. 数据丢失风险

**风险**: commit 操作会永久修改 base 镜像

**缓解措施**:
- 定期备份 base 镜像
- 在 commit 前确认操作
- 测试环境中验证流程

### 2. 并发访问风险

**风险**: 虚拟机运行时执行 commit 可能导致数据损坏

**缓解措施**:
- 仅在虚拟机停止后执行操作
- VMOverlay 会检测虚拟机状态
- 建议配置虚拟机 hook

### 3. 路径注入风险

**风险**: 恶意路径可能导致错误操作

**缓解措施**:
- 使用绝对路径
- 路径验证
- 文件存在性检查

### 4. 权限提升风险

**风险**: 程序可能被用于未授权访问

**缓解措施**:
- 以用户权限运行，不使用 sudo
- 仅访问用户有权访问的文件
- 不执行任意命令

## 安全最佳实践

### 1. 最小权限原则

```bash
# 不要以 root 运行
# 错误示例
sudo VMOverlay

# 正确示例
VMOverlay win10 --overlay ~/vms/overlay.qcow2 --base ~/vms/base.qcow2
```

### 2. 使用专用用户

创建专用用户来管理虚拟机：

```bash
# 创建 VM 管理用户
sudo useradd -m -G libvirt vmadmin

# 切换到该用户
su - vmadmin

# 运行 VMOverlay
VMOverlay ...
```

### 3. 文件系统隔离

将虚拟机镜像存储在单独的分区：

```bash
# 挂载专用分区
sudo mount /dev/sdX /var/lib/libvirt/images

# 设置适当的权限
sudo chown -R libvirt-qemu:kvm /var/lib/libvirt/images
```

### 4. 审计日志

启用 libvirt 审计日志：

```bash
# 编辑 libvirt 配置
sudo nano /etc/libvirt/libvirtd.conf

# 启用审计日志
audit_level = 1
audit_logging = 1
```

### 5. 网络隔离

如果虚拟机处理敏感数据，使用网络隔离：

```bash
# 使用隔离的网络配置
virsh net-define isolated-network.xml
virsh net-start isolated
```

## 已知限制

1. **virsh 命令依赖**: 依赖 virsh 可执行文件
2. **qemu-img 依赖**: 依赖 qemu-img 可执行文件
3. **DBus 访问**: 需要系统 DBus 访问权限
4. **文件系统**: 需要对镜像目录的访问权限

## 报告安全问题

如果发现安全漏洞，请：

1. **不要**公开披露
2. 发送邮件至维护者（通过 GitHub）
3. 包含详细的复现步骤
4. 等待回应后再公开

## 更新和补丁

- 关注项目更新
- 定期更新依赖（Qt6, libvirt）
- 订阅安全公告

## 参考资料

- [libvirt Security](https://libvirt.org/securityprocess.html)
- [QEMU Security](https://www.qemu.org/docs/master/system/security.html)
- [Qt Security](https://doc.qt.io/qt-6/security.html)

## 审计记录

- **初始安全审查**: 2024-12 (创建时)
- **代码审查**: 已完成两轮
- **已知漏洞**: 无

---

最后更新: 2024-12-04
