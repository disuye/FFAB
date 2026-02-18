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
 * FFEqualizer - Wraps FFmpeg's 'equalizer' audio filter
 * Apply two-pole peaking equalization (EQ) filter
 * FFmpeg filter: equalizer=frequency=<f>:width_type=<t>:width=<w>:gain=<g>:mix=<m>:...
 */
class FFEqualizer : public BaseFilter {
    Q_OBJECT
public:
    FFEqualizer();
    ~FFEqualizer() override = default;

    QString displayName() const override { return "Parametric EQ"; }
    QString filterType() const override { return "ff-equalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_frequency = 1000.0;    // 0 - 999999 Hz, default 0 (but we'll use 1000 as practical default)
    int m_widthType = 3;            // 1=h, 2=q, 3=o, 4=s, 5=k (default q)
    double m_width = 1.0;           // 0 - 99999, default 1
    double m_gain = 0.0;            // -900 - 900 dB, default 0
    double m_mix = 1.0;             // 0 - 1, default 1
    bool m_normalize = false;       // default false
    int m_transform = 0;            // 0-6, default di
    int m_precision = -1;           // -1=auto

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Tab 1 - EQ
    QDoubleSpinBox* frequencySpinBox = nullptr;
    QSlider* frequencySlider = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QSlider* gainSlider = nullptr;
    QComboBox* widthTypeCombo = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    
    // Tab 2 - Options
    QDoubleSpinBox* mixSpinBox = nullptr;
    QSlider* mixSlider = nullptr;
    QCheckBox* normalizeCheck = nullptr;
    QComboBox* transformCombo = nullptr;
    QComboBox* precisionCombo = nullptr;
};
