#include "vmoverlayapp.h"
#include <QDebug>
#include <QMessageBox>
#include <QIcon>

VMOverlayApp::VMOverlayApp(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
    , m_vmManager(nullptr)
    , m_overlayManager(nullptr)
    , m_waitingForRebuild(false)
{
    setApplicationName("VMOverlay");
    setApplicationVersion("1.0.0");
    setOrganizationName("VMOverlay");
}

VMOverlayApp::~VMOverlayApp()
{
}

void VMOverlayApp::initialize()
{
    // Check if system tray is available
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "VMOverlay",
            "系统托盘不可用。请确保您的桌面环境支持系统托盘。");
        quit();
        return;
    }
    
    // Create managers
    m_vmManager = new VMManager(this);
    m_overlayManager = new OverlayManager(this);
    
    // Connect signals
    connect(m_vmManager, &VMManager::vmShutdown,
            this, &VMOverlayApp::onVMShutdown);
    connect(m_overlayManager, &OverlayManager::commitFinished,
            this, &VMOverlayApp::onCommitFinished);
    connect(m_overlayManager, &OverlayManager::rebuildFinished,
            this, &VMOverlayApp::onRebuildFinished);
    
    // Create tray icon and menu
    createTrayIcon();
    createMenu();
    
    // Show tray icon
    m_trayIcon->show();
    
    // Show welcome message
    m_trayIcon->showMessage(
        "VMOverlay",
        "VMOverlay 已启动，正在监控虚拟机状态...",
        QSystemTrayIcon::Information,
        3000
    );
    
    // Start monitoring (you can configure VM name via command line or config file)
    QString vmName = "win10"; // Default VM name, can be configured
    if (arguments().size() > 1) {
        vmName = arguments().at(1);
    }
    
    m_vmManager->startMonitoring(vmName);
    
    // Configure overlay paths (these should be configured properly)
    QString overlayPath = "/var/lib/libvirt/images/overlay.qcow2";
    QString basePath = "/var/lib/libvirt/images/base.qcow2";
    
    // Check for command line arguments for paths
    for (int i = 1; i < arguments().size(); i++) {
        if (arguments().at(i) == "--overlay" && i + 1 < arguments().size()) {
            overlayPath = arguments().at(i + 1);
            i++;
        } else if (arguments().at(i) == "--base" && i + 1 < arguments().size()) {
            basePath = arguments().at(i + 1);
            i++;
        }
    }
    
    m_overlayManager->setOverlayPath(overlayPath);
    m_overlayManager->setBasePath(basePath);
    
    qDebug() << "VMOverlay initialized";
    qDebug() << "Monitoring VM:" << vmName;
    qDebug() << "Overlay path:" << overlayPath;
    qDebug() << "Base path:" << basePath;
}

void VMOverlayApp::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    
    // Load icon from resources
    QIcon icon(":/icon.svg");
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip("VMOverlay - 虚拟机 Overlay 管理");
    
    // Connect activation signal
    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &VMOverlayApp::onTrayIconActivated);
}

void VMOverlayApp::createMenu()
{
    m_trayMenu = new QMenu();
    
    QAction *statusAction = m_trayMenu->addAction("查看虚拟机状态");
    connect(statusAction, &QAction::triggered, this, &VMOverlayApp::showVMStatus);
    
    m_trayMenu->addSeparator();
    
    QAction *aboutAction = m_trayMenu->addAction("关于");
    connect(aboutAction, &QAction::triggered, this, &VMOverlayApp::showAbout);
    
    QAction *quitAction = m_trayMenu->addAction("退出");
    connect(quitAction, &QAction::triggered, this, &VMOverlayApp::quitApplication);
    
    m_trayIcon->setContextMenu(m_trayMenu);
}

void VMOverlayApp::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        showVMStatus();
    }
}

void VMOverlayApp::showVMStatus()
{
    QString status = m_vmManager->getVMStatus();
    QString title = "虚拟机状态";
    
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(status);
    msgBox.setIcon(m_vmManager->isVMRunning() ? 
                   QMessageBox::Information : QMessageBox::Warning);
    msgBox.exec();
}

void VMOverlayApp::onVMShutdown()
{
    qDebug() << "VM shutdown detected, asking for commit";
    
    // Show notification
    m_trayIcon->showMessage(
        "虚拟机已关闭",
        "检测到虚拟机已关闭",
        QSystemTrayIcon::Information,
        3000
    );
    
    // Ask user if they want to commit overlay
    askCommitOverlay();
}

void VMOverlayApp::askCommitOverlay()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("提交 Overlay");
    msgBox.setText("虚拟机已关闭。是否要将 Overlay 的内容提交到 Base 镜像？");
    msgBox.setInformativeText("选择后将重建 Overlay。");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);
    
    int ret = msgBox.exec();
    
    if (ret == QMessageBox::Yes) {
        qDebug() << "User chose to commit overlay";
        m_waitingForRebuild = true;
        
        // Show progress message
        m_trayIcon->showMessage(
            "提交 Overlay",
            "正在提交 Overlay 到 Base 镜像...",
            QSystemTrayIcon::Information,
            3000
        );
        
        m_overlayManager->commitOverlay();
    } else {
        qDebug() << "User chose not to commit overlay";
        // Rebuild overlay directly
        m_waitingForRebuild = false;
        
        m_trayIcon->showMessage(
            "重建 Overlay",
            "正在重建 Overlay...",
            QSystemTrayIcon::Information,
            3000
        );
        
        m_overlayManager->rebuildOverlay();
    }
}

void VMOverlayApp::onCommitFinished(bool success, const QString &message)
{
    qDebug() << "Commit finished:" << success << message;
    
    if (success) {
        m_trayIcon->showMessage(
            "提交成功",
            "Overlay 已成功提交到 Base 镜像",
            QSystemTrayIcon::Information,
            3000
        );
        
        // Now rebuild overlay
        m_trayIcon->showMessage(
            "重建 Overlay",
            "正在重建 Overlay...",
            QSystemTrayIcon::Information,
            3000
        );
        
        m_overlayManager->rebuildOverlay();
    } else {
        m_trayIcon->showMessage(
            "提交失败",
            "提交 Overlay 失败: " + message,
            QSystemTrayIcon::Critical,
            5000
        );
        
        // Ask if user still wants to rebuild
        QMessageBox msgBox;
        msgBox.setWindowTitle("提交失败");
        msgBox.setText("提交 Overlay 失败: " + message);
        msgBox.setInformativeText("是否仍要重建 Overlay？");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setIcon(QMessageBox::Warning);
        
        if (msgBox.exec() == QMessageBox::Yes) {
            m_overlayManager->rebuildOverlay();
        }
    }
}

void VMOverlayApp::onRebuildFinished(bool success, const QString &message)
{
    qDebug() << "Rebuild finished:" << success << message;
    
    if (success) {
        m_trayIcon->showMessage(
            "重建成功",
            "Overlay 已成功重建",
            QSystemTrayIcon::Information,
            3000
        );
    } else {
        m_trayIcon->showMessage(
            "重建失败",
            "重建 Overlay 失败: " + message,
            QSystemTrayIcon::Critical,
            5000
        );
    }
    
    m_waitingForRebuild = false;
}

void VMOverlayApp::showAbout()
{
    QMessageBox::about(nullptr, "关于 VMOverlay",
        "<h2>VMOverlay 1.0.0</h2>"
        "<p>虚拟机 Overlay 管理工具</p>"
        "<p>用于管理 libvirt 虚拟机的 qcow2 overlay 镜像。</p>"
        "<p>功能：</p>"
        "<ul>"
        "<li>监控虚拟机运行状态</li>"
        "<li>虚拟机关闭后提示是否提交 overlay</li>"
        "<li>自动重建 overlay</li>"
        "</ul>"
    );
}

void VMOverlayApp::quitApplication()
{
    qDebug() << "Quitting application";
    quit();
}
