#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include <QObject>
#include <QString>
#include <QProcess>

class OverlayManager : public QObject
{
    Q_OBJECT

public:
    explicit OverlayManager(QObject *parent = nullptr);
    ~OverlayManager();

    void setOverlayPath(const QString &overlayPath);
    void setBasePath(const QString &basePath);
    
    bool commitOverlay();
    bool rebuildOverlay();

signals:
    void commitFinished(bool success, const QString &message);
    void rebuildFinished(bool success, const QString &message);
    void operationProgress(const QString &message);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessOutput();

private:
    QString m_overlayPath;
    QString m_basePath;
    QProcess *m_currentProcess;
    
    bool executeCommand(const QString &command, const QStringList &args);
};

#endif // OVERLAYMANAGER_H
