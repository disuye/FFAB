#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QComboBox;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QLabel;
class QPushButton;
class QStackedWidget;

class OutputFilter : public BaseFilter {
    Q_OBJECT
public:
    OutputFilter();
    ~OutputFilter() override = default;

    QString displayName() const override { return "OUTPUT"; }
    QString filterType() const override { return "output"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    enum class Category { Uncompressed = 0, Lossless, Compressed, Advanced };
    enum class UncompressedFormat { WAV = 0, AIFF, CAF };
    enum class BitDepth { Float32 = 0, Int24, Int16, Int8 };
    enum class AudioFormat { Interleaved = 0, Split };
    enum class LosslessFormat { FLAC = 0 };
    enum class CompressedFormat { MP3 = 0, AAC };

    QString getOutputFolder() const { return m_outputFolder; }
    void setOutputFolder(const QString& folder) { m_outputFolder = folder; }
    QString getFilenameSuffix() const { return m_filenameSuffix; }
    void setFilenameSuffix(const QString& suffix) { m_filenameSuffix = suffix; }
    
    // Get the file extension for this output (wav, flac, mp3, etc.)
    QString getFileExtension() const;
    
    // Video passthrough — copy video/subtitle streams through untouched
    bool isVideoPassthrough() const { return m_videoPassthrough; }
    void setVideoPassthrough(bool enabled);
    
    // Build the output mapping flags (video mapping + codec flags)
    // When video passthrough is enabled, prepends -map 0:v -c:v copy -map 0:s? -c:s copy
    QString buildOutputMappingFlags() const;

signals:
    void outputFolderChanged(const QString& path);
    void videoPassthroughChanged(bool enabled);

private slots:
    void onCategoryChanged(int index);
    void onSettingsChanged();
    void onBrowseOutputFolder();

private:
    void updateFFmpegFlags();
    void createUncompressedSettings(QWidget* container);
    void createLosslessSettings(QWidget* container);
    void createCompressedSettings(QWidget* container);
    QString buildUncompressedFlags() const;
    QString buildLosslessFlags() const;
    QString buildCompressedFlags() const;

    // Category selection
    Category m_category = Category::Uncompressed;
    
    // Uncompressed settings
    UncompressedFormat m_uncompressedFormat = UncompressedFormat::WAV;
    BitDepth m_bitDepth = BitDepth::Int24;
    int m_sampleRate = 48000;
    AudioFormat m_audioFormat = AudioFormat::Interleaved;
    
    // Lossless settings
    LosslessFormat m_losslessFormat = LosslessFormat::FLAC;
    int m_flacLevel = 5;
    
    // Compressed settings
    CompressedFormat m_compressedFormat = CompressedFormat::MP3;
    bool m_constantBitrate = true;
    int m_bitrate = 320;
    
    // Custom FFmpeg flags (for Advanced category)
    QString m_customFlags = "-c:a libopus -b:a 320k -vbr on -f opus"; // demo flags
    QString m_customExtension = "opus";  // demo file extension
    
    // Output settings
    QString m_outputFolder;
    QString m_filenameSuffix = "_0000";
    
    // Video passthrough
    bool m_videoPassthrough = false;
    
    QString ffmpegFlags;

    // UI widgets - Main
    QWidget* parametersWidget = nullptr;
    QComboBox* categoryCombo = nullptr;
    QStackedWidget* settingsStack = nullptr;
    QWidget* uncompressedPage = nullptr;
    QWidget* losslessPage = nullptr;
    QWidget* compressedPage = nullptr;
    QLineEdit* customFlagsEdit = nullptr;
    QLineEdit* customExtensionEdit = nullptr;
    QLineEdit* outputFolderEdit = nullptr;
    QLineEdit* filenameSuffixEdit = nullptr;
    QCheckBox* videoPassthroughCheck = nullptr;
    
    // UI widgets - Uncompressed settings
    QComboBox* uncompressedFormatCombo = nullptr;
    QComboBox* bitDepthCombo = nullptr;
    QComboBox* sampleRateCombo = nullptr;
    QComboBox* audioFormatCombo = nullptr;
    
    // UI widgets - Lossless settings
    QComboBox* losslessEncodingCombo = nullptr;
    QComboBox* flacLevelCombo = nullptr;
    
    // UI widgets - Compressed settings
    QComboBox* compressedFormatCombo = nullptr;
    QCheckBox* constantBitrateCheck = nullptr;
    QComboBox* bitrateCombo = nullptr;
};
