#pragma once

#include <QString>

namespace DAG {

struct Connection {
    int     sourceNodeId;
    QString sourcePortName;   // e.g. "main_out"
    int     destNodeId;
    QString destPortName;     // e.g. "main_in"
};

} // namespace DAG
