#include "FilterPresetManager.h"
#include "Filters/BaseFilter.h"
#include "Core/AppConfig.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

FilterPresetManager::FilterPresetManager(QObject* parent)
    : QObject(parent)
{
}

// ========== Folder Management ==========

QString FilterPresetManager::basePresetsDirectory() {
    QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return docsPath + "/FFAB/Filter Presets";
}

QString FilterPresetManager::presetDirectoryForType(const QString& filterType) {
    QString path = basePresetsDirectory() + "/" + filterType;
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }
    return path;
}

// ========== Preset Listing ==========

QStringList FilterPresetManager::presetsForType(const QString& filterType) {
    QString dirPath = presetDirectoryForType(filterType);
    QDir dir(dirPath);

    QStringList filters;
    filters << QString("*") + EXTENSION;

    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);

    // Strip extension to get preset names
    QStringList names;
    for (const QString& file : files) {
        names << QFileInfo(file).completeBaseName();
    }
    return names;
}

// ========== Save ==========

bool FilterPresetManager::savePreset(BaseFilter* filter, const QString& presetName) {
    if (!filter || presetName.isEmpty()) {
        qWarning() << "FilterPresetManager::savePreset - null filter or empty name";
        return false;
    }

    QString filterType = filter->filterType();
    QString filePath = presetFilePath(filterType, presetName);

    // Get filter params via existing toJSON
    QJsonObject params;
    filter->toJSON(params);

    // Wrap in preset envelope
    QJsonObject root;
    root["ffab_version"] = VERSION_STR;
    root["filter_type"] = filterType;
    root["preset_name"] = presetName;
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["params"] = params;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "FilterPresetManager::savePreset - failed to write:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Filter preset saved:" << filePath;
    return true;
}

// ========== Load ==========

bool FilterPresetManager::loadPreset(BaseFilter* filter, const QString& presetName) {
    if (!filter || presetName.isEmpty()) {
        qWarning() << "FilterPresetManager::loadPreset - null filter or empty name";
        return false;
    }

    QString filterType = filter->filterType();
    QString filePath = presetFilePath(filterType, presetName);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "FilterPresetManager::loadPreset - failed to read:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "FilterPresetManager::loadPreset - invalid JSON:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();

    // Verify filter type matches
    QString storedType = root["filter_type"].toString();
    if (storedType != filterType) {
        qWarning() << "FilterPresetManager::loadPreset - type mismatch:"
                    << storedType << "vs" << filterType;
        return false;
    }

    // Apply params via existing fromJSON
    QJsonObject params = root["params"].toObject();
    filter->fromJSON(params);

    qDebug() << "Filter preset loaded:" << presetName << "for" << filterType;
    return true;
}

// ========== Delete ==========

bool FilterPresetManager::deletePreset(const QString& filterType, const QString& presetName) {
    QString filePath = presetFilePath(filterType, presetName);
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "FilterPresetManager::deletePreset - file not found:" << filePath;
        return false;
    }
    bool ok = file.remove();
    if (ok) {
        qDebug() << "Filter preset deleted:" << filePath;
    }
    return ok;
}

// ========== Utilities ==========

QString FilterPresetManager::presetFilePath(const QString& filterType, const QString& presetName) {
    return presetDirectoryForType(filterType) + "/" + presetName + EXTENSION;
}
