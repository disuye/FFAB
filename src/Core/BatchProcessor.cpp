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
    , logWriter(new LogFileWriter(this))
    , state(State::Idle)
    , totalFiles(0)
    , completedFiles(0)
    , failedFiles(0)
{
    connect(logWriter, &LogFileWriter::contentWritten, this, &BatchProcessor::logContentWritten);
}

BatchProcessor::~BatchProcessor() {
    for (auto& w : m_workers) {
        delete w.runner;
        w.runner = nullptr;
    }
}

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

    auto logSettings = LogSettings::fromQSettings();
    QList<JobInfo> jobs;

    for (const auto& file : files) {
        JobInfo job;
        job.inputFile = file;
        job.sidechainFiles = sidechainFiles;
        job.combinedBaseName = QFileInfo(file.fileName).completeBaseName();

        QString filenameSuffix = "_processed";
        QString outputExtension = "wav";

        if (filterChain) {
            auto outputFilter = filterChain->getFilter(filterChain->filterCount() - 1);
            if (auto* output = dynamic_cast<OutputFilter*>(outputFilter.get())) {
                filenameSuffix = output->getFilenameSuffix();
                outputExtension = output->getFileExtension();
            }
        }

        QString outputFileName = QFileInfo(file.fileName).completeBaseName() +
                                filenameSuffix + "." + outputExtension;
        job.outputPath = QDir(outputFolder).filePath(outputFileName);

        if (!sidechainFiles.isEmpty()) {
            job.command = filterChain->buildCompleteCommand(
                file.filePath, sidechainFiles, job.outputPath, mutedPositions, logSettings);
        } else {
            job.command = filterChain->buildCompleteCommand(
                file.filePath, job.outputPath, mutedPositions, logSettings);
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

    ffmpegPath = ffmpegPath_;
    jobQueue.clear();
    completedFiles = 0;
    failedFiles = 0;
    m_dispatched  = 0;

    for (const auto& job : jobs) {
        jobQueue.enqueue(job);
    }
    totalFiles = jobQueue.size();

    // Cascade guard: Iterate batches are inherently serial (each output feeds the next)
    bool hasCascade = std::any_of(jobs.begin(), jobs.end(),
                                  [](const JobInfo& j){ return j.isCascade; });

    QSettings settings;
    int maxConcurrent = settings.value("processing/maxConcurrent", 1).toInt();
    if (hasCascade) maxConcurrent = 1;
    maxConcurrent = qMax(1, maxConcurrent);

    // Progress pipe:2 lines are noise at low log levels; include only at verbose/debug/trace.
    const QString logLevel = settings.value("log/logLevel", "error").toString();
    const bool suppressProgress = !(logLevel == "verbose" || logLevel == "debug" || logLevel == "trace");

    // Tear down old runners and build fresh pool
    for (auto& w : m_workers) {
        if (w.runner) {
            w.runner->disconnect();
            delete w.runner;
            w.runner = nullptr;
        }
    }
    m_workers.resize(maxConcurrent);

    for (int i = 0; i < maxConcurrent; ++i) {
        m_workers[i] = WorkerState{};
        m_workers[i].runner = new FFmpegRunner(this);
        m_workers[i].runner->setSuppressProgressLines(suppressProgress);

        // Progress forwarding
        connect(m_workers[i].runner, &FFmpegRunner::progress,
                this, [this, i](FFmpegRunner::ProgressInfo info) {
                    onWorkerProgress(i, info);
                });

        // Completion callback
        connect(m_workers[i].runner, &FFmpegRunner::finished,
                this, [this, i](bool success) {
                    onWorkerFinished(i, success);
                });

        // Log output
        connect(m_workers[i].runner, &FFmpegRunner::outputReceived,
                this, [this, i](const QString& output) {
                    if (logWriter->isOpen() && !m_workers[i].currentInputFileName.isEmpty()) {
                        logWriter->writeLines(m_workers[i].currentInputFileName, output);
                    }
                });
    }

    // Open batch log
    bool loggingEnabled = settings.value("log/saveToFile", false).toBool();
    if (loggingEnabled && !jobs.isEmpty()) {
        QString outputFolder = QFileInfo(jobs.first().outputPath).absolutePath();
        if (!outputFolder.isEmpty()) {
            if (logWriter->open(outputFolder, "batch", totalFiles)) {
                emit logFileCreated(logWriter->filePath());
            }
        }
    }

    qDebug() << "BatchProcessor: Starting" << totalFiles << "files across"
             << maxConcurrent << "workers";

    setState(State::Processing);
    emit started(totalFiles);

    // Fill all slots immediately
    for (int i = 0; i < maxConcurrent && !jobQueue.isEmpty(); ++i) {
        dispatchToWorker(i);
    }
}

// ========== WORKER DISPATCH ==========

void BatchProcessor::dispatchToWorker(int i) {
    if (jobQueue.isEmpty()) {
        m_workers[i].active = false;
        return;
    }

    JobInfo job = jobQueue.dequeue();
    ++m_dispatched;

    m_workers[i].active               = true;
    m_workers[i].currentJob           = job;
    m_workers[i].currentFileName      = job.inputFile.fileName;
    m_workers[i].currentInputFileName = QFileInfo(job.inputFile.fileName).fileName();
    m_workers[i].currentJobIsCascade  = job.isCascade;

    qDebug() << "BatchProcessor: Worker" << i << "starting file"
             << m_dispatched << "/" << totalFiles << ":" << m_workers[i].currentFileName;

    emit fileStarted(m_workers[i].currentFileName, m_dispatched, totalFiles, i);

    m_workers[i].runner->runCommand(job.command, ffmpegPath);
    double dur = parseDurationString(job.inputFile.duration);
    if (dur > 0.0) m_workers[i].runner->setTotalDuration(dur);
}

void BatchProcessor::onWorkerProgress(int i, FFmpegRunner::ProgressInfo info) {
    emit fileProgress(info, i);
}

void BatchProcessor::onWorkerFinished(int i, bool success) {
    // If paused, this completion came from cancel() during pause — re-queue for after resume.
    // Don't count as failure; don't emit fileFinished.
    if (state == State::Paused) {
        jobQueue.prepend(m_workers[i].currentJob);
        --m_dispatched;
        m_workers[i].active = false;
        return;
    }

    const QString finishedName = m_workers[i].currentFileName;
    const bool wasCascade      = m_workers[i].currentJobIsCascade;

    if (success) {
        completedFiles++;
        qDebug() << "BatchProcessor: Worker" << i << "succeeded:" << finishedName;
    } else {
        failedFiles++;
        qWarning() << "BatchProcessor: Worker" << i << "failed:" << finishedName;

        if (wasCascade && !jobQueue.isEmpty()) {
            int aborted = jobQueue.size();
            failedFiles += aborted;
            qWarning() << "BatchProcessor: Cascade broken — aborting" << aborted << "remaining jobs";
            jobQueue.clear();

            m_workers[i].active = false;
            emit fileFinished(finishedName, false, i);

            if (activeWorkerCount() == 0) {
                logWriter->close();
                setState(State::Finished);
                emit allFinished(completedFiles, failedFiles);
            }
            return;
        }
    }

    m_workers[i].active = false;
    emit fileFinished(finishedName, success, i);

    if (state == State::Processing && !jobQueue.isEmpty()) {
        dispatchToWorker(i);
        return;
    }

    if (jobQueue.isEmpty() && activeWorkerCount() == 0) {
        logWriter->close();
        setState(State::Finished);
        qDebug() << "BatchProcessor: Finished —" << completedFiles << "succeeded,"
                 << failedFiles << "failed";
        emit allFinished(completedFiles, failedFiles);
    }
}

int BatchProcessor::activeWorkerCount() const {
    int count = 0;
    for (const auto& w : m_workers) {
        if (w.active) ++count;
    }
    return count;
}

// ========== CONTROLS ==========

void BatchProcessor::pause() {
    if (state != State::Processing) return;
    setState(State::Paused);
    for (auto& w : m_workers) {
        if (w.active && w.runner) w.runner->cancel();
    }
}

void BatchProcessor::resume() {
    if (state != State::Paused) return;
    setState(State::Processing);
    for (int i = 0; i < m_workers.size() && !jobQueue.isEmpty(); ++i) {
        if (!m_workers[i].active) dispatchToWorker(i);
    }
}

void BatchProcessor::cancel() {
    if (state == State::Idle || state == State::Finished) return;
    setState(State::Cancelled);
    for (auto& w : m_workers) {
        if (w.runner) w.runner->cancel();
    }
    jobQueue.clear();
    logWriter->close();
    qDebug() << "BatchProcessor: Cancelled";
    emit allFinished(completedFiles, failedFiles);
}

// ========== STATUS ==========

BatchProcessor::State BatchProcessor::getState() const { return state; }
int BatchProcessor::getTotalFiles()     const { return totalFiles; }
int BatchProcessor::getCompletedFiles() const { return completedFiles; }
int BatchProcessor::getFailedFiles()    const { return failedFiles; }

QString BatchProcessor::getCurrentFile() const {
    for (const auto& w : m_workers) {
        if (w.active) return w.currentFileName;
    }
    return {};
}

// ========== HELPERS ==========

void BatchProcessor::setState(State newState) {
    if (state != newState) {
        state = newState;
        emit stateChanged(state);
    }
}

double BatchProcessor::parseDurationString(const QString& dur) {
    QStringList parts = dur.split(':');
    if (parts.size() == 3) {
        return parts[0].toInt() * 3600 + parts[1].toInt() * 60 + parts[2].toDouble();
    }
    return 0.0;
}
