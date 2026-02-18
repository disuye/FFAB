// Complete PresetManager.cpp with proper file initialization
// Files are loaded with default metadata (0 values)
// User can click "Rescan Metadata" button to update if needed

#include "PresetManager.h"
#include "InputPanel.h"
#include "OutputSettingsPanel.h"
#include "FilterChainWidget.h"
#include "FileListWidget.h"
#include "../Core/FilterChain.h"
#include "../Core/AppConfig.h"
#include "../Filters/AudioInputFilter.h"

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QTextStream>

PresetManager::PresetManager(QObject* parent) 
    : QObject(parent) 
{
    lastUsedDirectory = getDefaultPresetsDirectory();
}

QString PresetManager::getDefaultPresetsDirectory() {
    QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString presetsPath = docsPath + "/FFAB/Presets";
    
    QDir dir;
    if (!dir.exists(presetsPath)) {
        dir.mkpath(presetsPath);
    }
    
    return presetsPath;
}

bool PresetManager::savePreset(const QString& filepath,
                               const QString& presetName,
                               FilterChain* filterChain,
                               InputPanel* inputPanel,
                               OutputSettingsPanel* outputPanel,
                               FilterChainWidget* filterChainWidget,
                               bool includeFileLists) {
    if (!filterChain || !inputPanel || !outputPanel || !filterChainWidget) {
        qWarning() << "PresetManager::savePreset - null pointer passed";
        return false;
    }
    
    QJsonObject json = serializeToJson(presetName, filterChain, inputPanel, 
                                       outputPanel, filterChainWidget, includeFileLists);
    
    QJsonDocument doc(json);
    QFile file(filepath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "PresetManager::savePreset - failed to open file:" << filepath;
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "Preset saved successfully:" << filepath;
    return true;
}

bool PresetManager::loadPreset(const QString& filepath,
                               FilterChain* filterChain,
                               InputPanel* inputPanel,
                               OutputSettingsPanel* outputPanel,
                               FilterChainWidget* filterChainWidget,
                               MissingFilesInfo& missingInfo,
                               ProgressCallback progressCallback) {
    if (!filterChain || !inputPanel || !outputPanel || !filterChainWidget) {
        qWarning() << "PresetManager::loadPreset - null pointer passed";
        return false;
    }
    
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "PresetManager::loadPreset - failed to open file:" << filepath;
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "PresetManager::loadPreset - invalid JSON";
        return false;
    }
    
    return deserializeFromJson(doc.object(), filterChain, inputPanel, 
                              outputPanel, filterChainWidget, missingInfo, progressCallback);
}

QJsonObject PresetManager::serializeToJson(const QString& presetName,
                                           FilterChain* filterChain,
                                           InputPanel* inputPanel,
                                           OutputSettingsPanel* outputPanel,
                                           FilterChainWidget* filterChainWidget,
                                           bool includeFileLists) {
    QJsonObject root;
    
    // Metadata
    root["ffab_version"] = VERSION_STR;
    root["preset_name"] = presetName;
    root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Input settings
    root["input_settings"] = serializeInputSettings(inputPanel);
    
    // Output settings
    root["output_settings"] = serializeOutputSettings(outputPanel);
    
    // Filter chain (already has toJSON method)
    root["filter_chain"] = filterChain->toJSON();
    
    // Mute/Solo states
    root["mute_solo_states"] = serializeMuteSoloStates(filterChainWidget);
    
    // File lists (optional)
    if (includeFileLists) {
        root["file_lists"] = serializeFileLists(inputPanel, filterChain);
    }
    
    return root;
}

bool PresetManager::deserializeFromJson(const QJsonObject& json,
                                        FilterChain* filterChain,
                                        InputPanel* inputPanel,
                                        OutputSettingsPanel* outputPanel,
                                        FilterChainWidget* filterChainWidget,
                                        MissingFilesInfo& missingInfo,
                                        ProgressCallback progressCallback) {
    // Version check (warn if newer)
    QString presetVersion = json["ffab_version"].toString();
    if (presetVersion > VERSION_STR) {
        qWarning() << "Preset was created with newer FFAB version:" << presetVersion 
                   << "(current:" << VERSION_STR << ")";
        // Continue anyway - best effort
    }
    
    // Load input settings
    if (json.contains("input_settings")) {
        deserializeInputSettings(json["input_settings"].toObject(), inputPanel);
    }
    
    // Load output settings
    if (json.contains("output_settings")) {
        deserializeOutputSettings(json["output_settings"].toObject(), outputPanel);
    }
    
    // Load filter chain
    if (json.contains("filter_chain")) {
        filterChain->fromJSON(json["filter_chain"].toObject());
    }
    
    // Load mute/solo states
    if (json.contains("mute_solo_states")) {
        deserializeMuteSoloStates(json["mute_solo_states"].toObject(), filterChainWidget);
    }
    
    // Load file lists (if present)
    if (json.contains("file_lists")) {
        deserializeFileLists(json["file_lists"].toObject(), inputPanel, 
                           filterChain, missingInfo, progressCallback);
    }
    
    qDebug() << "Preset loaded successfully";
    return true;
}

QJsonObject PresetManager::serializeInputSettings(InputPanel* inputPanel) {
    QJsonObject obj;
    obj["scan_metadata"] = inputPanel->shouldScanMetadata();
    return obj;
}

QJsonObject PresetManager::serializeOutputSettings(OutputSettingsPanel* outputPanel) {
    QJsonObject obj;
    obj["format"] = outputPanel->getOutputFormat();
    obj["sample_rate"] = outputPanel->getSampleRate();
    obj["bit_depth"] = outputPanel->getBitDepth();
    obj["bitrate"] = outputPanel->getBitrate();
    obj["output_folder"] = outputPanel->getOutputFolder();
    return obj;
}

QJsonObject PresetManager::serializeMuteSoloStates(FilterChainWidget* filterChainWidget) {
    QJsonObject obj;
    
    // Now using filter IDs instead of positions
    QList<int> mutedFilterIds = filterChainWidget->getMutedFilterIds();
    QList<int> soloFilterIds = filterChainWidget->getSoloFilterIds();
    
    QJsonArray mutedArray;
    for (int filterId : mutedFilterIds) {
        mutedArray.append(filterId);
    }
    
    QJsonArray soloArray;
    for (int filterId : soloFilterIds) {
        soloArray.append(filterId);
    }
    
    obj["muted_filter_ids"] = mutedArray;
    obj["solo_filter_ids"] = soloArray;
    
    // Keep old keys for backward compatibility (will be removed in future version)
    obj["muted_positions"] = mutedArray;
    obj["solo_positions"] = soloArray;
    
    return obj;
}

QJsonObject PresetManager::serializeFileLists(InputPanel* inputPanel, FilterChain* filterChain) {
    QJsonObject obj;
    obj["included"] = true;
    
    // Main file list
    QJsonArray mainFiles;
    FileListWidget* mainFileList = inputPanel->getFileListWidget();
    if (mainFileList) {
        QList<FileListWidget::AudioFileInfo> files = mainFileList->getAllFiles();
        for (const auto& file : files) {
            QJsonObject fileObj;
            fileObj["path"] = file.filePath;
            fileObj["enabled"] = file.enabled;
            mainFiles.append(fileObj);
        }
    }
    obj["main_files"] = mainFiles;
    
    // AudioInput file lists (main chain + sub-chains)
    QJsonObject audioInputs;
    for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
        int inputIndex = audioInput->getInputIndex();
        QJsonArray inputFiles;
        
        FileListWidget* inputFileList = audioInput->getFileListWidget();
        if (inputFileList) {
            QList<FileListWidget::AudioFileInfo> files = inputFileList->getAllFiles();
            for (const auto& file : files) {
                QJsonObject fileObj;
                fileObj["path"] = file.filePath;
                fileObj["enabled"] = file.enabled;
                inputFiles.append(fileObj);
            }
        }
        
        audioInputs[QString::number(inputIndex)] = inputFiles;
    }
    obj["audio_inputs"] = audioInputs;
    
    return obj;
}

bool PresetManager::deserializeInputSettings(const QJsonObject& json, InputPanel* inputPanel) {
    if (json.contains("scan_metadata")) {
        inputPanel->setScanMetadata(json["scan_metadata"].toBool());
    }
    
    return true;
}

bool PresetManager::deserializeOutputSettings(const QJsonObject& json, OutputSettingsPanel* outputPanel) {
    if (json.contains("format")) {
        outputPanel->setOutputFormat(json["format"].toString());
    }
    
    if (json.contains("sample_rate")) {
        outputPanel->setSampleRate(json["sample_rate"].toInt());
    }
    
    if (json.contains("bit_depth")) {
        outputPanel->setBitDepth(json["bit_depth"].toInt());
    }
    
    if (json.contains("bitrate")) {
        outputPanel->setBitrate(json["bitrate"].toInt());
    }
    
    if (json.contains("output_folder")) {
        outputPanel->setOutputFolder(json["output_folder"].toString());
    }
    
    return true;
}

bool PresetManager::deserializeMuteSoloStates(const QJsonObject& json, FilterChainWidget* filterChainWidget) {
    // Try loading new format (filter IDs) first
    if (json.contains("muted_filter_ids")) {
        QJsonArray mutedArray = json["muted_filter_ids"].toArray();
        QList<int> mutedFilterIds;
        for (const auto& val : mutedArray) {
            mutedFilterIds.append(val.toInt());
        }
        filterChainWidget->setMutedFilterIds(mutedFilterIds);
    } else if (json.contains("muted_positions")) {
        // Fall back to old format (positions) for backward compatibility
        QJsonArray mutedArray = json["muted_positions"].toArray();
        QList<int> mutedIds;
        for (const auto& val : mutedArray) {
            mutedIds.append(val.toInt());
        }
        // In old presets, these were positions but are now interpreted as IDs
        // This works because old presets didn't save filter IDs, so position == ID
        filterChainWidget->setMutedFilterIds(mutedIds);
    }
    
    if (json.contains("solo_filter_ids")) {
        QJsonArray soloArray = json["solo_filter_ids"].toArray();
        QList<int> soloFilterIds;
        for (const auto& val : soloArray) {
            soloFilterIds.append(val.toInt());
        }
        filterChainWidget->setSoloFilterIds(soloFilterIds);
    } else if (json.contains("solo_positions")) {
        // Fall back to old format (positions) for backward compatibility
        QJsonArray soloArray = json["solo_positions"].toArray();
        QList<int> soloIds;
        for (const auto& val : soloArray) {
            soloIds.append(val.toInt());
        }
        // In old presets, these were positions but are now interpreted as IDs
        filterChainWidget->setSoloFilterIds(soloIds);
    }
    
    return true;
}

bool PresetManager::deserializeFileLists(const QJsonObject& json,
                                         InputPanel* inputPanel,
                                         FilterChain* filterChain,
                                         MissingFilesInfo& missingInfo,
                                         ProgressCallback progressCallback) {
    if (!json["included"].toBool()) {
        return true;  // File lists not included in this preset
    }
    
    missingInfo.totalFiles = 0;
    missingInfo.foundFiles = 0;
    missingInfo.missingFiles.clear();
    
    // Count total files first for progress reporting
    int totalFilesToLoad = 0;
    if (json.contains("main_files")) {
        totalFilesToLoad += json["main_files"].toArray().size();
    }
    if (json.contains("audio_inputs")) {
        QJsonObject audioInputs = json["audio_inputs"].toObject();
        for (const QString& key : audioInputs.keys()) {
            totalFilesToLoad += audioInputs[key].toArray().size();
        }
    }
    
    int filesProcessed = 0;
    
    // Load main file list
    if (json.contains("main_files")) {
        QJsonArray mainFiles = json["main_files"].toArray();
        FileListWidget* mainFileList = inputPanel->getFileListWidget();
        
        if (mainFileList) {
            mainFileList->clearFiles();
            
            for (const auto& fileVal : mainFiles) {
                QJsonObject fileObj = fileVal.toObject();
                QString path = fileObj["path"].toString();
                bool enabled = fileObj["enabled"].toBool();
                
                missingInfo.totalFiles++;
                
                if (QFile::exists(path)) {
                    // File exists - add it with proper default values
                    FileListWidget::AudioFileInfo info;
                    info.filePath = path;
                    info.fileName = QFileInfo(path).fileName();
                    info.enabled = enabled;
                    info.format = QFileInfo(path).suffix().toUpper();
                    info.duration = "00:00:00";
                    info.sampleRate = 0;
                    info.bitsPerSample = 0;
                    info.channels = 0;
                    info.bitrate = 0;
                    mainFileList->addFile(info);
                    missingInfo.foundFiles++;
                } else {
                    // File missing
                    missingInfo.missingFiles.append(path);
                }
                
                filesProcessed++;
                
                // Update progress every 10 files or at the end
                if (progressCallback && (filesProcessed % 10 == 0 || filesProcessed == totalFilesToLoad)) {
                    progressCallback(filesProcessed, totalFilesToLoad, 
                                   QString("Loading files... %1/%2").arg(filesProcessed).arg(totalFilesToLoad));
                    // QApplication::processEvents();  // Keep UI responsive
                }
            }
        }
    }
    
    // Load AudioInput file lists (main chain + sub-chains)
    if (json.contains("audio_inputs")) {
        QJsonObject audioInputs = json["audio_inputs"].toObject();
        
        for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
            int inputIndex = audioInput->getInputIndex();
            QString indexKey = QString::number(inputIndex);
            
            if (audioInputs.contains(indexKey)) {
                QJsonArray inputFiles = audioInputs[indexKey].toArray();
                
                // CRITICAL: Ensure parameter widget is created before accessing FileListWidget
                audioInput->getParametersWidget();
                
                FileListWidget* inputFileList = audioInput->getFileListWidget();
                
                if (inputFileList) {
                    inputFileList->clearFiles();
                    
                    for (const auto& fileVal : inputFiles) {
                        QJsonObject fileObj = fileVal.toObject();
                        QString path = fileObj["path"].toString();
                        bool enabled = fileObj["enabled"].toBool();
                        
                        missingInfo.totalFiles++;
                        
                        if (QFile::exists(path)) {
                            FileListWidget::AudioFileInfo info;
                            info.filePath = path;
                            info.fileName = QFileInfo(path).fileName();
                            info.enabled = enabled;
                            info.format = QFileInfo(path).suffix().toUpper();
                            info.duration = "00:00:00";
                            info.sampleRate = 0;
                            info.bitsPerSample = 0;
                            info.channels = 0;
                            info.bitrate = 0;
                            inputFileList->addFile(info);
                            missingInfo.foundFiles++;
                        } else {
                            missingInfo.missingFiles.append(path);
                        }
                        
                        filesProcessed++;
                        
                        // Update progress every 10 files or at the end
                        if (progressCallback && (filesProcessed % 10 == 0 || filesProcessed == totalFilesToLoad)) {
                            progressCallback(filesProcessed, totalFilesToLoad, 
                                           QString("Loading files... %1/%2").arg(filesProcessed).arg(totalFilesToLoad));
                            // QApplication::processEvents();  // Keep UI responsive
                        }
                    }
                }
            }
        }
    }
    
    return true;
}

void PresetManager::writeMissingFilesReport(const MissingFilesInfo& info) {
    if (info.missingFiles.isEmpty()) {
        return;
    }
    
    QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString reportPath = docsPath + "/FFAB/FFAB_missing_files.txt";
    
    QFile file(reportPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to write missing files report to:" << reportPath;
        return;
    }
    
    QTextStream out(&file);
    out << "FFAB Missing Files Report\n";
    out << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    out << "===========================================\n\n";
    out << "Total files in preset: " << info.totalFiles << "\n";
    out << "Files found: " << info.foundFiles << "\n";
    out << "Files missing: " << info.missingFiles.size() << "\n\n";
    out << "Missing file paths:\n";
    out << "-------------------\n";
    
    for (const QString& path : info.missingFiles) {
        out << path << "\n";
    }
    
    file.close();
    
    qDebug() << "Missing files report written to:" << reportPath;
}