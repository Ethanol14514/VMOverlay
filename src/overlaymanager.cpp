#include "overlaymanager.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>

OverlayManager::OverlayManager(QObject *parent)
    : QObject(parent)
    , m_currentProcess(nullptr)
{
}

OverlayManager::~OverlayManager()
{
    if (m_currentProcess) {
        m_currentProcess->kill();
        m_currentProcess->deleteLater();
    }
}

void OverlayManager::setOverlayPath(const QString &overlayPath)
{
    m_overlayPath = overlayPath;
}

void OverlayManager::setBasePath(const QString &basePath)
{
    m_basePath = basePath;
}

bool OverlayManager::commitOverlay()
{
    if (m_overlayPath.isEmpty() || m_basePath.isEmpty()) {
        qDebug() << "Overlay or base path not set";
        emit commitFinished(false, "路径未配置");
        return false;
    }
    
    emit operationProgress("正在提交 overlay 到 base...");
    
    // Use qemu-img commit to merge overlay into base
    QStringList args;
    args << "commit" << m_overlayPath;
    
    return executeCommand("qemu-img", args);
}

bool OverlayManager::rebuildOverlay()
{
    if (m_overlayPath.isEmpty() || m_basePath.isEmpty()) {
        qDebug() << "Overlay or base path not set";
        emit rebuildFinished(false, "路径未配置");
        return false;
    }
    
    emit operationProgress("正在重建 overlay...");
    
    // Remove old overlay if exists
    if (QFile::exists(m_overlayPath)) {
        if (!QFile::remove(m_overlayPath)) {
            qDebug() << "Failed to remove old overlay";
            emit rebuildFinished(false, "删除旧 overlay 失败");
            return false;
        }
    }
    
    // Create new overlay using qemu-img
    QStringList args;
    args << "create" << "-f" << "qcow2" 
         << "-F" << "qcow2"
         << "-b" << m_basePath 
         << m_overlayPath;
    
    return executeCommand("qemu-img", args);
}

bool OverlayManager::executeCommand(const QString &command, const QStringList &args)
{
    if (m_currentProcess) {
        qDebug() << "Another operation is in progress";
        emit operationProgress("另一个操作正在进行中，请稍后再试");
        return false;
    }
    
    m_currentProcess = new QProcess(this);
    
    connect(m_currentProcess, &QProcess::finished,
            this, &OverlayManager::onProcessFinished);
    connect(m_currentProcess, &QProcess::errorOccurred,
            this, &OverlayManager::onProcessError);
    connect(m_currentProcess, &QProcess::readyReadStandardOutput,
            this, &OverlayManager::onProcessOutput);
    connect(m_currentProcess, &QProcess::readyReadStandardError,
            this, &OverlayManager::onProcessOutput);
    
    qDebug() << "Executing:" << command << args.join(" ");
    m_currentProcess->start(command, args);
    
    return m_currentProcess->waitForStarted();
}

void OverlayManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString output = m_currentProcess->readAllStandardOutput();
    QString errorOutput = m_currentProcess->readAllStandardError();
    
    qDebug() << "Process finished with exit code:" << exitCode;
    qDebug() << "Output:" << output;
    qDebug() << "Error:" << errorOutput;
    
    bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
    QString message = success ? "操作成功" : "操作失败: " + errorOutput;
    
    QString program = m_currentProcess->program();
    QStringList args = m_currentProcess->arguments();
    
    m_currentProcess->deleteLater();
    m_currentProcess = nullptr;
    
    // Determine which operation finished
    if (args.contains("commit")) {
        emit commitFinished(success, message);
    } else if (args.contains("create")) {
        emit rebuildFinished(success, message);
    }
}

void OverlayManager::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "进程启动失败";
            break;
        case QProcess::Crashed:
            errorMsg = "进程崩溃";
            break;
        case QProcess::Timedout:
            errorMsg = "进程超时";
            break;
        default:
            errorMsg = "未知错误";
    }
    
    qDebug() << "Process error:" << errorMsg;
    
    QString program = m_currentProcess->program();
    QStringList args = m_currentProcess->arguments();
    
    m_currentProcess->deleteLater();
    m_currentProcess = nullptr;
    
    if (args.contains("commit")) {
        emit commitFinished(false, errorMsg);
    } else if (args.contains("create")) {
        emit rebuildFinished(false, errorMsg);
    }
}

void OverlayManager::onProcessOutput()
{
    QString output = m_currentProcess->readAllStandardOutput();
    QString errorOutput = m_currentProcess->readAllStandardError();
    
    if (!output.isEmpty()) {
        emit operationProgress(output);
    }
    if (!errorOutput.isEmpty()) {
        emit operationProgress(errorOutput);
    }
}
