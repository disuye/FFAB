#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <memory>
#include <vector>
#include "FilterGraph.h"
#include "DAGCommandBuilder.h"

class BaseFilter;
class InputFilter;
class OutputFilter;
class MultiOutputFilter;
class FFShowwavespic;  // Forward declaration for waveform image filter
class FFAnullsink;     // Forward declaration for null sink filter

/**
 * LogSettings - FFmpeg logging flags configuration
 *
 * Passed to command builders to control -loglevel, -stats, -hide_banner, etc.
 * Default-constructed values match FFAB's current hardcoded defaults.
 */
struct LogSettings {
    bool showBanner = false;             // false = -hide_banner (default)
    QString logLevel = "error";          // -loglevel value
    bool showStats = true;               // true = -stats, false = -nostats

    // Build the flags portion of the command
    QString buildFlags() const;

    // Hardcoded safe defaults for preview — never changes
    static LogSettings preview();

    // Read user preferences from QSettings
    static LogSettings fromQSettings();
};

/**
 * FilterChain - Main audio processing chain
 *
 * Manages a linear chain of filters: INPUT → filters → OUTPUT
 *
 * Multi-output filters (asplit, channelsplit, etc.) own their sub-chains internally.
 * This class provides delegation methods to add/remove filters in those sub-chains
 * while maintaining the global filter ID counter.
 *
 * Architecture:
 *   Main chain: INPUT → [filter1] → [asplit] → [filter2] → OUTPUT
 *                                      ↓
 *                              (internal sub-chains)
 *                              stream 1: [eq] → [volume]
 *                              stream 2: [reverb]
 *                              stream 3: (empty)
 */
class FilterChain {
public:
    FilterChain();
    ~FilterChain();

    // ========== Main Chain Operations ==========

    void addFilter(std::shared_ptr<BaseFilter> filter, int position = -1);
    void removeFilter(int position);
    void moveFilter(int fromPos, int toPos);

    std::shared_ptr<BaseFilter> getFilter(int position) const;
    const std::vector<std::shared_ptr<BaseFilter>>& getAllFilters() const { return filters; }

    int filterCount() const { return filters.size(); }

    // ========== Sub-Chain Operations (for MultiOutputFilter) ==========

    // Add a filter to a multi-output filter's stream sub-chain
    // mainChainPos: position of the MultiOutputFilter in the main chain
    // streamIndex: which stream (1-7)
    // filter: the filter to add
    void addFilterToStream(int mainChainPos, int streamIndex, std::shared_ptr<BaseFilter> filter);

    // Remove a filter from a multi-output filter's stream sub-chain
    void removeFilterFromStream(int mainChainPos, int streamIndex, int filterPos);

    // Move a filter within a multi-output filter's stream sub-chain
    void moveFilterInStream(int mainChainPos, int streamIndex, int fromPos, int toPos);

    // Get a filter from a multi-output filter's stream sub-chain
    std::shared_ptr<BaseFilter> getFilterFromStream(int mainChainPos, int streamIndex, int filterPos) const;

    // Get the entire sub-chain for a stream
    const std::vector<std::shared_ptr<BaseFilter>>& getSubChain(int mainChainPos, int streamIndex) const;

    // Get filter count for a stream
    int getStreamFilterCount(int mainChainPos, int streamIndex) const;

    // Get active streams for a multi-output filter
    QSet<int> getActiveStreamsForMultiOutput(int mainChainPos) const;

    // Get the MultiOutputFilter at a position (returns nullptr if not a multi-output filter)
    MultiOutputFilter* getMultiOutputFilter(int position) const;

    // ========== FFmpeg Command Building ==========

    QString buildCompleteCommand(const QString& inputFile, const QString& outputFile,
                                 const LogSettings& logSettings = LogSettings()) const;
    QString buildFilterFlags() const;

    // Overloaded versions that skip muted filters
    QString buildCompleteCommand(const QString& inputFile, const QString& outputFile,
                                 const QList<int>& mutedPositions,
                                 const LogSettings& logSettings = LogSettings()) const;
    QString buildFilterFlags(const QList<int>& mutedPositions) const;

    // Overloaded version with sidechain inputs
    QString buildCompleteCommand(const QString& inputFile,
                                 const QStringList& sidechainFiles,
                                 const QString& outputFile,
                                 const LogSettings& logSettings = LogSettings()) const;
    QString buildCompleteCommand(const QString& inputFile,
                                 const QStringList& sidechainFiles,
                                 const QString& outputFile,
                                 const QList<int>& mutedPositions,
                                 const LogSettings& logSettings = LogSettings()) const;

    // ========== Preview Command Generation ==========

    // Build command for preview (aux outputs discarded to null muxer)
    // Uses hardcoded safe flags by default. When preview logging is enabled,
    // pass user's LogSettings to capture analysis filter output.
    QString buildPreviewCommand(const QString& inputFile,
                                const QStringList& sidechainFiles,
                                const QString& outputFile,
                                const QList<int>& mutedPositions,
                                const LogSettings& logSettings = LogSettings()) const;

    // Static helpers for command formatting/parsing
    static QString formatCommandForDisplay(const QString& command, const QString& ffmpegPath);
    static QStringList parseCommandToArgs(const QString& command);

    // ========== Serialization ==========

    void saveToJSON(const QString& filepath) const;
    bool loadFromJSON(const QString& filepath);

    QJsonObject toJSON() const;
    bool fromJSON(const QJsonObject& json);

    // ========== Filter Factory ==========

    static std::shared_ptr<BaseFilter> createFilterByType(const QString& type);

    // ========== Audio Input Management ==========

    void updateAudioInputIndices();
    void updateMultiInputFilterIndices();
    int getRequiredAudioInputCount() const;
    std::vector<int> getAudioInputIndices() const;

    // Get all AudioInputFilters (main chain + sub-chains) in index order
    std::vector<class AudioInputFilter*> getAllAudioInputFilters() const;

    // ========== Utilities ==========

    // Stream label helper - converts filter ID to 4-char hex (e.g., 42 → "002A")
    QString getFilterHexLabel(int filterId) const;

    // Get the next filter ID (for external use if needed)
    int getNextFilterId() const { return m_nextFilterId; }

    // Assign an ID to a filter (increments counter)
    void assignFilterId(std::shared_ptr<BaseFilter> filter);

private:
    // Helper structure for tracking aux outputs
    struct AuxOutputInfo {
        class AuxOutputFilter* filter = nullptr;
        QString streamLabel;        // e.g., "[0001-auxOut]"
        int mainChainPosition = -1;
    };

    // Build output file path for an aux output
    QString buildAuxOutputPath(const QString& inputFile, class AuxOutputFilter* auxOut, int auxIndex) const;

    // Build output file path for a waveform image filter
    QString buildWaveformOutputPath(const QString& inputFile, FFShowwavespic* waveform) const;

    // Check if the main chain ends with a sink filter (no audio output)
    bool endsWithSinkFilter(const QList<int>& mutedPositions = QList<int>()) const;

    // DAG infrastructure (Phase A)
    bool isLinearChain(const QList<int>& mutedPositions) const;
    QString buildFilterFlagsDAG(const QList<int>& mutedPositions) const;

    std::vector<std::shared_ptr<BaseFilter>> filters;

    InputFilter* inputFilter = nullptr;
    OutputFilter* outputFilter = nullptr;

    int m_nextFilterId = 0;  // Sequential ID counter for all filters (main chain + sub-chains)

    // Empty vector for out-of-bounds access
    static const std::vector<std::shared_ptr<BaseFilter>> s_emptyChain;
};
