#include "vmoverlayapp.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    // Enable high DPI support
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    VMOverlayApp app(argc, argv);
    
    // Initialize the application
    app.initialize();
    
    // Print usage information
    qDebug() << "VMOverlay - 虚拟机 Overlay 管理工具";
    qDebug() << "用法: VMOverlay [VM名称] [--overlay overlay路径] [--base base路径]";
    qDebug() << "示例: VMOverlay win10 --overlay /var/lib/libvirt/images/overlay.qcow2 --base /var/lib/libvirt/images/base.qcow2";
    
    return app.exec();
}
