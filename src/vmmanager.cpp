#include "vmmanager.h"
#include <QDebug>
#include <QDBusReply>
#include <QTimer>
#include <QProcess>

VMManager::VMManager(QObject *parent)
    : QObject(parent)
    , m_lastRunningState(false)
    , m_systemBus(QDBusConnection::systemBus())
    , m_serviceWatcher(nullptr)
{
}

VMManager::~VMManager()
{
}

void VMManager::startMonitoring(const QString &vmName)
{
    m_vmName = vmName;
    
    // Set up service watcher for libvirt
    m_serviceWatcher = new QDBusServiceWatcher(
        "org.libvirt",
        m_systemBus,
        QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
        this
    );
    
    // Initial status check
    checkVMStatus();
    
    // Set up periodic checking (every 5 seconds)
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VMManager::checkVMStatus);
    timer->start(5000);
}

bool VMManager::isVMRunning() const
{
    return m_lastRunningState;
}

QString VMManager::getVMStatus() const
{
    if (m_vmName.isEmpty()) {
        return "未配置虚拟机";
    }
    return m_lastRunningState ? "虚拟机运行中" : "虚拟机已停止";
}

void VMManager::checkVMStatus()
{
    bool currentState = queryVMState();
    
    if (currentState != m_lastRunningState) {
        m_lastRunningState = currentState;
        emit vmStatusChanged(currentState);
        
        // If VM just shutdown, emit shutdown signal
        if (!currentState) {
            qDebug() << "VM shutdown detected";
            emit vmShutdown();
        }
    }
}

bool VMManager::queryVMState()
{
    if (m_vmName.isEmpty()) {
        return false;
    }
    
    // Try to query libvirt via DBus
    // Note: libvirt DBus API is complex and may vary by version
    // We primarily rely on the virsh fallback method for reliability
    QDBusInterface interface(
        "org.libvirt",
        "/org/libvirt/QEMU",
        "org.libvirt.Connect",
        m_systemBus
    );
    
    if (!interface.isValid()) {
        qDebug() << "Failed to connect to libvirt DBus interface, using virsh fallback";
        // Fallback: use virsh command which is more reliable
        return queryVMStateViaCommand();
    }
    
    // For now, use virsh as primary method since DBus API is complex
    return queryVMStateViaCommand();
}

bool VMManager::queryVMStateViaCommand()
{
    // Fallback method using virsh command
    QProcess process;
    process.start("virsh", QStringList() << "list" << "--name");
    
    if (!process.waitForFinished(3000)) {
        qDebug() << "virsh command timed out";
        return false;
    }
    
    QString output = process.readAllStandardOutput();
    QStringList runningVMs = output.split('\n', Qt::SplitBehavior::SkipEmptyParts);
    
    return runningVMs.contains(m_vmName);
}

void VMManager::onLibvirtSignal(const QString &signal)
{
    qDebug() << "Received libvirt signal:" << signal;
    checkVMStatus();
}
