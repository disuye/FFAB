#pragma once
#include <QString>

class PresetStorage {
public:
    static QString getPresetFolder();
    static void ensurePresetFolderExists();
};
