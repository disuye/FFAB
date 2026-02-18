#include "DAGCommandBuilder.h"
#include "CustomFFmpegFilter.h"
#include <QRegularExpression>
#include <QStringList>

namespace DAG {

QString DAGCommandBuilder::buildFilterFlags(
    const FilterGraph& graph,
    const QList<int>& mutedPositions,
    const QMap<int, int>& nodeIdToChainPosition,
    std::function<QString(int)> hexLabelFunc)
{
    const auto& nodes = graph.nodes();
    if (nodes.size() <= 2) return "";  // Only INPUT + OUTPUT

    auto topoOrder = graph.topologicalOrder();
    int firstNodeId = topoOrder.front();
    int lastNodeId  = topoOrder.back();

    // Check if there are non-muted filters with output after a given index
    auto hasFiltersAfter = [&](size_t currentIdx) -> bool {
        for (size_t j = currentIdx + 1; j < topoOrder.size(); ++j) {
            int nid = topoOrder[j];
            if (nid == lastNodeId) continue;  // Skip OUTPUT node

            int chainPos = nodeIdToChainPosition.value(nid, -1);
            if (mutedPositions.contains(chainPos)) continue;

            const FilterNode* node = graph.findNode(nid);
            if (!node) continue;
            if (!node->filter->buildFFmpegFlags().isEmpty()) return true;
        }
        return false;
    };

    QStringList filterStrs;
    QString mainChainInput = "[0:a]";

    for (size_t i = 0; i < topoOrder.size(); ++i) {
        int nodeId = topoOrder[i];

        // Skip INPUT and OUTPUT nodes
        if (nodeId == firstNodeId || nodeId == lastNodeId) continue;

        int chainPos = nodeIdToChainPosition.value(nodeId, -1);
        if (mutedPositions.contains(chainPos)) continue;

        const FilterNode* node = graph.findNode(nodeId);
        if (!node) continue;

        QString filterStr = node->filter->buildFFmpegFlags();
        if (filterStr.isEmpty()) continue;

        bool isLastFilter = !hasFiltersAfter(i);
        int filterId = node->filter->getFilterId();

        // Handle input labels — replicate FilterChain.cpp lines 1122-1128
        bool hasInputLabels = filterStr.contains("[0:a]") ||
                              filterStr.contains(QRegularExpression("^\\["));

        if (hasInputLabels) {
            filterStr.replace("[0:a]", mainChainInput);
        } else {
            filterStr = mainChainInput + filterStr;
        }

        // Handle output labels — replicate FilterChain.cpp lines 1130-1155
        bool usesCustomOutput = node->filter->usesCustomOutputStream();
        QString outputLabel;
        if (usesCustomOutput) {
            outputLabel = QString("[%1]").arg(hexLabelFunc(filterId));
        } else {
            outputLabel = isLastFilter ? "[out]" :
                          QString("[%1]").arg(hexLabelFunc(filterId));
        }

        bool manualLabels = false;
        if (auto* customFilter =
                dynamic_cast<CustomFFmpegFilter*>(node->filter.get())) {
            manualLabels = customFilter->getManualOutputLabels();
        }

        if (!manualLabels) {
            // Strip any trailing output labels the filter may have generated
            int lastParam = qMax(filterStr.lastIndexOf('='),
                                 filterStr.lastIndexOf(':'));
            if (lastParam != -1) {
                int outputLabelStart = filterStr.indexOf('[', lastParam);
                if (outputLabelStart != -1) {
                    filterStr = filterStr.left(outputLabelStart);
                }
            }
            filterStr += outputLabel;
            if (!usesCustomOutput && !isLastFilter) {
                mainChainInput = outputLabel;
            }
        }

        filterStrs.append(filterStr);
    }

    return filterStrs.join(";");
}

} // namespace DAG
