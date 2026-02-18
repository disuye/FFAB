#include "FFmpegCommandBuilder.h"

QString FFmpegCommandBuilder::buildCommand(const QString& input,
                                           const QString& filterFlags,
                                           const QString& outputFlags,
                                           const QString& output) {
    QString cmd = "ffmpeg ";
    cmd += input + " ";
    if (!filterFlags.isEmpty()) cmd += filterFlags + " ";
    cmd += outputFlags + " ";
    cmd += "\"" + output + "\"";
    return cmd;
}
