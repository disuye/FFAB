#pragma once
#include <QString>
#include <QMap>

class FFmpegPresets {
public:
    static QString getPreset(const QString& name);
private:
    static QMap<QString, QString> presets;
};
