#include "FFmpegPresets.h"

QMap<QString, QString> FFmpegPresets::presets;

QString FFmpegPresets::getPreset(const QString& name) {
    return presets.value(name, "");
}
