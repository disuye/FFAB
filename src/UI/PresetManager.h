#pragma once

#include <QString>
#include <QJsonObject>
#include <QStringList>
#include <QWidget>
#include <functional>

class FilterChain;
class InputPanel;
class OutputSettingsPanel;
class FilterChainWidget;

/**
 * PresetManager - Save and load complete FFAB application state
 * 
 * Saves/loads:
 * - Input settings (format, sample rate, etc.)
 * - Output settings (format, bitrate, bit depth, etc.)
 * - Complete filter chain with all parameters
 * - Mute/Solo button states (position-based)
 * - File lists (main + all AudioInput file lists) - optional
 * 
 * File format: .ffabjson (JSON format, human-readable)
 * Default location: ~/Documents/FFAB/Presets/
 */
class PresetManager : public QObject {
    Q_OBJECT
    
public:
    PresetManager(QObject* parent = nullptr);
    
    // Progress callback: (current, total, message)
    using ProgressCallback = std::function<void(int, int, const QString&)>;
    
    struct PresetData {
        QString version;
        QString presetName;
        QString timestamp;
        QJsonObject inputSettings;
        QJsonObject outputSettings;
        QJsonObject filterChain;
        QJsonObject muteSoloStates;
        bool includeFileLists = false;
        QJsonObject fileLists;
    };
    
    struct MissingFilesInfo {
        QStringList missingFiles;
        int totalFiles = 0;
        int foundFiles = 0;
    };
    
    // Save preset to file
    bool savePreset(const QString& filepath,
                   const QString& presetName,
                   FilterChain* filterChain,
                   InputPanel* inputPanel,
                   OutputSettingsPanel* outputPanel,
                   FilterChainWidget* filterChainWidget,
                   bool includeFileLists);
    
    // Load preset from file
    bool loadPreset(const QString& filepath,
                   FilterChain* filterChain,
                   InputPanel* inputPanel,
                   OutputSettingsPanel* outputPanel,
                   FilterChainWidget* filterChainWidget,
                   MissingFilesInfo& missingInfo,
                   ProgressCallback progressCallback = nullptr);
    
    // Get default presets directory (creates if needed)
    static QString getDefaultPresetsDirectory();
    
    // Get last used directory for file dialogs
    QString getLastUsedDirectory() const { return lastUsedDirectory; }
    void setLastUsedDirectory(const QString& dir) { lastUsedDirectory = dir; }
    
    // Write missing files report
    static void writeMissingFilesReport(const MissingFilesInfo& info);
    
private:
    QString lastUsedDirectory;
    
    // Serialize complete state to JSON
    QJsonObject serializeToJson(const QString& presetName,
                                FilterChain* filterChain,
                                InputPanel* inputPanel,
                                OutputSettingsPanel* outputPanel,
                                FilterChainWidget* filterChainWidget,
                                bool includeFileLists);
    
    // Deserialize JSON to application state
    bool deserializeFromJson(const QJsonObject& json,
                            FilterChain* filterChain,
                            InputPanel* inputPanel,
                            OutputSettingsPanel* outputPanel,
                            FilterChainWidget* filterChainWidget,
                            MissingFilesInfo& missingInfo,
                            ProgressCallback progressCallback);
    
    // Helper methods
    QJsonObject serializeInputSettings(InputPanel* inputPanel);
    QJsonObject serializeOutputSettings(OutputSettingsPanel* outputPanel);
    QJsonObject serializeMuteSoloStates(FilterChainWidget* filterChainWidget);
    QJsonObject serializeFileLists(InputPanel* inputPanel, FilterChain* filterChain);
    
    bool deserializeInputSettings(const QJsonObject& json, InputPanel* inputPanel);
    bool deserializeOutputSettings(const QJsonObject& json, OutputSettingsPanel* outputPanel);
    bool deserializeMuteSoloStates(const QJsonObject& json, FilterChainWidget* filterChainWidget);
    bool deserializeFileLists(const QJsonObject& json, 
                             InputPanel* inputPanel, 
                             FilterChain* filterChain,
                             MissingFilesInfo& missingInfo,
                             ProgressCallback progressCallback);
};
