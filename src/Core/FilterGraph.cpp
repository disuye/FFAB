#include "FilterGraph.h"

namespace DAG {

void FilterGraph::addNode(FilterNode node) {
    m_nodes.push_back(std::move(node));
}

void FilterGraph::addConnection(Connection conn) {
    m_connections.push_back(std::move(conn));
}

bool FilterGraph::buildLinearChain(
    const std::vector<std::shared_ptr<BaseFilter>>& orderedFilters)
{
    clear();
    if (orderedFilters.empty()) return false;

    for (const auto& filter : orderedFilters) {
        addNode(FilterNode::fromFilter(filter));
    }

    for (size_t i = 0; i + 1 < m_nodes.size(); ++i) {
        Connection conn;
        conn.sourceNodeId   = m_nodes[i].nodeId;
        conn.sourcePortName = "main_out";
        conn.destNodeId     = m_nodes[i + 1].nodeId;
        conn.destPortName   = "main_in";
        addConnection(conn);
    }

    return true;
}

std::vector<int> FilterGraph::topologicalOrder() const {
    std::vector<int> order;
    order.reserve(m_nodes.size());
    for (const auto& node : m_nodes) {
        order.push_back(node.nodeId);
    }
    return order;
}

const FilterNode* FilterGraph::findNode(int nodeId) const {
    for (const auto& node : m_nodes) {
        if (node.nodeId == nodeId) return &node;
    }
    return nullptr;
}

QString FilterGraph::validate() const {
    if (m_nodes.empty()) return "Graph is empty";
    return {};
}

void FilterGraph::clear() {
    m_nodes.clear();
    m_connections.clear();
}

} // namespace DAG
