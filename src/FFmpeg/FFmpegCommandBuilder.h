#pragma once
#include <QString>

class FFmpegCommandBuilder {
public:
    static QString buildCommand(const QString& input, const QString& filterFlags,
                               const QString& outputFlags, const QString& output);
};
