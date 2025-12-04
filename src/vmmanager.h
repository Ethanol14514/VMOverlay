#ifndef VMMANAGER_H
#define VMMANAGER_H

#include <QObject>
#include <QString>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusServiceWatcher>

class VMManager : public QObject
{
    Q_OBJECT

public:
    explicit VMManager(QObject *parent = nullptr);
    ~VMManager();

    bool isVMRunning() const;
    QString getVMStatus() const;
    void startMonitoring(const QString &vmName);

signals:
    void vmShutdown();
    void vmStatusChanged(bool running);

private slots:
    void checkVMStatus();
    void onLibvirtSignal(const QString &signal);

private:
    QString m_vmName;
    bool m_lastRunningState;
    QDBusConnection m_systemBus;
    QDBusServiceWatcher *m_serviceWatcher;
    
    bool queryVMState();
    bool queryVMStateViaCommand();
};

#endif // VMMANAGER_H
