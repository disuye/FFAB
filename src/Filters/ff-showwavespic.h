#pragma once

#include "BaseFilter.h"
#include <QColor>
#include <QString>

class QSpinBox;
class QPushButton;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QGroupBox;

/**
 * FFShowwavespic - Waveform Image Generator
 * 
 * Creates a PNG image of the audio waveform. Like AuxOutputFilter, this filter
 * branches from the main audio chain (via asplit) and outputs a video file
 * alongside the processed audio.
 * 
 * Output: PNG image file saved to the same output folder with filename:
 *         <input_basename>_<hexID>-waveform.png
 */
class FFShowwavespic : public BaseFilter {
    Q_OBJECT
public:
    FFShowwavespic();
    ~FFShowwavespic() override = default;
    
    void setFilterId(int id) override;

    QString displayName() const override { return "Waveform Image"; }
    QString filterType() const override { return "ff-showwavespic"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    enum class ScaleMode { LINEAR = 0, SQRT, LOG };
    enum class DrawMode { FULL = 0, SCALE };
    enum class FilterMode { AVERAGE = 0, PEAK };

    // Output destination (like AuxOutputFilter)
    QString getOutputFolder() const { return m_outputFolder; }
    void setOutputFolder(const QString& folder) { m_outputFolder = folder; }
    
    bool getUseCustomOutputFolder() const { return m_useCustomOutputFolder; }
    void setUseCustomOutputFolder(bool use) { m_useCustomOutputFolder = use; }
    
    // Video output flags for -map section (not for filter_complex)
    QString getVideoCodecFlags() const { return m_videoCodecFlags; }
    
    // File extension for output
    QString getFileExtension() const { return "png"; }
    
    // Get the filename suffix (includes hex ID)
    QString getFilenameSuffix() const;

private slots:
    void onBrowseOutputFolder();

private:
    void updateFFmpegFlags();
    void updateVideoCodecFlags();
    QString scaleModeToString(ScaleMode mode) const;
    QString drawModeToString(DrawMode mode) const;
    QString filterModeToString(FilterMode mode) const;

    // Waveform settings
    int m_width = 1024;
    int m_height = 768;
    QColor m_color1 = QColor("#838172");
    QColor m_color2 = QColor("#AD9E67");
    ScaleMode m_scaleMode = ScaleMode::SQRT;
    DrawMode m_drawMode = DrawMode::FULL;
    FilterMode m_filterMode = FilterMode::AVERAGE;
    bool m_splitChannels = true;
    
    // Output settings
    QString m_outputFolder;
    bool m_useCustomOutputFolder = false;
    
    // Generated flags
    QString ffmpegFlags;        // Filter portion: showwavespic=...
    QString m_videoCodecFlags;  // Output portion: -frames:v 1 -c:v png

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* widthSpinBox = nullptr;
    QSpinBox* heightSpinBox = nullptr;
    QPushButton* color1Button = nullptr;
    QPushButton* color2Button = nullptr;
    QComboBox* scaleModeCombo = nullptr;
    QComboBox* drawModeCombo = nullptr;
    QComboBox* filterModeCombo = nullptr;
    QCheckBox* splitChannelsCheckBox = nullptr;
    QGroupBox* destGroup = nullptr;
    QCheckBox* useCustomOutputFolderCheck = nullptr;
    QPushButton* browseOutputFolderButton = nullptr;
    QLineEdit* outputFolderEdit = nullptr;
};
