#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>
#include <QStringList>

class QLabel;
class QComboBox;
class QPushButton;
class QCheckBox;
class FileListWidget;

/**
 * AudioInputFilter - Injects additional audio inputs into the filter chain
 * 
 * This special filter doesn't produce FFmpeg filter flags itself. Instead,
 * it signals to FFmpegCommandBuilder that an additional -i input is needed.
 * 
 * Primary audio input is always [0:a]
 * First AudioInputFilter creates [1:a]
 * Second AudioInputFilter creates [2:a]
 * And so on...
 * 
 * Each AudioInputFilter has its own file list for loading sidechain/IR files.
 * 
 * Used for:
 * - Sidechain compression/gating (sidechaincompress, sidechaingate)
 * - Impulse response convolution (afir)
 * - Channel merging (amerge)
 * - Any other multi-input FFmpeg audio filters
 */
class AudioInputFilter : public BaseFilter {
    Q_OBJECT
public:
    AudioInputFilter();
    ~AudioInputFilter() override = default;

    QString displayName() const override { 
        return QString("Audio Input"); 
    }
    QString filterType() const override { return "audio-input"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    // Get the input index this filter represents ([1:a], [2:a], etc.)
    int getInputIndex() const { return inputIndex; }
    void setInputIndex(int index);
    
    // Refresh the parameter widget labels after index change
    void refreshLabels();
    
    // Get the label this filter creates (e.g., "[1:a]", "[2:a]")
    QString getInputLabel() const;
    
    // Get the file list widget for this AudioInput
    FileListWidget* getFileListWidget() const { return fileListWidget; }
    
    // Get list of enabled (checked) files - for batch processing
    QStringList getEnabledFilePaths() const;
    
    // Get the highlighted/selected file path - for preview generation
    // Falls back to first enabled file if nothing is selected
    QString getSelectedFilePath() const;
    
    // Check if metadata scanning is enabled
    bool shouldScanMetadata() const;
    
    // Public button access for MainWindow to connect signals
    QPushButton* addFilesBtn = nullptr;
    QPushButton* addFolderBtn = nullptr;
    QPushButton* clearBtn = nullptr;
    QPushButton* rescanBtn = nullptr;

signals:
    void inputIndexChanged();
    void filesChanged();  // Emitted when files are added/removed

private:
    int inputIndex = 1;  // Default to [1:a], auto-calculated by FilterChain
    
    QWidget* parametersWidget = nullptr;
    QLabel* labelDisplay = nullptr;
    QLabel* filesListLabel = nullptr;  // "Files for [X:a]:" label
    QCheckBox* scanMetadataCheckbox = nullptr;
    FileListWidget* fileListWidget = nullptr;
};

