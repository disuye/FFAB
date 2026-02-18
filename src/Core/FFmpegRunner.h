#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

/**
 * FFmpegRunner - Execute FFmpeg commands with progress tracking
 * 
 * Features:
 * - Run FFmpeg commands via QProcess
 * - Parse progress output (time, speed, etc.)
 * - Emit signals for progress updates
 * - Capture stdout/stderr
 * - Handle errors gracefully
 * - Cancel/kill running processes
 */
class FFmpegRunner : public QObject {
    Q_OBJECT
    
public:
    enum class Status {
        Idle,
        Running,
        Finished,
        Error,
        Cancelled
    };
    
    struct ProgressInfo {
        double currentTime;    // Current position in seconds
        double totalTime;      // Total duration in seconds
        double speed;          // Processing speed (1.0 = realtime)
        int progressPercent;   // 0-100
        QString timeString;    // e.g. "00:01:23.45"
    };
    
    explicit FFmpegRunner(QObject* parent = nullptr);
    ~FFmpegRunner() override;
    
    // Execute an FFmpeg command (command should NOT include 'ffmpeg' at start)
    void runCommand(const QString& command, const QString& ffmpegPath = "ffmpeg");
    
    // Cancel current process
    void cancel();
    
    // Check if currently running
    bool isRunning() const;
    void setTotalDuration(double seconds) { totalDuration = seconds; }

    // Get current status
    Status getStatus() const;
    
    // Get last error message
    QString getLastError() const;
    
signals:
    void started();
    void progress(const FFmpegRunner::ProgressInfo& info);
    void finished(bool success);
    void error(const QString& errorMessage);
    void outputReceived(const QString& output);  // Raw stdout/stderr
    
private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    
private:
    void parseProgressLine(const QString& line);
    double parseTimeString(const QString& timeStr);
    
    QProcess* process;
    Status status;
    QString lastError;
    double totalDuration;  // Total duration of input file (if known)
    QString currentCommand;
    QString m_stderrBuffer;  // Line buffer for incomplete stderr chunks
};
