#pragma once

#include <QString>
#include <vector>

namespace DAG {

struct PortDescriptor {
    enum class Direction { Input, Output };
    enum class Kind {
        MainAudio,      // Primary audio stream (Phase A)
        Sidechain,      // Sidechain input (Phase B)
        BranchOutput,   // Aux/waveform tap (Phase B/C)
    };

    QString   name;
    Direction direction;
    Kind      kind = Kind::MainAudio;

    bool operator==(const PortDescriptor& o) const {
        return name == o.name && direction == o.direction && kind == o.kind;
    }
};

inline PortDescriptor mainInput(const QString& name = "main_in") {
    return { name, PortDescriptor::Direction::Input, PortDescriptor::Kind::MainAudio };
}

inline PortDescriptor mainOutput(const QString& name = "main_out") {
    return { name, PortDescriptor::Direction::Output, PortDescriptor::Kind::MainAudio };
}

} // namespace DAG
