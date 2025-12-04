# libvirt 配置示例

本文档说明如何配置 libvirt 虚拟机使用 overlay 镜像。

## 背景

Overlay 镜像（也称为 backing file 或 COW - Copy-On-Write）允许您：
- 保持一个只读的 base 镜像
- 所有修改写入 overlay 镜像
- 可以选择性地提交更改到 base
- 可以轻松丢弃更改并重新开始

## 创建 Overlay 镜像

### 1. 准备 Base 镜像

```bash
# 假设您已有一个虚拟机镜像
BASE_IMAGE="/var/lib/libvirt/images/windows10.qcow2"

# 确保 base 镜像是 qcow2 格式
qemu-img info "$BASE_IMAGE"
```

### 2. 创建 Overlay

```bash
# 创建 overlay 镜像
OVERLAY_IMAGE="/var/lib/libvirt/images/windows10-overlay.qcow2"

qemu-img create -f qcow2 \
  -F qcow2 \
  -b "$BASE_IMAGE" \
  "$OVERLAY_IMAGE"

# 验证创建成功
qemu-img info "$OVERLAY_IMAGE"
```

输出应该显示：
```
backing file: /var/lib/libvirt/images/windows10.qcow2
backing file format: qcow2
```

## 配置 libvirt 虚拟机

### 方式 1: 使用 virsh edit

```bash
# 编辑虚拟机配置
virsh edit windows10

# 找到 <disk> 部分，修改 <source> 指向 overlay
```

修改前：
```xml
<disk type='file' device='disk'>
  <driver name='qemu' type='qcow2'/>
  <source file='/var/lib/libvirt/images/windows10.qcow2'/>
  <target dev='vda' bus='virtio'/>
</disk>
```

修改后：
```xml
<disk type='file' device='disk'>
  <driver name='qemu' type='qcow2'/>
  <source file='/var/lib/libvirt/images/windows10-overlay.qcow2'/>
  <target dev='vda' bus='virtio'/>
</disk>
```

### 方式 2: 创建新虚拟机

```bash
# 克隆现有虚拟机配置
virsh dumpxml windows10 > windows10-overlay.xml

# 编辑 XML 文件
nano windows10-overlay.xml

# 修改：
# 1. 虚拟机名称 <name>
# 2. UUID（删除，libvirt 会自动生成）
# 3. 磁盘路径指向 overlay

# 导入新虚拟机
virsh define windows10-overlay.xml
```

## 完整示例：从头设置

```bash
#!/bin/bash

# 配置变量
VM_NAME="windows10"
BASE_IMAGE="/var/lib/libvirt/images/windows10-base.qcow2"
OVERLAY_IMAGE="/var/lib/libvirt/images/windows10-overlay.qcow2"

# 1. 停止虚拟机（如果正在运行）
virsh shutdown "$VM_NAME" 2>/dev/null || true
sleep 5

# 2. 备份原始镜像作为 base（如果还没有）
if [ ! -f "${BASE_IMAGE}" ]; then
    echo "Moving original image to base..."
    mv "/var/lib/libvirt/images/${VM_NAME}.qcow2" "$BASE_IMAGE"
fi

# 3. 创建 overlay
echo "Creating overlay..."
qemu-img create -f qcow2 -F qcow2 -b "$BASE_IMAGE" "$OVERLAY_IMAGE"

# 4. 更新虚拟机配置
echo "Updating VM configuration..."
virsh dumpxml "$VM_NAME" | \
  sed "s|${BASE_IMAGE}|${OVERLAY_IMAGE}|g" | \
  virsh define /dev/stdin

# 5. 启动 VMOverlay 监控
echo "Starting VMOverlay..."
VMOverlay "$VM_NAME" \
  --overlay "$OVERLAY_IMAGE" \
  --base "$BASE_IMAGE" &

echo "Setup complete!"
echo "You can now start the VM: virsh start $VM_NAME"
```

## Overlay 操作

### 查看 Overlay 大小

```bash
# 查看实际占用空间
qemu-img info /var/lib/libvirt/images/windows10-overlay.qcow2

# 查看文件大小
ls -lh /var/lib/libvirt/images/windows10-overlay.qcow2
```

### 提交 Overlay 到 Base

```bash
# VMOverlay 会自动处理，或手动执行：
qemu-img commit /var/lib/libvirt/images/windows10-overlay.qcow2
```

### 重建 Overlay

```bash
# VMOverlay 会自动处理，或手动执行：
rm /var/lib/libvirt/images/windows10-overlay.qcow2
qemu-img create -f qcow2 -F qcow2 \
  -b /var/lib/libvirt/images/windows10-base.qcow2 \
  /var/lib/libvirt/images/windows10-overlay.qcow2
```

### 丢弃 Overlay 修改

```bash
# 简单重建即可丢弃所有修改
virsh shutdown windows10
rm /var/lib/libvirt/images/windows10-overlay.qcow2
qemu-img create -f qcow2 -F qcow2 \
  -b /var/lib/libvirt/images/windows10-base.qcow2 \
  /var/lib/libvirt/images/windows10-overlay.qcow2
virsh start windows10
```

## 高级用法

### 多层 Overlay

```bash
# 可以创建多层 overlay
BASE="/var/lib/libvirt/images/base.qcow2"
OVERLAY1="/var/lib/libvirt/images/overlay1.qcow2"
OVERLAY2="/var/lib/libvirt/images/overlay2.qcow2"

qemu-img create -f qcow2 -F qcow2 -b "$BASE" "$OVERLAY1"
qemu-img commit "$OVERLAY1"  # 提交到 base
qemu-img create -f qcow2 -F qcow2 -b "$BASE" "$OVERLAY2"
```

### 快照链查看

```bash
# 查看 backing file 链
qemu-img info --backing-chain /var/lib/libvirt/images/overlay.qcow2
```

## 注意事项

1. **Base 镜像保护**: 
   - Base 镜像应该是只读的
   - 可以设置文件权限：`chmod 444 base.qcow2`

2. **路径问题**:
   - 使用绝对路径
   - 不要移动 base 或 overlay 文件

3. **备份**:
   - 定期备份 base 镜像
   - Overlay 可以随时重建

4. **性能**:
   - Overlay 会有轻微性能影响
   - 定期提交可以减少 overlay 大小

5. **权限**:
   - 确保 qemu/libvirt 用户有访问权限
   - 通常是 libvirt-qemu:kvm

## 故障排除

### 问题: 虚拟机启动失败

```bash
# 检查镜像完整性
qemu-img check /var/lib/libvirt/images/overlay.qcow2

# 检查 backing file 路径
qemu-img info /var/lib/libvirt/images/overlay.qcow2
```

### 问题: Backing file 路径错误

```bash
# 重新指定 backing file（慎用！）
qemu-img rebase -u -b /correct/path/to/base.qcow2 overlay.qcow2
```

### 问题: 权限被拒绝

```bash
# 检查文件所有者
ls -la /var/lib/libvirt/images/

# 修正权限
sudo chown libvirt-qemu:kvm /var/lib/libvirt/images/*.qcow2
sudo chmod 644 /var/lib/libvirt/images/*.qcow2
```

## 参考资料

- [QEMU 文档 - qemu-img](https://qemu.readthedocs.io/en/latest/tools/qemu-img.html)
- [libvirt 文档 - Storage](https://libvirt.org/storage.html)
- [VMOverlay GitHub](https://github.com/Ethanol14514/VMOverlay)
