#ifndef VMOVERLAYAPP_H
#define VMOVERLAYAPP_H

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMessageBox>
#include "vmmanager.h"
#include "overlaymanager.h"

class VMOverlayApp : public QApplication
{
    Q_OBJECT

public:
    VMOverlayApp(int &argc, char **argv);
    ~VMOverlayApp();

    void initialize();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onVMShutdown();
    void onCommitFinished(bool success, const QString &message);
    void onRebuildFinished(bool success, const QString &message);
    void showAbout();
    void quitApplication();

private:
    void createTrayIcon();
    void createMenu();
    void showVMStatus();
    void askCommitOverlay();

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    VMManager *m_vmManager;
    OverlayManager *m_overlayManager;
    bool m_waitingForRebuild;
};

#endif // VMOVERLAYAPP_H
