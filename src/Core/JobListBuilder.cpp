#include "JobListBuilder.h"
#include "Core/FilterChain.h"
#include "Filters/OutputFilter.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>

// ========== ALGORITHM INFO ==========

QList<JobListBuilder::AlgorithmInfo> JobListBuilder::getAllAlgorithms() {
    return {
        { Algorithm::Sequential,      "Sequential",       "Seq",  "Apply filter chain to every file",                    0, true  },
        { Algorithm::Iterate,         "Iterate",          "Iter", "Re-process each file R times (photocopy of photocopy)", 0, true  },
        { Algorithm::Zip,             "Zip (1:1)",        "Zip",  "Pair main and aux files by index position",           1, true  },
        { Algorithm::BroadcastFixed,  "Broadcast Fixed",  "Bcast","Single aux file applied to every main file",          1, true  },
        { Algorithm::BroadcastRandom, "Broadcast Random", "Rand", "Random aux file selected per main file",              1, true  },
        { Algorithm::Cartesian,       "Cartesian (N×M)",  "Cart", "Every main file combined with every aux file",        1, true  },
        { Algorithm::CartesianTriple, "Cartesian (N×M×P)","3Cart","Three-way Cartesian product",                         2, true  },
    };
}

JobListBuilder::AlgorithmInfo JobListBuilder::getAlgorithmInfo(Algorithm algo) {
    auto all = getAllAlgorithms();
    for (const auto& info : all) {
        if (info.id == algo) return info;
    }
    // Fallback
    return { Algorithm::Sequential, "Sequential", "Seq", "Apply filter chain to every file", 0, true };
}

// ========== OUTPUT COUNT ==========

int JobListBuilder::calculateOutputCount(Algorithm algo,
                                         int mainFileCount,
                                         int aux1FileCount,
                                         int aux2FileCount,
                                         ZipMismatch zipMode,
                                         int iterateRepeats) {
    switch (algo) {
        case Algorithm::Sequential:
            return mainFileCount;
            
        case Algorithm::Iterate:
            return mainFileCount * qBound(ITERATE_MIN_REPEATS, iterateRepeats, ITERATE_MAX_REPEATS);
            
        case Algorithm::Zip:
            if (zipMode == ZipMismatch::Truncate) {
                return (aux1FileCount > 0) ? qMin(mainFileCount, aux1FileCount) : 0;
            } else {
                // Cycle or Random: output count matches the longer list
                return qMax(mainFileCount, aux1FileCount);
            }
            
        case Algorithm::BroadcastFixed:
            return mainFileCount;
            
        case Algorithm::BroadcastRandom:
            return mainFileCount;
            
        case Algorithm::Cartesian:
            return mainFileCount * qMax(1, aux1FileCount);
            
        case Algorithm::CartesianTriple:
            return mainFileCount * qMax(1, aux1FileCount) * qMax(1, aux2FileCount);
    }
    
    return 0;
}

// ========== HELPERS ==========

QString JobListBuilder::baseName(const QString& fileNameOrPath) {
    return QFileInfo(fileNameOrPath).completeBaseName();
}

QString JobListBuilder::buildOutputPath(const QString& combinedBaseName,
                                        const QString& outputFolder,
                                        std::shared_ptr<FilterChain> filterChain) {
    QString suffix = "_processed";
    QString extension = "wav";
    
    if (filterChain) {
        auto lastFilter = filterChain->getFilter(filterChain->filterCount() - 1);
        if (auto* output = dynamic_cast<OutputFilter*>(lastFilter.get())) {
            suffix = output->getFilenameSuffix();
            extension = output->getFileExtension();
        }
    }
    
    QString outputFileName = combinedBaseName + suffix + "." + extension;
    return QDir(outputFolder).filePath(outputFileName);
}

QString JobListBuilder::buildCommand(const QString& mainInputPath,
                                     const QStringList& sidechainFiles,
                                     const QString& outputPath,
                                     std::shared_ptr<FilterChain> filterChain,
                                     const QList<int>& mutedPositions) {
    if (!filterChain) return QString();

    auto logSettings = LogSettings::fromQSettings();
    if (!sidechainFiles.isEmpty()) {
        return filterChain->buildCompleteCommand(
            mainInputPath, sidechainFiles, outputPath, mutedPositions, logSettings);
    } else {
        return filterChain->buildCompleteCommand(
            mainInputPath, outputPath, mutedPositions, logSettings);
    }
}

QString JobListBuilder::buildCommandWithGain(const QString& mainInputPath,
                                              const QStringList& sidechainFiles,
                                              const QString& outputPath,
                                              std::shared_ptr<FilterChain> filterChain,
                                              const QList<int>& mutedPositions,
                                              double gainDb) {
    // Build the normal command first
    QString cmd = buildCommand(mainInputPath, sidechainFiles, outputPath,
                               filterChain, mutedPositions);
    
    if (cmd.isEmpty() || qFuzzyCompare(gainDb, 0.0)) {
        return cmd;  // No gain adjustment needed
    }
    
    // Format gain value: volume filter accepts dB with "dB" suffix
    // e.g. "volume=-3dB"
    QString gainFilter = QString("volume=%1dB").arg(gainDb, 0, 'f', 1);
    
    // Strategy: inject gain filter at the start of the filter graph.
    //
    // Case 1: Command has -filter_complex "..."
    //   Insert gainFilter + "," right after the opening quote.
    //   BUT: the filter graph uses labeled pads like [0:a] and [1:a].
    //   The gain filter should only apply to the main input (pad [0:a]).
    //   So we need to prepend: [0:a]volume=-3dB[vol];  and then replace
    //   the first [0:a] reference in the rest of the graph with [vol].
    //
    //   Actually, the simplest approach that works for FFAB's filter graph structure:
    //   The chain always starts with [0:a] going into the first filter.
    //   We can inject our volume as the very first filter in the semicolon chain.
    //
    // Case 2: No -filter_complex (rare — chain with only I/O filters)
    //   Add "-af volume=-3dB" before the output file.
    
    // Check for -filter_complex
    int fcIdx = cmd.indexOf("-filter_complex \"");
    if (fcIdx >= 0) {
        // Find the opening quote of the filter graph
        int graphStart = cmd.indexOf('"', fcIdx) + 1;
        
        // The FFAB filter graph typically starts with: [0:a]firstfilter...
        // We want to inject: [0:a]volume=-3dB[_vol];[_vol]  replacing the initial [0:a]
        // This is cleanest because it preserves the rest of the graph intact.
        
        // Check if graph starts with [0:a] (FFAB convention)
        QString graphPrefix = cmd.mid(graphStart, 5);  // "[0:a]"
        if (graphPrefix == "[0:a]") {
            // Inject: [0:a]volume=-3dB[_igain];[_igain]
            // replacing the first [0:a] that the next filter would consume
            QString injection = QString("[0:a]%1[_igain];[_igain]").arg(gainFilter);
            cmd.remove(graphStart, 5);       // Remove original [0:a]
            cmd.insert(graphStart, injection); // Insert gain stage + relabeled pad
        } else {
            // Graph doesn't start with [0:a] — just prepend the filter with semicolon
            // This is a fallback; less clean but still functional
            cmd.insert(graphStart, gainFilter + ",");
        }
    } else {
        // No filter_complex — add -af before output file
        // Find the last quoted argument (the output path)
        int lastQuoteEnd = cmd.lastIndexOf('"');
        int lastQuoteStart = cmd.lastIndexOf('"', lastQuoteEnd - 1);
        if (lastQuoteStart > 0) {
            cmd.insert(lastQuoteStart, QString("-af %1 ").arg(gainFilter));
        }
    }
    
    return cmd;
}

QStringList JobListBuilder::buildSidechainList(const QStringList& otherSidechainFiles,
                                               const QString& aux1File,
                                               int aux1InputIndex,
                                               const QString& aux2File,
                                               int aux2InputIndex) {
    // Start with a copy of existing sidechain files (may have entries from other AudioInputFilters)
    QStringList result = otherSidechainFiles;
    
    // AudioInputFilter indices are 1-based, but the sidechain list is 0-based
    // (AudioInput index 1 = sidechain list position 0, etc.)
    int aux1ListIdx = aux1InputIndex - 1;
    
    // Ensure list is large enough
    while (result.size() <= aux1ListIdx) {
        result.append(QString());
    }
    
    // Place aux1 file at the correct position
    result[aux1ListIdx] = aux1File;
    
    // Handle aux2 if provided
    if (aux2InputIndex > 0 && !aux2File.isEmpty()) {
        int aux2ListIdx = aux2InputIndex - 1;
        while (result.size() <= aux2ListIdx) {
            result.append(QString());
        }
        result[aux2ListIdx] = aux2File;
    }
    
    return result;
}

// ========== ALGORITHM 1: SEQUENTIAL ==========

QList<BatchProcessor::JobInfo> JobListBuilder::buildSequential(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& sidechainFiles) {
    
    QList<BatchProcessor::JobInfo> jobs;
    jobs.reserve(mainFiles.size());
    
    for (const auto& file : mainFiles) {
        BatchProcessor::JobInfo job;
        job.inputFile = file;
        
        QString combined = baseName(file.fileName);
        job.combinedBaseName = combined;
        job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
        job.sidechainFiles = sidechainFiles;
        job.command = buildCommand(file.filePath, sidechainFiles, job.outputPath,
                                   filterChain, mutedPositions);
        
        jobs.append(job);
    }
    
    return jobs;
}

// ========== ALGORITHM 2: ITERATE ==========
//
// "Photocopying a photocopy" — re-process each file R times through the chain.
//
// For each main file:
//   Pass 1: original.wav  → [gain] → [chain] → drums_i01_processed.wav
//   Pass 2: drums_i01...  → [gain] → [chain] → drums_i02_processed.wav
//   Pass 3: drums_i02...  → [gain] → [chain] → drums_i03_processed.wav
//   ...
//
// The gain reduction is applied as a volume filter injected at the start of the
// filter graph. This prevents each generation from clipping — the interesting
// part is the timbral degradation, not the volume accumulation.
//
// All intermediate results are kept so the user can audition the progression
// and pick the iteration they like best.
//
// Jobs for each file are marked isCascade = true so that if one iteration fails,
// the remaining iterations for that file are skipped (the output chain is broken).
// However, the next file's iterations start fresh.

QList<BatchProcessor::JobInfo> JobListBuilder::buildIterate(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    int repeatCount,
    double gainReductionDb,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& sidechainFiles) {
    
    QList<BatchProcessor::JobInfo> jobs;
    
    if (mainFiles.isEmpty()) return jobs;
    
    // Clamp parameters
    repeatCount = qBound(ITERATE_MIN_REPEATS, repeatCount, ITERATE_MAX_REPEATS);
    gainReductionDb = qBound(ITERATE_MIN_GAIN_DB, gainReductionDb, ITERATE_MAX_GAIN_DB);
    
    int totalJobs = mainFiles.size() * repeatCount;
    jobs.reserve(totalJobs);
    
    // Width of iteration number for formatting (e.g. "01" for ≤99, "001" for ≥100)
    int numWidth = (repeatCount >= 100) ? 3 : 2;
    
    for (const auto& mainFile : mainFiles) {
        QString fileBase = baseName(mainFile.fileName);
        QString previousOutputPath;
        
        for (int r = 0; r < repeatCount; ++r) {
            BatchProcessor::JobInfo job;
            job.isCascade = true;  // Abort remaining iterations on failure
            
            // Iteration label: i01, i02, ...
            QString iterLabel = QString("i%1").arg(r + 1, numWidth, 10, QChar('0'));
            QString combined = fileBase + "_" + iterLabel;
            
            job.combinedBaseName = combined;
            job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
            job.sidechainFiles = sidechainFiles;
            
            // Determine the input for this pass
            QString inputPath;
            if (r == 0) {
                // First pass: use the original file
                inputPath = mainFile.filePath;
                job.inputFile = mainFile;
            } else {
                // Subsequent passes: feed previous output back as input
                inputPath = previousOutputPath;
                
                // Create a synthetic AudioFileInfo for the intermediate file
                FileListWidget::AudioFileInfo iterFile;
                iterFile.filePath = previousOutputPath;
                iterFile.fileName = QFileInfo(previousOutputPath).fileName();
                iterFile.duration = mainFile.duration;  // Approximate
                iterFile.sampleRate = mainFile.sampleRate;
                iterFile.channels = mainFile.channels;
                job.inputFile = iterFile;
            }
            
            // Build command with gain reduction injected
            if (qFuzzyCompare(gainReductionDb, 0.0)) {
                // No gain reduction — pure chaos mode
                job.command = buildCommand(inputPath, sidechainFiles, job.outputPath,
                                           filterChain, mutedPositions);
            } else {
                // Inject volume filter for gain staging
                job.command = buildCommandWithGain(inputPath, sidechainFiles, job.outputPath,
                                                    filterChain, mutedPositions,
                                                    gainReductionDb);
            }
            
            previousOutputPath = job.outputPath;
            jobs.append(job);
        }
    }
    
    qDebug() << "Iterate: built" << jobs.size() << "jobs"
             << "(" << mainFiles.size() << "files ×" << repeatCount << "repeats,"
             << gainReductionDb << "dB/pass)";
    
    return jobs;
}

// ========== ALGORITHM 3: ZIP ==========

QList<BatchProcessor::JobInfo> JobListBuilder::buildZip(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    const QList<FileListWidget::AudioFileInfo>& aux1Files,
    int aux1InputIndex,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& otherSidechainFiles,
    ZipMismatch mismatchMode) {
    
    QList<BatchProcessor::JobInfo> jobs;
    
    if (mainFiles.isEmpty() || aux1Files.isEmpty()) return jobs;
    
    int outputCount;
    switch (mismatchMode) {
        case ZipMismatch::Truncate:
            outputCount = qMin(mainFiles.size(), aux1Files.size());
            break;
        case ZipMismatch::Cycle:
        case ZipMismatch::Random:
            outputCount = qMax(mainFiles.size(), aux1Files.size());
            break;
    }
    
    jobs.reserve(outputCount);
    
    for (int i = 0; i < outputCount; ++i) {
        // Main file: cycle if shorter
        int mainIdx = (i < mainFiles.size()) ? i : (i % mainFiles.size());
        const auto& mainFile = mainFiles[mainIdx];
        
        // Aux file: depends on mismatch mode
        int auxIdx;
        switch (mismatchMode) {
            case ZipMismatch::Truncate:
                auxIdx = i;  // Both within bounds (capped by outputCount)
                break;
            case ZipMismatch::Cycle:
                auxIdx = i % aux1Files.size();
                break;
            case ZipMismatch::Random:
                auxIdx = (i < aux1Files.size()) ? i 
                    : QRandomGenerator::global()->bounded(aux1Files.size());
                break;
        }
        const auto& auxFile = aux1Files[auxIdx];
        
        BatchProcessor::JobInfo job;
        job.inputFile = mainFile;
        
        QString combined = baseName(mainFile.fileName) + "_" + baseName(auxFile.fileName);
        job.combinedBaseName = combined;
        job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
        job.sidechainFiles = buildSidechainList(otherSidechainFiles, auxFile.filePath, aux1InputIndex);
        job.command = buildCommand(mainFile.filePath, job.sidechainFiles, job.outputPath,
                                   filterChain, mutedPositions);
        
        jobs.append(job);
    }
    
    return jobs;
}

// ========== ALGORITHM 4: BROADCAST FIXED ==========

QList<BatchProcessor::JobInfo> JobListBuilder::buildBroadcastFixed(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    const QString& fixedAuxFile,
    int aux1InputIndex,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& otherSidechainFiles) {
    
    QList<BatchProcessor::JobInfo> jobs;
    jobs.reserve(mainFiles.size());
    
    QString auxBase = baseName(fixedAuxFile);
    
    for (const auto& mainFile : mainFiles) {
        BatchProcessor::JobInfo job;
        job.inputFile = mainFile;
        
        QString combined = baseName(mainFile.fileName) + "_" + auxBase;
        job.combinedBaseName = combined;
        job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
        job.sidechainFiles = buildSidechainList(otherSidechainFiles, fixedAuxFile, aux1InputIndex);
        job.command = buildCommand(mainFile.filePath, job.sidechainFiles, job.outputPath,
                                   filterChain, mutedPositions);
        
        jobs.append(job);
    }
    
    return jobs;
}

// ========== ALGORITHM 5: BROADCAST RANDOM ==========

QList<BatchProcessor::JobInfo> JobListBuilder::buildBroadcastRandom(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    const QList<FileListWidget::AudioFileInfo>& aux1Files,
    int aux1InputIndex,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& otherSidechainFiles) {
    
    QList<BatchProcessor::JobInfo> jobs;
    
    if (mainFiles.isEmpty() || aux1Files.isEmpty()) return jobs;
    
    jobs.reserve(mainFiles.size());
    
    for (const auto& mainFile : mainFiles) {
        // Pick a random aux file
        int randIdx = QRandomGenerator::global()->bounded(aux1Files.size());
        const auto& auxFile = aux1Files[randIdx];
        
        BatchProcessor::JobInfo job;
        job.inputFile = mainFile;
        
        // Include "rnd" marker so user knows randomness was involved
        QString combined = baseName(mainFile.fileName) + "_" + baseName(auxFile.fileName);
        job.combinedBaseName = combined;
        job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
        job.sidechainFiles = buildSidechainList(otherSidechainFiles, auxFile.filePath, aux1InputIndex);
        job.command = buildCommand(mainFile.filePath, job.sidechainFiles, job.outputPath,
                                   filterChain, mutedPositions);
        
        jobs.append(job);
    }
    
    return jobs;
}

// ========== ALGORITHM 6: CARTESIAN ==========

QList<BatchProcessor::JobInfo> JobListBuilder::buildCartesian(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    const QList<FileListWidget::AudioFileInfo>& aux1Files,
    int aux1InputIndex,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& otherSidechainFiles) {
    
    QList<BatchProcessor::JobInfo> jobs;
    
    if (mainFiles.isEmpty() || aux1Files.isEmpty()) return jobs;
    
    int total = mainFiles.size() * aux1Files.size();
    jobs.reserve(total);
    
    for (const auto& mainFile : mainFiles) {
        for (const auto& auxFile : aux1Files) {
            BatchProcessor::JobInfo job;
            job.inputFile = mainFile;
            
            QString combined = baseName(mainFile.fileName) + "_" + baseName(auxFile.fileName);
            job.combinedBaseName = combined;
            job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
            job.sidechainFiles = buildSidechainList(otherSidechainFiles, auxFile.filePath, aux1InputIndex);
            job.command = buildCommand(mainFile.filePath, job.sidechainFiles, job.outputPath,
                                       filterChain, mutedPositions);
            
            jobs.append(job);
        }
    }
    
    return jobs;
}

// ========== ALGORITHM 7: CARTESIAN TRIPLE ==========

QList<BatchProcessor::JobInfo> JobListBuilder::buildCartesianTriple(
    const QList<FileListWidget::AudioFileInfo>& mainFiles,
    const QList<FileListWidget::AudioFileInfo>& aux1Files,
    const QList<FileListWidget::AudioFileInfo>& aux2Files,
    int aux1InputIndex,
    int aux2InputIndex,
    const QString& outputFolder,
    std::shared_ptr<FilterChain> filterChain,
    const QList<int>& mutedPositions,
    const QStringList& otherSidechainFiles) {
    
    QList<BatchProcessor::JobInfo> jobs;
    
    if (mainFiles.isEmpty() || aux1Files.isEmpty() || aux2Files.isEmpty()) return jobs;
    
    int total = mainFiles.size() * aux1Files.size() * aux2Files.size();
    jobs.reserve(total);
    
    for (const auto& mainFile : mainFiles) {
        for (const auto& aux1File : aux1Files) {
            for (const auto& aux2File : aux2Files) {
                BatchProcessor::JobInfo job;
                job.inputFile = mainFile;
                
                QString combined = baseName(mainFile.fileName) + "_" +
                                   baseName(aux1File.fileName) + "_" +
                                   baseName(aux2File.fileName);
                job.combinedBaseName = combined;
                job.outputPath = buildOutputPath(combined, outputFolder, filterChain);
                job.sidechainFiles = buildSidechainList(otherSidechainFiles,
                                                        aux1File.filePath, aux1InputIndex,
                                                        aux2File.filePath, aux2InputIndex);
                job.command = buildCommand(mainFile.filePath, job.sidechainFiles, job.outputPath,
                                           filterChain, mutedPositions);
                
                jobs.append(job);
            }
        }
    }
    
    return jobs;
}

// ========== SIZE ESTIMATION ==========

double JobListBuilder::estimateBytesPerSecond(const QString& format,
                                              int sampleRate,
                                              int bitDepth,
                                              int channels) {
    QString fmt = format.toLower();
    
    if (fmt == "wav" || fmt == "aiff" || fmt == "caf") {
        return sampleRate * (bitDepth / 8.0) * channels;
    } else if (fmt == "mp3") {
        return 40000.0;   // ~320kbps
    } else if (fmt == "aac") {
        return 32000.0;   // ~256kbps
    } else if (fmt == "flac") {
        return sampleRate * (bitDepth / 8.0) * channels * 0.55;  // ~55% compression
    } else if (fmt == "ogg" || fmt == "opus") {
        return 32000.0;   // ~256kbps
    }
    
    // Fallback: uncompressed
    return sampleRate * (bitDepth / 8.0) * channels;
}

JobListBuilder::SizeEstimate JobListBuilder::estimateSize(
    int outputFileCount,
    double avgDurationSeconds,
    const QString& format,
    int sampleRate,
    int bitDepth,
    int channels) {
    
    SizeEstimate est;
    est.avgDurationSec = avgDurationSeconds;
    
    double bytesPerSec = estimateBytesPerSecond(format, sampleRate, bitDepth, channels);
    est.totalBytes = static_cast<qint64>(outputFileCount * avgDurationSeconds * bytesPerSec);
    est.formattedSize = formatBytes(est.totalBytes);
    
    return est;
}

QString JobListBuilder::formatBytes(qint64 bytes) {
    if (bytes < 0) return "~0 B";
    
    double val = static_cast<double>(bytes);
    
    if (val < 1024.0) {
        return QString("~%1 B").arg(qRound(val));
    } else if (val < 1024.0 * 1024.0) {
        return QString("~%1 KB").arg(val / 1024.0, 0, 'f', 1);
    } else if (val < 1024.0 * 1024.0 * 1024.0) {
        return QString("~%1 MB").arg(val / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        return QString("~%1 GB").arg(val / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
    }
}

// ========== VALIDATION ==========

QString JobListBuilder::validateOutputCount(int count) {
    if (count > HARD_LIMIT) {
        return QString("Output count (%1) exceeds the hard limit of %2 files. "
                       "Please reduce your input file counts.")
            .arg(QLocale().toString(count))
            .arg(QLocale().toString(HARD_LIMIT));
    }
    if (count > TIER2_THRESHOLD) {
        return QString("This batch will produce %1 files. Are you absolutely sure?")
            .arg(QLocale().toString(count));
    }
    return QString();  // Empty = OK
}
