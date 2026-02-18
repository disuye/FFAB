#pragma once

#include "FilterGraph.h"
#include <QString>
#include <QList>
#include <QMap>
#include <functional>

namespace DAG {

class DAGCommandBuilder {
public:
    // Build the filter_complex string from a linear FilterGraph.
    //
    // graph:                  The filter graph (linear chain).
    // mutedPositions:         Chain positions to skip.
    // nodeIdToChainPosition:  Maps nodeId -> position in the original filters vector.
    // hexLabelFunc:           Converts filter ID to 4-char hex label.
    //
    // Returns the semicolon-joined filter_complex string, or empty if no filters.
    static QString buildFilterFlags(
        const FilterGraph& graph,
        const QList<int>& mutedPositions,
        const QMap<int, int>& nodeIdToChainPosition,
        std::function<QString(int)> hexLabelFunc
    );
};

} // namespace DAG
