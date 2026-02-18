#include "FFmpegRunner.h"
#include <QRegularExpression>
#include <QDebug>

FFmpegRunner::FFmpegRunner(QObject* parent)
    : QObject(parent)
    , process(new QProcess(this))
    , status(Status::Idle)
    , totalDuration(0.0)
{
    // Connect process signals
    connect(process, &QProcess::started, this, &FFmpegRunner::onProcessStarted);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FFmpegRunner::onProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &FFmpegRunner::onProcessError);
    connect(process, &QProcess::readyReadStandardOutput, this, &FFmpegRunner::onReadyReadStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &FFmpegRunner::onReadyReadStandardError);
}

FFmpegRunner::~FFmpegRunner() {
    if (process->state() == QProcess::Running) {
        process->kill();
        process->waitForFinished();
    }
}

void FFmpegRunner::runCommand(const QString& command, const QString& ffmpegPath) {
    if (status == Status::Running) {
        qWarning() << "FFmpegRunner: Already running a command";
        return;
    }
    
    currentCommand = command;
    status = Status::Running;
    lastError.clear();
    totalDuration = 0.0;
    m_stderrBuffer.clear();
    
    qDebug() << "FFmpegRunner: Executing:" << ffmpegPath << command;
    
    // Parse command string into arguments (simple split on spaces, respecting quotes)
    QStringList args;
    QRegularExpression re("(\"[^\"]*\"|\\S+)");
    QRegularExpressionMatchIterator it = re.globalMatch(command);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString arg = match.captured(1);
        // Remove quotes if present
        if (arg.startsWith('"') && arg.endsWith('"')) {
            arg = arg.mid(1, arg.length() - 2);
        }
        args << arg;
    }
    
    // Execute directly without shell
    process->start(ffmpegPath, args);
}

void FFmpegRunner::cancel() {
    if (status == Status::Running) {
        qDebug() << "FFmpegRunner: Cancelling process";
        status = Status::Cancelled;
        process->kill();
    }
}

bool FFmpegRunner::isRunning() const {
    return status == Status::Running;
}

FFmpegRunner::Status FFmpegRunner::getStatus() const {
    return status;
}

QString FFmpegRunner::getLastError() const {
    return lastError;
}

// ========== PROCESS SLOTS ==========

void FFmpegRunner::onProcessStarted() {
    qDebug() << "FFmpegRunner: Process started";
    emit started();
}

void FFmpegRunner::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    // Flush any remaining buffered stderr
    if (!m_stderrBuffer.isEmpty()) {
        emit outputReceived(m_stderrBuffer);
        m_stderrBuffer.clear();
    }

    bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit && status != Status::Cancelled);

    if (status == Status::Cancelled) {
        qDebug() << "FFmpegRunner: Process cancelled";
        emit finished(false);
        return;
    }
    
    if (success) {
        qDebug() << "FFmpegRunner: Process finished successfully";
        status = Status::Finished;
        emit finished(true);
    } else {
        qWarning() << "FFmpegRunner: Process failed with exit code:" << exitCode;
        status = Status::Error;
        lastError = QString("FFmpeg exited with code %1").arg(exitCode);
        emit error(lastError);
        emit finished(false);
    }
}

void FFmpegRunner::onProcessError(QProcess::ProcessError error) {
    QString errorMsg;
    
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start FFmpeg. Is it installed?";
            break;
        case QProcess::Crashed:
            errorMsg = "FFmpeg crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "FFmpeg timed out";
            break;
        case QProcess::WriteError:
            errorMsg = "Write error";
            break;
        case QProcess::ReadError:
            errorMsg = "Read error";
            break;
        default:
            errorMsg = "Unknown error";
            break;
    }
    
    qWarning() << "FFmpegRunner error:" << errorMsg;
    status = Status::Error;
    lastError = errorMsg;
    emit this->error(errorMsg);
}

void FFmpegRunner::onReadyReadStandardOutput() {
    QString output = process->readAllStandardOutput();
    emit outputReceived(output);
    
    // Parse for progress info
    QStringList lines = output.split('\n');
    for (const QString& line : lines) {
        parseProgressLine(line);
    }
}

void FFmpegRunner::onReadyReadStandardError() {
    QByteArray rawData = process->readAllStandardError();
    m_stderrBuffer += QString::fromUtf8(rawData);

    int lastNewline = m_stderrBuffer.lastIndexOf('\n');
    if (lastNewline < 0) return;  // No complete lines yet

    // Extract all complete lines, keep incomplete tail
    QString completeLines = m_stderrBuffer.left(lastNewline + 1);
    m_stderrBuffer = m_stderrBuffer.mid(lastNewline + 1);

    emit outputReceived(completeLines);

    // FFmpeg writes progress to stderr
    QStringList lines = completeLines.split('\n');
    for (const QString& line : lines) {
        parseProgressLine(line);

        // Parse total duration from input file info
        // Example: "Duration: 00:03:45.67, start: 0.000000, bitrate: 1411 kb/s"
        if (line.contains("Duration:") && totalDuration == 0.0) {
            QRegularExpression durationRe("Duration: (\\d{2}):(\\d{2}):(\\d{2}\\.\\d{2})");
            QRegularExpressionMatch match = durationRe.match(line);
            if (match.hasMatch()) {
                int hours = match.captured(1).toInt();
                int minutes = match.captured(2).toInt();
                double seconds = match.captured(3).toDouble();
                totalDuration = hours * 3600 + minutes * 60 + seconds;
                qDebug() << "FFmpegRunner: Detected total duration:" << totalDuration << "seconds";
            }
        }
    }
}

// ========== PROGRESS PARSING ==========

void FFmpegRunner::parseProgressLine(const QString& line) {
    // FFmpeg progress line format:
    // "frame=  123 fps= 0.0 q=-1.0 size=    1024kB time=00:00:05.12 bitrate=1638.4kbits/s speed=10.2x"
    
    if (!line.contains("time=")) return;
    
    ProgressInfo info;
    info.currentTime = 0.0;
    info.totalTime = totalDuration;
    info.speed = 0.0;
    info.progressPercent = 0;
    info.timeString = "00:00:00.00";
    
    // Extract time
    QRegularExpression timeRe("time=(\\d{2}):(\\d{2}):(\\d{2}\\.\\d{2})");
    QRegularExpressionMatch timeMatch = timeRe.match(line);
    if (timeMatch.hasMatch()) {
        info.timeString = timeMatch.captured(0).mid(5);  // Remove "time="
        info.currentTime = parseTimeString(info.timeString);
    }
    
    // Extract speed
    QRegularExpression speedRe("speed=\\s*(\\d+\\.?\\d*)x");
    QRegularExpressionMatch speedMatch = speedRe.match(line);
    if (speedMatch.hasMatch()) {
        info.speed = speedMatch.captured(1).toDouble();
    }
    
    // Calculate progress percentage
    if (totalDuration > 0.0 && info.currentTime > 0.0) {
        info.progressPercent = static_cast<int>((info.currentTime / totalDuration) * 100);
        info.progressPercent = qMin(info.progressPercent, 100);
    }
    
    emit progress(info);
}

double FFmpegRunner::parseTimeString(const QString& timeStr) {
    // Format: "HH:MM:SS.SS"
    QRegularExpression re("(\\d{2}):(\\d{2}):(\\d{2}\\.\\d{2})");
    QRegularExpressionMatch match = re.match(timeStr);
    
    if (!match.hasMatch()) return 0.0;
    
    int hours = match.captured(1).toInt();
    int minutes = match.captured(2).toInt();
    double seconds = match.captured(3).toDouble();
    
    return hours * 3600 + minutes * 60 + seconds;
}
