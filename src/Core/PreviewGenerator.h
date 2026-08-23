#ifndef PREVIEWGENERATOR_H
#define PREVIEWGENERATOR_H

#include <QObject>
#include <QString>
#include <QProcess>
#include <memory>

class FilterChain;
class LogFileWriter;

class PreviewGenerator : public QObject {
    Q_OBJECT
    
public:
    explicit PreviewGenerator(QObject* parent = nullptr);
    ~PreviewGenerator() override;
    
    // Generate preview and waveform
    // waveformSize: resolution for the showwavespic PNG (e.g. "2000x160" or "3000x2000")
    // outputExtension: container extension matching the OUTPUT filter's actual codec
    //   (e.g. "wav", "flac", "m4a" for ALAC/AAC, "mp3") — pass outputFilter->getFileExtension().
    //   Some codecs (ALAC, and likely AAC) cannot be muxed into a .wav container, so this
    //   must match the real codec being previewed rather than defaulting to wav.
    void generate(
        const QString& sourceFile,
        const QString& outputFormat,
        int sampleRate,
        int bitDepth,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& sidechainFiles,
        const QString& ffmpegPath,
        const QString& waveformSize = "2400x1800",
        const QString& outputExtension = "wav"
    );
    
    void cancel();
    
signals:
    void started();
    void progress(int percent, const QString& stage);
    void finished(const QString& audioFilePath, const QString& waveformPath);
    void error(const QString& message);
    void logFileCreated(const QString& filePath);
    void logContentWritten();

private slots:
    void onAudioProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onWaveformProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
private:
    bool needsProcessing(
        const QString& sourceFile,
        const QString& outputFormat,
        int sampleRate,
        int bitDepth,
        std::shared_ptr<FilterChain> filterChain
    );
    
    void generateWaveform(const QString& audioFile);
    
    static constexpr int kTempSlots = 3;  // Rotating buffer — never overwrite the playing file
    int m_tempSlot = 0;

    QString sourceFile;
    QString ffmpegPath;
    QString tempAudioPath;
    QString tempWaveformPath;
    QString audioFileForPlayback;
    QString waveformSize;
    
    QProcess* audioProcess;
    QProcess* waveformProcess;
    LogFileWriter* logWriter;
    QString currentInputFileName;
    QString accumulatedStderr;
    QString m_stderrBuffer;  // Line buffer for incomplete stderr chunks
};

#endif // PREVIEWGENERATOR_H