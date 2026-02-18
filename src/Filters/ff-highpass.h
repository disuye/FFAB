#pragma once

#include "BaseFilter.h"
#include "FrequencySliderHelpers.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFHighpass - Wraps FFmpeg's 'highpass' audio filter
 * Apply a high-pass filter with 3dB point frequency
 * FFmpeg filter: highpass=frequency=<f>:width_type=<t>:width=<w>:poles=<p>:mix=<m>:...
 */
class FFHighpass : public BaseFilter {
    Q_OBJECT
public:
    FFHighpass();
    ~FFHighpass() override = default;

    QString displayName() const override { return "High Pass"; }
    QString filterType() const override { return "ff-highpass"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_frequency = 3000.0;    // 0 - 999999 Hz, default 3000
    int m_widthType = 3;            // 1=h, 2=q, 3=o, 4=s, 5=k (default q)
    double m_width = 0.707;         // 0 - 99999, default 0.707
    int m_poles = 2;                // 1 - 2, default 2
    double m_mix = 1.0;             // 0 - 1, default 1
    bool m_normalize = false;       // default false
    int m_transform = 0;            // 0-6 (di, dii, tdi, tdii, latt, svf, zdf), default di
    int m_precision = -1;           // -1=auto, 0=s16, 1=s32, 2=f32, 3=f64

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Tab 1 - Filter
    QDoubleSpinBox* frequencySpinBox = nullptr;
    QSlider* frequencySlider = nullptr;
    QComboBox* widthTypeCombo = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    QComboBox* polesCombo = nullptr;
    
    // Tab 2 - Options
    QDoubleSpinBox* mixSpinBox = nullptr;
    QSlider* mixSlider = nullptr;
    QCheckBox* normalizeCheck = nullptr;
    QComboBox* transformCombo = nullptr;
    QComboBox* precisionCombo = nullptr;
};
