#include "MultiOutputFilter.h"
#include <QJsonArray>
#include <QDebug>

// Static empty chain for out-of-bounds access
const std::vector<std::shared_ptr<BaseFilter>> MultiOutputFilter::s_emptyChain;

MultiOutputFilter::MultiOutputFilter() {
    // All stream chains start empty
}

void MultiOutputFilter::addFilterToStream(int streamIndex, std::shared_ptr<BaseFilter> filter) {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        qWarning() << "MultiOutputFilter::addFilterToStream: Invalid stream index" << streamIndex;
        return;
    }
    if (!filter) {
        qWarning() << "MultiOutputFilter::addFilterToStream: Null filter";
        return;
    }
    
    m_subChains[streamIndex].push_back(filter);
    qDebug() << "Added filter to stream" << streamIndex << "- now has" << m_subChains[streamIndex].size() << "filters";
}

void MultiOutputFilter::insertFilterInStream(int streamIndex, int position, std::shared_ptr<BaseFilter> filter) {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        qWarning() << "MultiOutputFilter::insertFilterInStream: Invalid stream index" << streamIndex;
        return;
    }
    if (!filter) {
        qWarning() << "MultiOutputFilter::insertFilterInStream: Null filter";
        return;
    }
    
    auto& chain = m_subChains[streamIndex];
    
    // Clamp position to valid range
    if (position < 0) position = 0;
    if (position > static_cast<int>(chain.size())) position = chain.size();
    
    chain.insert(chain.begin() + position, filter);
    qDebug() << "Inserted filter at position" << position << "in stream" << streamIndex;
}

void MultiOutputFilter::removeFilterFromStream(int streamIndex, int position) {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        qWarning() << "MultiOutputFilter::removeFilterFromStream: Invalid stream index" << streamIndex;
        return;
    }
    
    auto& chain = m_subChains[streamIndex];
    
    if (position < 0 || position >= static_cast<int>(chain.size())) {
        qWarning() << "MultiOutputFilter::removeFilterFromStream: Invalid position" << position 
                   << "for stream" << streamIndex << "with" << chain.size() << "filters";
        return;
    }
    
    chain.erase(chain.begin() + position);
    qDebug() << "Removed filter at position" << position << "from stream" << streamIndex;
}

void MultiOutputFilter::moveFilterInStream(int streamIndex, int fromPos, int toPos) {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        qWarning() << "MultiOutputFilter::moveFilterInStream: Invalid stream index" << streamIndex;
        return;
    }
    
    auto& chain = m_subChains[streamIndex];
    int chainSize = static_cast<int>(chain.size());
    
    if (fromPos < 0 || fromPos >= chainSize || toPos < 0 || toPos >= chainSize) {
        qWarning() << "MultiOutputFilter::moveFilterInStream: Invalid positions" 
                   << fromPos << "->" << toPos << "for chain size" << chainSize;
        return;
    }
    
    if (fromPos == toPos) return;
    
    auto filter = chain[fromPos];
    chain.erase(chain.begin() + fromPos);
    chain.insert(chain.begin() + toPos, filter);
    
    qDebug() << "Moved filter in stream" << streamIndex << "from" << fromPos << "to" << toPos;
}

std::shared_ptr<BaseFilter> MultiOutputFilter::getFilterFromStream(int streamIndex, int position) const {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        return nullptr;
    }
    
    const auto& chain = m_subChains[streamIndex];
    if (position < 0 || position >= static_cast<int>(chain.size())) {
        return nullptr;
    }
    
    return chain[position];
}

const std::vector<std::shared_ptr<BaseFilter>>& MultiOutputFilter::getSubChain(int streamIndex) const {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        return s_emptyChain;
    }
    return m_subChains[streamIndex];
}

int MultiOutputFilter::getStreamFilterCount(int streamIndex) const {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        return 0;
    }
    return static_cast<int>(m_subChains[streamIndex].size());
}

QSet<int> MultiOutputFilter::getActiveStreams() const {
    QSet<int> active;
    for (int i = 1; i < MAX_STREAMS; ++i) {
        if (!m_subChains[i].empty()) {
            active.insert(i);
        }
    }
    return active;
}

bool MultiOutputFilter::streamHasFilters(int streamIndex) const {
    if (streamIndex < 1 || streamIndex >= MAX_STREAMS) {
        return false;
    }
    return !m_subChains[streamIndex].empty();
}

int MultiOutputFilter::getTotalSubchainFilterCount() const {
    int total = 0;
    for (int i = 1; i < MAX_STREAMS; ++i) {
        total += static_cast<int>(m_subChains[i].size());
    }
    return total;
}

void MultiOutputFilter::subChainsToJSON(QJsonObject& json) const {
    QJsonArray streamsArray;
    
    for (int streamIdx = 1; streamIdx < MAX_STREAMS; ++streamIdx) {
        const auto& chain = m_subChains[streamIdx];
        
        if (chain.empty()) {
            // Still save empty array to preserve stream indices
            streamsArray.append(QJsonArray());
        } else {
            QJsonArray chainArray;
            for (const auto& filter : chain) {
                QJsonObject filterObj;
                filter->toJSON(filterObj);
                filterObj["filter_id"] = filter->getFilterId();
                filterObj["use_custom_output"] = filter->usesCustomOutputStream();
                chainArray.append(filterObj);
            }
            streamsArray.append(chainArray);
        }
    }
    
    json["stream_chains"] = streamsArray;
}

void MultiOutputFilter::subChainsFromJSON(const QJsonObject& json) {
    // Clear existing chains
    for (int i = 1; i < MAX_STREAMS; ++i) {
        m_subChains[i].clear();
    }
    
    if (!json.contains("stream_chains")) {
        return;
    }
    
    QJsonArray streamsArray = json["stream_chains"].toArray();
    
    for (int streamIdx = 0; streamIdx < streamsArray.size() && streamIdx < MAX_STREAMS - 1; ++streamIdx) {
        QJsonArray chainArray = streamsArray[streamIdx].toArray();
        int actualStreamIdx = streamIdx + 1;  // streamsArray[0] = stream 1
        
        for (int filterIdx = 0; filterIdx < chainArray.size(); ++filterIdx) {
            QJsonObject filterObj = chainArray[filterIdx].toObject();
            QString type = filterObj["type"].toString();
            
            // Use FilterChain's factory (forward declaration issue - we'll handle this)
            // For now, we need to include FilterChain or have a static factory
            // This will be resolved by having FilterChain call this method and pass a factory
            
            // Actually, we'll defer filter creation to FilterChain::fromJSON
            // This method just stores the raw JSON for later processing
        }
    }
    
    // Note: Actual filter instantiation happens in FilterChain::fromJSON
    // which has access to createFilterByType(). We store the JSON and let
    // FilterChain handle the instantiation.
}
