#pragma once

#include "FilterNode.h"
#include "Connection.h"
#include <vector>
#include <memory>

namespace DAG {

class FilterGraph {
public:
    FilterGraph() = default;

    // ========== Construction ==========

    void addNode(FilterNode node);
    void addConnection(Connection conn);

    // Build a linear graph from an ordered list of filters.
    // First element is the source (InputFilter), last is the sink (OutputFilter).
    // Returns false if the chain is empty.
    bool buildLinearChain(const std::vector<std::shared_ptr<BaseFilter>>& orderedFilters);

    // ========== Query ==========

    const std::vector<FilterNode>& nodes() const { return m_nodes; }
    const std::vector<Connection>& connections() const { return m_connections; }

    // Topologically sorted node IDs.
    // For a linear chain this is trivially the insertion order.
    std::vector<int> topologicalOrder() const;

    const FilterNode* findNode(int nodeId) const;

    // ========== Validation ==========

    // Returns empty string on success, error description on failure.
    QString validate() const;

    void clear();

private:
    std::vector<FilterNode> m_nodes;
    std::vector<Connection> m_connections;
};

} // namespace DAG
