#pragma once

#include <QList>
#include <QString>
#include <QStringList>
#include <memory>
#include "BatchProcessor.h"
#include "FileListWidget.h"

class FilterChain;
class OutputFilter;

/**
 * JobListBuilder - Pure logic: algorithm + file inputs → flat QList<JobInfo>
 * 
 * No UI, no signals, no state. Just functions.
 * 
 * Each algorithm takes file lists and chain context, returns ready-to-execute jobs.
 * The returned jobs have pre-computed commands and output paths — BatchProcessor
 * doesn't know or care how they were generated.
 * 
 * Algorithms:
 *   1. Sequential     — apply chain to every main file (N → N)
 *   2. Iterate        — re-process each file R times, output feeds back as input (N × R)
 *   3. Zip            — 1:1 pairing of main + aux (min(M,A) → min(M,A))
 *   4. BroadcastFixed — single aux applied to every main (N × 1 → N)
 *   5. BroadcastRandom — random aux per main file (N × rand(A) → N)
 *   6. Cartesian      — every main × every aux (N × M → N*M)
 *   7. CartesianTriple — N × M × P three-way product
 */
class JobListBuilder {
public:
    // Algorithm identifiers
    enum class Algorithm {
        Sequential = 1,      // ■ → OUT (N files)
        Iterate = 2,         // ■ → OUT → OUT' → OUT'' ... (N × R files)
        Zip = 3,             // ■ ⊕ ■ → OUT (min(M,A) files)
        BroadcastFixed = 4,  // ■ ⊗ ● → OUT (N files)
        BroadcastRandom = 5, // ■ ⊗ ▲ → OUT (N files)
        Cartesian = 6,       // ■ × ■ → OUT (N×M files)
        CartesianTriple = 7  // ■ × ■ × ■ → OUT (N×M×P files)
    };
    
    // Zip mismatch behavior when lists are different lengths
    enum class ZipMismatch {
        Truncate = 0,  // Use min(M, A) — default
        Cycle,         // Cycle shorter list to match longer
        Random         // Random from shorter list for remaining
    };
    
    // Size estimation result
    struct SizeEstimate {
        qint64 totalBytes = 0;
        double avgDurationSec = 0.0;
        QString formattedSize;     // e.g. "~1.2 GB"
    };
    
    // Algorithm description for UI
    struct AlgorithmInfo {
        Algorithm id;
        QString name;
        QString shortName;       // For compact display
        QString description;
        int requiredInputs;      // How many AudioInput filters needed (0 = none)
        bool available;
    };
    
    // Iterate algorithm constraints
    static constexpr int ITERATE_MIN_REPEATS = 2;
    static constexpr int ITERATE_MAX_REPEATS = 50;
    static constexpr int ITERATE_DEFAULT_REPEATS = 5;
    static constexpr double ITERATE_MIN_GAIN_DB = -12.0;
    static constexpr double ITERATE_MAX_GAIN_DB = 0.0;
    static constexpr double ITERATE_DEFAULT_GAIN_DB = -3.0;
    
    // Get info for all algorithms
    static QList<AlgorithmInfo> getAllAlgorithms();
    
    // Get info for a specific algorithm
    static AlgorithmInfo getAlgorithmInfo(Algorithm algo);
    
    // ========== Output Count Calculation ==========
    // Pure math — no command building, just count
    
    static int calculateOutputCount(Algorithm algo,
                                    int mainFileCount,
                                    int aux1FileCount = 0,
                                    int aux2FileCount = 0,
                                    ZipMismatch zipMode = ZipMismatch::Truncate,
                                    int iterateRepeats = ITERATE_DEFAULT_REPEATS);
    
    // ========== Job List Builders ==========
    // Each returns a flat QList<JobInfo> ready for BatchProcessor::start()
    
    // Algorithm 1: Sequential — every main file through the chain
    static QList<BatchProcessor::JobInfo> buildSequential(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& sidechainFiles);
    
    // Algorithm 2: Iterate — re-process each file R times through the chain.
    // "Photocopying a photocopy" — each pass accumulates the chain's effect.
    // A per-iteration gain reduction (dB) is injected before each pass to prevent
    // runaway clipping. All intermediate results are preserved.
    // If any iteration fails, remaining iterations for that file are aborted (isCascade).
    //
    // Output naming: drums_i01, drums_i02, drums_i03 ... drums_i10
    // Output count:  N files × R repeats
    static QList<BatchProcessor::JobInfo> buildIterate(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        int repeatCount,
        double gainReductionDb,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& sidechainFiles);
    
    // Algorithm 3: Zip — 1:1 pairing of main + aux1
    static QList<BatchProcessor::JobInfo> buildZip(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        const QList<FileListWidget::AudioFileInfo>& aux1Files,
        int aux1InputIndex,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& otherSidechainFiles,
        ZipMismatch mismatchMode = ZipMismatch::Truncate);
    
    // Algorithm 4: Broadcast Fixed — single aux applied to every main
    static QList<BatchProcessor::JobInfo> buildBroadcastFixed(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        const QString& fixedAuxFile,
        int aux1InputIndex,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& otherSidechainFiles);
    
    // Algorithm 5: Broadcast Random — random aux per main file
    static QList<BatchProcessor::JobInfo> buildBroadcastRandom(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        const QList<FileListWidget::AudioFileInfo>& aux1Files,
        int aux1InputIndex,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& otherSidechainFiles);
    
    // Algorithm 6: Cartesian — every main × every aux
    static QList<BatchProcessor::JobInfo> buildCartesian(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        const QList<FileListWidget::AudioFileInfo>& aux1Files,
        int aux1InputIndex,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& otherSidechainFiles);
    
    // Algorithm 7: Cartesian Triple — N × M × P
    static QList<BatchProcessor::JobInfo> buildCartesianTriple(
        const QList<FileListWidget::AudioFileInfo>& mainFiles,
        const QList<FileListWidget::AudioFileInfo>& aux1Files,
        const QList<FileListWidget::AudioFileInfo>& aux2Files,
        int aux1InputIndex,
        int aux2InputIndex,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        const QStringList& otherSidechainFiles);
    
    // ========== Size Estimation ==========
    
    static SizeEstimate estimateSize(
        int outputFileCount,
        double avgDurationSeconds,
        const QString& format,
        int sampleRate = 48000,
        int bitDepth = 24,
        int channels = 2);
    
    // Estimate bytes per second for a given format
    static double estimateBytesPerSecond(
        const QString& format,
        int sampleRate = 48000,
        int bitDepth = 24,
        int channels = 2);
    
    // Format byte count as human-readable string ("~1.2 GB")
    static QString formatBytes(qint64 bytes);
    
    // ========== Validation ==========
    
    static constexpr int TIER2_THRESHOLD = 10000;  // Double-confirm above this
    static constexpr int HARD_LIMIT = 1000000;     // Refuse above this
    
    // Check if output count is within safe limits
    // Returns empty string if OK, warning message if problematic
    static QString validateOutputCount(int count);
    
private:
    // Build output path for a job given combined basename
    static QString buildOutputPath(
        const QString& combinedBaseName,
        const QString& outputFolder,
        std::shared_ptr<FilterChain> filterChain);
    
    // Build FFmpeg command for a job
    static QString buildCommand(
        const QString& mainInputPath,
        const QStringList& sidechainFiles,
        const QString& outputPath,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions);
    
    // Build FFmpeg command with a volume filter prepended to the filter graph.
    // Used by Iterate to inject per-pass gain reduction.
    // Inserts "volume=XdB," at the start of -filter_complex, or adds "-af volume=XdB"
    // if no filter_complex is present.
    static QString buildCommandWithGain(
        const QString& mainInputPath,
        const QStringList& sidechainFiles,
        const QString& outputPath,
        std::shared_ptr<FilterChain> filterChain,
        const QList<int>& mutedPositions,
        double gainDb);
    
    // Prepare sidechain file list with a specific aux file at a specific index
    static QStringList buildSidechainList(
        const QStringList& otherSidechainFiles,
        const QString& aux1File,
        int aux1InputIndex,
        const QString& aux2File = QString(),
        int aux2InputIndex = -1);
    
    // Extract base name without extension from a file path or file name
    static QString baseName(const QString& fileNameOrPath);
};
