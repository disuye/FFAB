#include "PresetStorage.h"
#include <QStandardPaths>
#include <QDir>

QString PresetStorage::getPresetFolder() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/presets";
}

void PresetStorage::ensurePresetFolderExists() {
    QDir().mkpath(getPresetFolder());
}
