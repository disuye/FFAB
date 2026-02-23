#pragma once

#include <QObject>
#include <QQueue>
#include <QString>
#include <QVector>
#include <memory>
#include "FFmpegRunner.h"
#include "UI/FileListWidget.h"

class FilterChain;
class LogFileWriter;

/**
 * BatchProcessor - Process multiple audio files in parallel
 *
 * Features:
 * - Queue of files to process
 * - Parallel processing (N workers, N read from QSettings "processing/maxConcurrent")
 * - Progress tracking per worker and overall
 * - Pause/resume/cancel functionality
 * - Uses FilterChain to build commands
 * - Uses FFmpegRunner instances to execute
 *
 * Two entry points:
 *   1. start(files, outputFolder, filterChain, ...) — original, builds jobs internally
 *   2. start(jobs, ffmpegPath) — new, accepts pre-built jobs from JobListBuilder
 */
class BatchProcessor : public QObject {
    Q_OBJECT

public:
    enum class State {
        Idle,
        Processing,
        Paused,
        Finished,
        Cancelled
    };

    struct JobInfo {
        FileListWidget::AudioFileInfo inputFile;
        QStringList sidechainFiles;      // Per-job sidechain files (varies per algorithm)
        QString combinedBaseName;        // Pre-computed: "drumhit01_cathedral" etc.
        QString outputPath;
        QString command;
        bool isCascade = false;          // If true, abort remaining jobs on failure
    };

    explicit BatchProcessor(QObject* parent = nullptr);
    ~BatchProcessor() override;

    // Original entry point — builds jobs internally (backward compatible)
    void start(const QList<FileListWidget::AudioFileInfo>& files,
              const QString& outputFolder,
              std::shared_ptr<FilterChain> filterChain,
              const QList<int>& mutedPositions,
              const QStringList& sidechainFiles,
              const QString& ffmpegPath);

    // New entry point — accepts pre-built jobs from JobListBuilder
    void start(const QList<JobInfo>& jobs, const QString& ffmpegPath);

    // Control processing
    void pause();
    void resume();
    void cancel();

    // Status
    State getState() const;
    int getTotalFiles() const;
    int getCompletedFiles() const;
    int getFailedFiles() const;
    QString getCurrentFile() const;

signals:
    void started(int totalFiles);
    void fileStarted(const QString& fileName, int fileNumber, int totalFiles, int workerIndex);
    void fileProgress(const FFmpegRunner::ProgressInfo& info, int workerIndex);
    void fileFinished(const QString& fileName, bool success, int workerIndex);
    void allFinished(int completed, int failed);
    void stateChanged(BatchProcessor::State state);
    void logFileCreated(const QString& filePath);
    void logContentWritten();

private:
    struct WorkerState {
        FFmpegRunner* runner          = nullptr;
        JobInfo       currentJob;              // full job — needed to re-queue on pause
        QString currentFileName;
        QString currentInputFileName;  // bare filename for log prefixing
        bool active                   = false;
        bool currentJobIsCascade      = false;
    };

    void dispatchToWorker(int workerIndex);
    int  activeWorkerCount() const;
    void onWorkerProgress(int workerIndex, FFmpegRunner::ProgressInfo info);
    void onWorkerFinished(int workerIndex, bool success);
    void setState(State newState);
    double parseDurationString(const QString& dur);

    QVector<WorkerState> m_workers;
    int m_dispatched = 0;  // total jobs dispatched so far (drives fileNumber)

    LogFileWriter* logWriter;
    QQueue<JobInfo> jobQueue;
    State state;
    int totalFiles;
    int completedFiles;
    int failedFiles;
    QString ffmpegPath;
};
