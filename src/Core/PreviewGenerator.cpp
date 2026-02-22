#include "PreviewGenerator.h"
#include "LogFileWriter.h"
#include "FilterChain.h"
#include <QProcess>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>

PreviewGenerator::PreviewGenerator(QObject* parent)
    : QObject(parent), audioProcess(nullptr), waveformProcess(nullptr)
    , logWriter(new LogFileWriter(this))
{
    connect(logWriter, &LogFileWriter::contentWritten, this, &PreviewGenerator::logContentWritten);
}

PreviewGenerator::~PreviewGenerator() {
    cancel();
}

void PreviewGenerator::generate(
    const QString& sourceFile,
    const QString& outputFormat,
    int sampleRate,
    int bitDepth,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& sidechainFiles,
    const QString& ffmpegPath,
    const QString& waveformSize) {

    this->sourceFile = sourceFile;
    this->ffmpegPath = ffmpegPath;
    this->waveformSize = waveformSize;

    // Setup temp file paths in FFAB subdirectory
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString ffabTempDir = tempDir + "/FFAB";

    // Create FFAB temp directory if it doesn't exist
    QDir().mkpath(ffabTempDir);

    tempAudioPath = ffabTempDir + "/ffab_preview.wav";  // Always WAV for preview
    tempWaveformPath = ffabTempDir + "/ffab_waveform.png";

    emit started();

    // Open log file if both logging and preview logging are enabled
    accumulatedStderr.clear();
    m_stderrBuffer.clear();
    QSettings settings;
    bool loggingEnabled = settings.value("log/saveToFile", false).toBool();
    bool previewLogging = settings.value("log/logPreview", false).toBool();
    bool previewLoggingActive = loggingEnabled && previewLogging;
    if (previewLoggingActive) {
        QString logFolder = QFileInfo(tempAudioPath).absolutePath();
        if (logWriter->open(logFolder, "preview", 1)) {
            emit logFileCreated(logWriter->filePath());
        }
    }

    currentInputFileName = QFileInfo(sourceFile).fileName();

    // Check if we need to process the audio
    bool needsAudioProcessing = needsProcessing(sourceFile, outputFormat, sampleRate, bitDepth, filterChain);

    if (needsAudioProcessing) {
        // Build FFmpeg command for audio processing
        emit progress(0, "Processing audio...");

        audioProcess = new QProcess(this);
        connect(audioProcess, &QProcess::finished,
                this, &PreviewGenerator::onAudioProcessFinished);

        // Capture stderr for logging (line-buffered to avoid split lines)
        connect(audioProcess, &QProcess::readyReadStandardError, this, [this]() {
            if (!audioProcess) return;
            QByteArray rawData = audioProcess->readAllStandardError();
            m_stderrBuffer += QString::fromUtf8(rawData);

            int lastNewline = m_stderrBuffer.lastIndexOf('\n');
            if (lastNewline < 0) return;  // No complete lines yet

            QString completeLines = m_stderrBuffer.left(lastNewline + 1);
            m_stderrBuffer = m_stderrBuffer.mid(lastNewline + 1);

            accumulatedStderr += completeLines;
            if (logWriter->isOpen()) {
                logWriter->writeLines(currentInputFileName, completeLines);
            }
        });

        // ========== USE FILTERCHAIN AS SOURCE OF TRUTH ==========
        // Build preview command (aux outputs discarded to null muxer)
        // When preview logging is active, use user's log settings so analysis
        // filters produce output; otherwise use safe defaults (-loglevel error).
        auto logSettings = previewLoggingActive ? LogSettings::fromQSettings() : LogSettings();
        QString command = filterChain->buildPreviewCommand(
            sourceFile,
            sidechainFiles,
            tempAudioPath,
            mutedPositions,
            logSettings
        );

        // Parse command string to QStringList for QProcess
        QStringList args = FilterChain::parseCommandToArgs(command);

        qDebug() << "=== PREVIEW GENERATION DEBUG ===";
        qDebug() << "Muted positions:" << mutedPositions;
        qDebug() << "Sidechain files:" << sidechainFiles;
        qDebug() << "Full FFmpeg command:" << ffmpegPath << args.join(" ");
        qDebug() << "================================";

        audioProcess->start(ffmpegPath, args);

        audioFileForPlayback = tempAudioPath;

    } else {
        // No processing needed - generate waveform directly from source
        logWriter->close();
        emit progress(50, "Generating waveform...");
        audioFileForPlayback = sourceFile;
        generateWaveform(sourceFile);
    }
}

void PreviewGenerator::onAudioProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (!audioProcess) return;  // Safety check

    // Drain any remaining stderr that arrived after the last readyReadStandardError signal.
    // Analysis filters (astats, volumedetect, etc.) often write results right before exit.
    m_stderrBuffer += audioProcess->readAllStandardError();
    if (!m_stderrBuffer.isEmpty()) {
        accumulatedStderr += m_stderrBuffer;
        if (logWriter->isOpen()) {
            logWriter->writeLines(currentInputFileName, m_stderrBuffer);
        }
        m_stderrBuffer.clear();
    }

    if (exitCode != 0) {
        QString errorMsg = accumulatedStderr;
        accumulatedStderr.clear();
        qWarning() << "Audio processing failed:" << errorMsg;
        logWriter->close();
        emit error("Failed to process audio: " + errorMsg);
        audioProcess->deleteLater();
        audioProcess = nullptr;
        return;
    }

    emit progress(50, "Generating waveform...");

    // Audio processed successfully - now generate waveform
    generateWaveform(tempAudioPath);

    audioProcess->deleteLater();
    audioProcess = nullptr;
}

void PreviewGenerator::generateWaveform(const QString& audioFile) {
    waveformProcess = new QProcess(this);
    connect(waveformProcess, &QProcess::finished,
            this, &PreviewGenerator::onWaveformProcessFinished);

    // Waveform Colors:
    // #c25018
    // #3f7d9c
    // #838172
    // #AD9E67
    // #a0a0a0
    // #6a6a6a

    QStringList args;
    args << "-i" << audioFile
         << "-filter_complex"
         << QString("showwavespic=s=%1:split_channels=1:colors=#838172|#838172:scale=sqrt:draw=full").arg(waveformSize)
         << "-frames:v" << "1"
         << "-y"
         << "-hide_banner"
         << "-loglevel"
         << "error"
         << "-stats"
         << tempWaveformPath;

    qDebug() << "FFmpeg waveform command:" << ffmpegPath << args.join(" ");

    waveformProcess->start(ffmpegPath, args);
}

void PreviewGenerator::onWaveformProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {   if (!waveformProcess) return; // Safety check
    if (exitCode != 0) {
        QString errorMsg = waveformProcess->readAllStandardError();
        qWarning() << "Waveform generation failed:" << errorMsg;
        emit error("Failed to generate waveform: " + errorMsg);
        logWriter->close();
        emit finished(audioFileForPlayback, tempWaveformPath);
        waveformProcess->deleteLater();
        waveformProcess = nullptr;
        return;
    }

    emit progress(100, "Complete");

    logWriter->close();

    // Determine which audio file to use for playback
    emit finished(audioFileForPlayback, tempWaveformPath);

    waveformProcess->deleteLater();
    waveformProcess = nullptr;
}

bool PreviewGenerator::needsProcessing(
    const QString& sourceFile,
    const QString& outputFormat,
    int sampleRate,
    int bitDepth,
    std::shared_ptr<FilterChain> filterChain) {

    // Check if there are any filters beyond INPUT and OUTPUT
    if (filterChain->getAllFilters().size() > 2) {
        return true;  // Has filters - needs processing
    }

    // Get source file properties
    QFileInfo fileInfo(sourceFile);
    QString sourceFormat = fileInfo.suffix().toUpper();

    // Check if codec/format has changed
    if (outputFormat != "WAV (PCM)" && outputFormat != sourceFormat) {
        return true;  // Output format differs from source
    }

    // TODO: Check sample rate and bit depth changes
    // For now, assume no processing needed if no filters and format matches

    return false;
}

void PreviewGenerator::cancel() {
    logWriter->close();

    if (audioProcess) {
        disconnect(audioProcess, nullptr, this, nullptr);  // Prevent finished/readyRead from firing during waitForFinished
        audioProcess->kill();
        audioProcess->waitForFinished(1000);
        delete audioProcess;
        audioProcess = nullptr;
    }

    if (waveformProcess) {
        disconnect(waveformProcess, nullptr, this, nullptr);
        waveformProcess->kill();
        waveformProcess->waitForFinished(1000);
        delete waveformProcess;
        waveformProcess = nullptr;
    }
}
