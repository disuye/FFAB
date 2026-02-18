#pragma once

#include <QObject>
#include <QQueue>
#include <QString>
#include <memory>
#include "FFmpegRunner.h"
#include "UI/FileListWidget.h"

class FilterChain;
class LogFileWriter;

/**
 * BatchProcessor - Process multiple audio files sequentially
 * 
 * Features:
 * - Queue of files to process
 * - Sequential processing (one at a time)
 * - Progress tracking per file and overall
 * - Pause/resume/cancel functionality
 * - Uses FilterChain to build commands
 * - Uses FFmpegRunner to execute
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
    void fileStarted(const QString& fileName, int fileNumber, int totalFiles);
    void fileProgress(const FFmpegRunner::ProgressInfo& info);
    void fileFinished(const QString& fileName, bool success);
    void allFinished(int completed, int failed);
    void stateChanged(BatchProcessor::State state);
    void logFileCreated(const QString& filePath);
    void logContentWritten();
    
private slots:
    void onFileFinished(bool success);
    
private:
    void processNextFile();
    void setState(State newState);
    double parseDurationString(const QString& dur);
    
    FFmpegRunner* runner;
    LogFileWriter* logWriter;
    QQueue<JobInfo> jobQueue;
    State state;
    int totalFiles;
    int completedFiles;
    int failedFiles;
    QString currentFileName;
    QString currentInputFileName;  // bare filename for log prefixing
    QString ffmpegPath;  // Path to ffmpeg executable
    bool currentJobIsCascade = false;  // Tracks if current job is part of a cascade
};
