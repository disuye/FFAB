#pragma once

#include "BaseFilter.h"
#include <vector>
#include <array>
#include <memory>
#include <QSet>
#include <QJsonArray>

/**
 * MultiOutputFilter - Base class for filters that split audio into multiple output streams
 * 
 * Filters like asplit, channelsplit, asegment, acrossover can create multiple outputs,
 * each of which can have its own processing chain (sub-chain).
 * 
 * This class owns the sub-chains internally - no more position-based ownership!
 * 
 * Stream numbering:
 * - Stream 0: Reserved for "thru" / main chain passthrough (not used by sub-chains)
 * - Streams 1-7: Split output streams, each can have filters
 * 
 * Maximum 7 split streams (+ thru = 8 total) to match FFmpeg asplit limit
 * and keep UI manageable with [1][2][3][4][5][6][7] buttons.
 */
class MultiOutputFilter : public BaseFilter {
    Q_OBJECT
    
public:
    static constexpr int MAX_STREAMS = 8;  // 0 = thru, 1-7 = splits
    
    MultiOutputFilter();
    virtual ~MultiOutputFilter() = default;
    
    // ========== Sub-chain Management ==========
    
    // Add a filter to a stream's sub-chain (appends to end)
    void addFilterToStream(int streamIndex, std::shared_ptr<BaseFilter> filter);
    
    // Insert a filter at specific position in a stream's sub-chain
    void insertFilterInStream(int streamIndex, int position, std::shared_ptr<BaseFilter> filter);
    
    // Remove a filter from a stream's sub-chain
    void removeFilterFromStream(int streamIndex, int position);
    
    // Move a filter within a stream's sub-chain
    void moveFilterInStream(int streamIndex, int fromPos, int toPos);
    
    // Get a filter from a stream's sub-chain
    std::shared_ptr<BaseFilter> getFilterFromStream(int streamIndex, int position) const;
    
    // Get the entire sub-chain for a stream
    const std::vector<std::shared_ptr<BaseFilter>>& getSubChain(int streamIndex) const;
    
    // Get filter count for a stream
    int getStreamFilterCount(int streamIndex) const;
    
    // ========== Query Methods ==========
    
    // Get set of stream indices that have at least one filter
    QSet<int> getActiveStreams() const;
    
    // Check if a specific stream has filters
    bool streamHasFilters(int streamIndex) const;
    
    // Get total filter count across all streams
    int getTotalSubchainFilterCount() const;
    
    // ========== Abstract Methods (implement in derived classes) ==========
    
    // How many output streams does this filter create? (e.g., asplit returns numSplits)
    virtual int getNumOutputStreams() const = 0;
    
    // ========== Serialization ==========
    
    // Serialize sub-chains to JSON (call from derived class toJSON)
    void subChainsToJSON(QJsonObject& json) const;
    
    // Deserialize sub-chains from JSON (call from derived class fromJSON)
    void subChainsFromJSON(const QJsonObject& json);
    
protected:
    // Storage for sub-chains: index 0 unused (thru), 1-7 for split streams
    std::array<std::vector<std::shared_ptr<BaseFilter>>, MAX_STREAMS> m_subChains;
    
private:
    // Empty vector for out-of-bounds access
    static const std::vector<std::shared_ptr<BaseFilter>> s_emptyChain;
};
