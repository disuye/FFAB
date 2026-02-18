#pragma once

#include "Port.h"
#include "BaseFilter.h"
#include <memory>
#include <vector>

namespace DAG {

struct FilterNode {
    int                              nodeId;
    std::shared_ptr<BaseFilter>      filter;
    std::vector<PortDescriptor>      inputs;
    std::vector<PortDescriptor>      outputs;

    static FilterNode fromFilter(std::shared_ptr<BaseFilter> f) {
        FilterNode node;
        node.nodeId  = f->getFilterId();
        node.filter  = f;
        node.inputs  = { mainInput() };
        node.outputs = { mainOutput() };
        return node;
    }
};

} // namespace DAG
