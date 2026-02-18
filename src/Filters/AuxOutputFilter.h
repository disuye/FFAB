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

class AuxOutputFilter : public BaseFilter {
    Q_OBJECT
public:
    AuxOutputFilter();
    ~AuxOutputFilter() override = default;
    
    void setFilterId(int id) override;

    QString displayName() const override { return "Aux Output"; }
    QString filterType() const override { return "aux-output"; }
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
    
    bool getUseCustomOutputFolder() const { return m_useCustomOutputFolder; }
    void setUseCustomOutputFolder(bool use) { m_useCustomOutputFolder = use; }
    
    // For internal stream branching when multiple aux outputs on main chain
    int getAuxOutputIndex() const { return m_auxOutputIndex; }
    void setAuxOutputIndex(int index) { m_auxOutputIndex = index; }
    
    // Get codec flags for output mapping (not for filter_complex)
    QString getCodecFlags() const { return ffmpegFlags; }
    
    // Get the file extension for this aux output (wav, flac, mp3, etc.)
    QString getFileExtension() const;

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
    QString m_filenameSuffix;  // Will be initialized in setFilterId with hex ID
    bool m_useCustomOutputFolder = false; // If false, use main OUTPUT folder
    
    // For managing multiple aux outputs
    int m_auxOutputIndex = 0; // Set by FilterChain during command generation
    
    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QComboBox* categoryCombo = nullptr;
    QStackedWidget* settingsStack = nullptr;
    QWidget* uncompressedPage = nullptr;
    QWidget* losslessPage = nullptr;
    QWidget* compressedPage = nullptr;
    QLineEdit* customFlagsEdit = nullptr;
    QCheckBox* useCustomOutputFolderCheck = nullptr;
    QPushButton* browseOutputFolderButton = nullptr;
    QLineEdit* outputFolderEdit = nullptr;
    QLineEdit* filenameSuffixEdit = nullptr;
    QLabel* filenamePreviewLabel = nullptr;
};
