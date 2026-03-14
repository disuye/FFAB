#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QObject>

class BaseFilter;

/**
 * FilterPresetManager — per-filter preset save/load system
 *
 * Each filter type gets its own preset folder under:
 *   ~/Documents/FFAB/Filter Presets/<filterType>/
 *
 * Preset files are .ffabfilter (JSON, single filter state).
 * Intentionally separate from PresetManager which handles full app state.
 *
 * JSON format (example for ff-dynaudnorm):
 * {
 *     "ffab_version": "1.2.0",
 *     "filter_type": "ff-dynaudnorm",
 *     "preset_name": "Gentle Broadcast",
 *     "params": { ...filter's toJSON output... }
 * }
 */
class FilterPresetManager : public QObject {
    Q_OBJECT

public:
    explicit FilterPresetManager(QObject* parent = nullptr);

    // ========== Folder Management ==========

    // Get the base directory for all filter presets
    static QString basePresetsDirectory();

    // Get the preset folder for a specific filter type (creates if needed)
    static QString presetDirectoryForType(const QString& filterType);

    // ========== Preset Listing ==========

    // Get sorted list of preset names for a filter type (without extension)
    static QStringList presetsForType(const QString& filterType);

    // ========== Save ==========

    // Save current filter state as a named preset.
    // Returns true on success.
    bool savePreset(BaseFilter* filter, const QString& presetName);

    // ========== Load ==========

    // Load a named preset into a filter, updating its parameters in-place.
    // The filter's member variables are updated via fromJSON().
    // Returns true on success.
    // IMPORTANT: caller must rebuild the parameters widget after this call
    // (fromJSON updates member vars but not the live UI widgets).
    bool loadPreset(BaseFilter* filter, const QString& presetName);

    // ========== Delete ==========

    // Delete a named preset for a filter type.
    bool deletePreset(const QString& filterType, const QString& presetName);

    // ========== Utilities ==========

    // Full file path for a preset
    static QString presetFilePath(const QString& filterType, const QString& presetName);

    // File extension for filter presets
    static constexpr const char* EXTENSION = ".ffabfilter";
};
