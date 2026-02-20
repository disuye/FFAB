#pragma once

#include "BaseFilter.h"
#include <QString>

class QSpinBox;
class QDoubleSpinBox;
class QPushButton;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QGroupBox;

/**
 * FFShowspectrumpic - Spectrum Image Generator
 *
 * Creates a PNG image of the audio frequency spectrum. Like showwavespic, this
 * filter branches from the main audio chain (via asplit) and outputs an image
 * file alongside the processed audio.
 *
 * Output: PNG image file saved to the same output folder with filename:
 *         <input_basename>_<hexID>-spectrum.png
 */
class FFShowspectrumpic : public BaseFilter {
    Q_OBJECT
public:
    FFShowspectrumpic();
    ~FFShowspectrumpic() override = default;

    void setFilterId(int id) override;

    QString displayName() const override { return "Spectrum Image"; }
    QString filterType() const override { return "ff-showspectrumpic"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    enum class DisplayMode { COMBINED = 0, SEPARATE };
    enum class ColorScheme {
        CHANNEL = 0, INTENSITY, RAINBOW, MORELAND, NEBULAE, FIRE, FIERY,
        FRUIT, COOL, MAGMA, GREEN, VIRIDIS, PLASMA, CIVIDIS, TERRAIN
    };
    enum class ScaleMode { LINEAR = 0, SQRT, CBRT, LOG, FOURTHRT, FIFTHRT };
    enum class FreqScale { LINEAR = 0, LOG };
    enum class Orientation { VERTICAL = 0, HORIZONTAL };
    enum class WindowFunc {
        RECT = 0, HANN, HAMMING, BLACKMAN, BARTLETT, WELCH, FLATTOP,
        BHARRIS, BNUTTALL, SINE, NUTTALL, BHANN, LANCZOS, GAUSS,
        TUKEY, DOLPH, CAUCHY, PARZEN, POISSON, BOHMAN, KAISER
    };

    // Output destination (like showwavespic)
    QString getOutputFolder() const { return m_outputFolder; }
    void setOutputFolder(const QString& folder) { m_outputFolder = folder; }

    bool getUseCustomOutputFolder() const { return m_useCustomOutputFolder; }
    void setUseCustomOutputFolder(bool use) { m_useCustomOutputFolder = use; }

    // Video output flags for -map section
    QString getVideoCodecFlags() const { return m_videoCodecFlags; }

    QString getFileExtension() const { return "png"; }
    QString getFilenameSuffix() const;

private slots:
    void onBrowseOutputFolder();

private:
    void updateFFmpegFlags();
    void updateVideoCodecFlags();
    QString colorSchemeToString(ColorScheme scheme) const;
    QString scaleModeToString(ScaleMode mode) const;
    QString freqScaleToString(FreqScale scale) const;
    QString orientationToString(Orientation orient) const;
    QString windowFuncToString(WindowFunc func) const;

    // Spectrum settings
    int m_width = 2048;
    int m_height = 1024;
    DisplayMode m_displayMode = DisplayMode::COMBINED;
    ColorScheme m_colorScheme = ColorScheme::INTENSITY;
    ScaleMode m_scaleMode = ScaleMode::LOG;
    FreqScale m_freqScale = FreqScale::LINEAR;
    Orientation m_orientation = Orientation::VERTICAL;
    WindowFunc m_windowFunc = WindowFunc::HANN;
    bool m_legend = true;
    float m_gain = 1.0f;
    float m_saturation = 1.0f;
    float m_dynamicRange = 120.0f;
    int m_startFreq = 0;
    int m_stopFreq = 0;

    // Output settings
    QString m_outputFolder;
    bool m_useCustomOutputFolder = false;

    // Generated flags
    QString ffmpegFlags;
    QString m_videoCodecFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* widthSpinBox = nullptr;
    QSpinBox* heightSpinBox = nullptr;
    QComboBox* displayModeCombo = nullptr;
    QComboBox* colorSchemeCombo = nullptr;
    QComboBox* scaleModeCombo = nullptr;
    QComboBox* freqScaleCombo = nullptr;
    QComboBox* orientationCombo = nullptr;
    QComboBox* windowFuncCombo = nullptr;
    QCheckBox* legendCheckBox = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QDoubleSpinBox* saturationSpinBox = nullptr;
    QDoubleSpinBox* dynamicRangeSpinBox = nullptr;
    QSpinBox* startFreqSpinBox = nullptr;
    QSpinBox* stopFreqSpinBox = nullptr;
    QGroupBox* destGroup = nullptr;
    QCheckBox* useCustomOutputFolderCheck = nullptr;
    QPushButton* browseOutputFolderButton = nullptr;
    QLineEdit* outputFolderEdit = nullptr;
};
