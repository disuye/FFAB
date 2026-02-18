#pragma once

#include "BaseFilter.h"
#include "FrequencySliderHelpers.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFBandpass - Wraps FFmpeg's 'bandpass' audio filter
 * Apply a two-pole Butterworth band-pass filter
 * FFmpeg filter: bandpass=frequency=<f>:width_type=<t>:width=<w>:csg=<c>:mix=<m>:...
 */
class FFBandpass : public BaseFilter {
    Q_OBJECT
public:
    FFBandpass();
    ~FFBandpass() override = default;

    QString displayName() const override { return "Band Pass"; }
    QString filterType() const override { return "ff-bandpass"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_frequency = 3000.0;    // 0 - 999999 Hz, default 3000
    int m_widthType = 3;            // 1=h, 2=q, 3=o, 4=s, 5=k (default q)
    double m_width = 0.5;           // 0 - 99999, default 0.5
    bool m_csg = false;             // constant skirt gain, default false
    double m_mix = 1.0;             // 0 - 1, default 1
    bool m_normalize = false;       // default false
    int m_transform = 0;            // 0-6, default di
    int m_precision = -1;           // -1=auto

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Tab 1 - Filter
    QDoubleSpinBox* frequencySpinBox = nullptr;
    QSlider* frequencySlider = nullptr;
    QComboBox* widthTypeCombo = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    QCheckBox* csgCheck = nullptr;
    
    // Tab 2 - Options
    QDoubleSpinBox* mixSpinBox = nullptr;
    QSlider* mixSlider = nullptr;
    QCheckBox* normalizeCheck = nullptr;
    QComboBox* transformCombo = nullptr;
    QComboBox* precisionCombo = nullptr;
};
