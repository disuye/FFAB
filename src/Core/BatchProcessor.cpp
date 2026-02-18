#include "BatchProcessor.h"
#include "LogFileWriter.h"
#include "Core/FilterChain.h"
#include "Filters/OutputFilter.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QSettings>

BatchProcessor::BatchProcessor(QObject* parent)
    : QObject(parent)
    , runner(new FFmpegRunner(this))
    , logWriter(new LogFileWriter(this))
    , state(State::Idle)
    , totalFiles(0)
    , completedFiles(0)
    , failedFiles(0)
{
    // Connect runner signals
    connect(runner, &FFmpegRunner::progress, this, &BatchProcessor::fileProgress);
    connect(runner, &FFmpegRunner::finished, this, &BatchProcessor::onFileFinished);

    // Connect stderr capture for logging
    connect(runner, &FFmpegRunner::outputReceived, this, [this](const QString& output) {
        if (logWriter->isOpen() && !currentInputFileName.isEmpty()) {
            logWriter->writeLines(currentInputFileName, output);
        }
    });

    // Forward contentWritten for live View Log updates
    connect(logWriter, &LogFileWriter::contentWritten, this, &BatchProcessor::logContentWritten);
}

BatchProcessor::~BatchProcessor() = default;

// ========== ORIGINAL START (backward compatible) ==========

void BatchProcessor::start(const QList<FileListWidget::AudioFileInfo>& files,
                           const QString& outputFolder,
                           std::shared_ptr<FilterChain> filterChain,
                           const QList<int>& mutedPositions,
                           const QStringList& sidechainFiles,
                           const QString& ffmpegPath_) {
    
    if (state == State::Processing) {
        qWarning() << "BatchProcessor: Already processing";
        return;
    }
    
    // Build jobs the old way and delegate to new start()
    auto logSettings = LogSettings::fromQSettings();
    QList<JobInfo> jobs;

    for (const auto& file : files) {
        JobInfo job;
        job.inputFile = file;
        job.sidechainFiles = sidechainFiles;
        job.combinedBaseName = QFileInfo(file.fileName).completeBaseName();
        
        // Get OUTPUT settings from filterChain
        QString filenameSuffix = "_processed";  // Default fallback
        QString outputExtension = "wav";  // Default fallback
        
        if (filterChain) {
            auto outputFilter = filterChain->getFilter(filterChain->filterCount() - 1);
            if (auto* output = dynamic_cast<OutputFilter*>(outputFilter.get())) {
                filenameSuffix = output->getFilenameSuffix();
                outputExtension = output->getFileExtension();
            }
        }
        
        // Build output path: baseName + suffix + extension
        QString outputFileName = QFileInfo(file.fileName).completeBaseName() + 
                                filenameSuffix + "." + outputExtension;
        job.outputPath = QDir(outputFolder).filePath(outputFileName);
        
        // Build FFmpeg command WITH sidechain files
        if (!sidechainFiles.isEmpty()) {
            job.command = filterChain->buildCompleteCommand(
                file.filePath,
                sidechainFiles,
                job.outputPath,
                mutedPositions,
                logSettings
            );
        } else {
            job.command = filterChain->buildCompleteCommand(
                file.filePath,
                job.outputPath,
                mutedPositions,
                logSettings
            );
        }
        
        jobs.append(job);
    }
    
    start(jobs, ffmpegPath_);
}

// ========== NEW START (pre-built jobs from JobListBuilder) ==========

void BatchProcessor::start(const QList<JobInfo>& jobs, const QString& ffmpegPath_) {
    if (state == State::Processing) {
        qWarning() << "BatchProcessor: Already processing";
        return;
    }
    
    // Store ffmpeg path
    ffmpegPath = ffmpegPath_;
    
    // Clear previous queue
    jobQueue.clear();
    completedFiles = 0;
    failedFiles = 0;
    
    // Enqueue all jobs
    for (const auto& job : jobs) {
        jobQueue.enqueue(job);
    }
    
    totalFiles = jobQueue.size();

    // Open log file if logging is enabled
    QSettings settings;
    bool loggingEnabled = settings.value("log/saveToFile", false).toBool();
    if (loggingEnabled) {
        QString outputFolder;
        if (!jobs.isEmpty()) {
            outputFolder = QFileInfo(jobs.first().outputPath).absolutePath();
        }
        if (!outputFolder.isEmpty()) {
            if (logWriter->open(outputFolder, "batch", totalFiles)) {
                emit logFileCreated(logWriter->filePath());
            }
        }
    }

    qDebug() << "BatchProcessor: Starting batch with" << totalFiles << "files";
    
    setState(State::Processing);
    emit started(totalFiles);
    
    // Start processing first file
    processNextFile();
}

void BatchProcessor::pause() {
    if (state == State::Processing) {
        setState(State::Paused);
        runner->cancel();  // Cancel current file
    }
}

void BatchProcessor::resume() {
    if (state == State::Paused) {
        setState(State::Processing);
        processNextFile();
    }
}

void BatchProcessor::cancel() {
    if (state == State::Idle || state == State::Finished) {
        // Nothing running, just return
        return;
    }
    
    setState(State::Cancelled);
    runner->cancel();
    jobQueue.clear();
    logWriter->close();

    qDebug() << "BatchProcessor: Cancelled";
    emit allFinished(completedFiles, failedFiles);
}

BatchProcessor::State BatchProcessor::getState() const {
    return state;
}

int BatchProcessor::getTotalFiles() const {
    return totalFiles;
}

int BatchProcessor::getCompletedFiles() const {
    return completedFiles;
}

int BatchProcessor::getFailedFiles() const {
    return failedFiles;
}

QString BatchProcessor::getCurrentFile() const {
    return currentFileName;
}

// ========== PRIVATE METHODS ==========

void BatchProcessor::processNextFile() {
    if (state != State::Processing) {
        return;
    }
    
    if (jobQueue.isEmpty()) {
        // All files processed
        logWriter->close();
        setState(State::Finished);
        qDebug() << "BatchProcessor: Finished -" << completedFiles << "succeeded," << failedFiles << "failed";
        emit allFinished(completedFiles, failedFiles);
        return;
    }
    
    // Get next job
    JobInfo job = jobQueue.dequeue();
    currentFileName = job.inputFile.fileName;
    currentInputFileName = QFileInfo(job.inputFile.fileName).fileName();
    currentJobIsCascade = job.isCascade;
    
    int fileNumber = (totalFiles - jobQueue.size());
    
    qDebug() << "BatchProcessor: Processing file" << fileNumber << "/" << totalFiles << ":" << currentFileName;
    emit fileStarted(currentFileName, fileNumber, totalFiles);
    
    // Run FFmpeg command with detected ffmpeg path
    runner->runCommand(job.command, ffmpegPath);
    // Pre-set duration for progress percentage (loglevel error suppresses Duration: line)
    double dur = parseDurationString(job.inputFile.duration);
    if (dur > 0.0) runner->setTotalDuration(dur);
}

void BatchProcessor::onFileFinished(bool success) {
    if (success) {
        completedFiles++;
        qDebug() << "BatchProcessor: File succeeded:" << currentFileName;
    } else {
        failedFiles++;
        qWarning() << "BatchProcessor: File failed:" << currentFileName;
        
        // Cascade abort: if a cascade job fails, the remaining chain is invalid
        // because each job's sidechain depends on the previous job's output
        if (currentJobIsCascade && !jobQueue.isEmpty()) {
            int aborted = jobQueue.size();
            failedFiles += aborted;
            qWarning() << "BatchProcessor: Cascade broken — aborting" << aborted << "remaining jobs";
            jobQueue.clear();
            logWriter->close();

            emit fileFinished(currentFileName, false);
            setState(State::Finished);
            emit allFinished(completedFiles, failedFiles);
            return;
        }
    }
    
    emit fileFinished(currentFileName, success);
    
    // Process next file (if not paused/cancelled)
    if (state == State::Processing) {
        processNextFile();
    }
}

void BatchProcessor::setState(State newState) {
    if (state != newState) {
        state = newState;
        emit stateChanged(state);
    }
}

double BatchProcessor::parseDurationString(const QString& dur) {
    // "HH:MM:SS" or "HH:MM:SS.mm"
    QStringList parts = dur.split(':');
    if (parts.size() == 3) {
        return parts[0].toInt() * 3600 + parts[1].toInt() * 60 + parts[2].toDouble();
    }
    return 0.0;
}
