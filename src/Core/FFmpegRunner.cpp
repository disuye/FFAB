#include "FFmpegRunner.h"
#include <QRegularExpression>
#include <QDebug>

// Returns true for lines produced by FFmpeg's -progress pipe:2 (key=value, no spaces).
// These are parsed for the UI but are noise in a log file at low verbosity levels.
static bool isProgressPipeLine(const QString& line) {
    if (line.isEmpty() || line.contains(' ')) return false;
    int eq = line.indexOf('=');
    if (eq <= 0) return false;
    for (int i = 0; i < eq; ++i) {
        QChar c = line[i];
        if (!c.isLetterOrNumber() && c != '_') return false;
    }
    return true;
}

FFmpegRunner::FFmpegRunner(QObject* parent)
    : QObject(parent)
    , process(new QProcess(this))
    , status(Status::Idle)
    , totalDuration(0.0)
    , m_lastSpeed(0.0)
    , m_suppressProgressLines(true)   // Default: keep logs clean; caller opts in for verbose
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
    m_lastSpeed = 0.0;
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

    // FFmpeg terminates live progress lines with \r (overwrites in terminal)
    // and metadata/summary lines with \n. Accept either as a line boundary.
    int lastTerminator = qMax(m_stderrBuffer.lastIndexOf('\n'),
                              m_stderrBuffer.lastIndexOf('\r'));
    if (lastTerminator < 0) return;  // No complete lines yet

    // Extract all complete lines, keep incomplete tail
    QString completeLines = m_stderrBuffer.left(lastTerminator + 1);
    m_stderrBuffer = m_stderrBuffer.mid(lastTerminator + 1);

    // Split on both \r and \n so \r-only progress lines are parsed live
    QStringList lines = completeLines.split(QRegularExpression("[\\r\\n]"), Qt::SkipEmptyParts);

    // Build filtered output: omit -progress pipe:2 key=value lines at low verbosity.
    // At verbose/debug level m_suppressProgressLines is false, so everything passes through.
    QString logOutput;
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

        if (!m_suppressProgressLines || !isProgressPipeLine(line)) {
            logOutput += line + '\n';
        }
    }

    if (!logOutput.isEmpty()) {
        emit outputReceived(logOutput);
    }
}

// ========== PROGRESS PARSING ==========

void FFmpegRunner::parseProgressLine(const QString& line) {
    // Handles two FFmpeg output formats:
    //
    // Classic -stats (single line, \r-terminated):
    //   "frame= 123 fps=25.0 q=-1.0 size= 1234kB time=00:00:05.12 bitrate=1638kbits/s speed=4.23x"
    //
    // -progress pipe:2 (one key=value per line, \n-terminated):
    //   out_time=00:00:05.123456   ← "time=" is a substring of "out_time="
    //   speed=4.23x                ← separate line, no "time=" present
    //   progress=continue

    if (!line.contains("time=")) {
        // Not a time line — may be a speed-only line from -progress pipe:2
        if (line.contains("speed=")) {
            QRegularExpression speedRe("speed=\\s*(\\d+\\.?\\d*)x");
            QRegularExpressionMatch m = speedRe.match(line);
            if (m.hasMatch()) {
                m_lastSpeed = m.captured(1).toDouble();
            }
        }
        return;
    }

    ProgressInfo info;
    info.currentTime = 0.0;
    info.totalTime   = totalDuration;
    info.speed       = m_lastSpeed;   // Seed with last-seen speed
    info.progressPercent = 0;
    info.timeString  = "00:00:00.00";

    // Extract time — works for both "time=HH:MM:SS.ff" and "out_time=HH:MM:SS.ffffff"
    // because "out_time=" contains "time=" as a trailing substring.
    QRegularExpression timeRe("time=(\\d{2}):(\\d{2}):(\\d{2}\\.\\d{2})");
    QRegularExpressionMatch timeMatch = timeRe.match(line);
    if (timeMatch.hasMatch()) {
        info.timeString  = timeMatch.captured(0).mid(5);  // Remove "time="
        info.currentTime = parseTimeString(info.timeString);
    }

    // Also parse speed if it appears on the same line (classic -stats format)
    QRegularExpression speedRe("speed=\\s*(\\d+\\.?\\d*)x");
    QRegularExpressionMatch speedMatch = speedRe.match(line);
    if (speedMatch.hasMatch()) {
        info.speed = speedMatch.captured(1).toDouble();
        m_lastSpeed = info.speed;
    }

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
