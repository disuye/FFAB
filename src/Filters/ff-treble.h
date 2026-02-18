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
 * FFTreble - Wraps FFmpeg's 'treble' audio filter
 * Boost or cut upper frequencies using a two-pole shelving filter
 * FFmpeg filter: treble=frequency=<f>:width_type=<t>:width=<w>:gain=<g>:poles=<p>:mix=<m>:...
 */
class FFTreble : public BaseFilter {
    Q_OBJECT
public:
    FFTreble();
    ~FFTreble() override = default;

    QString displayName() const override { return "Treble"; }
    QString filterType() const override { return "ff-treble"; }
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
    double m_gain = 0.0;            // -900 - 900 dB, default 0
    int m_poles = 2;                // 1 - 2, default 2
    double m_mix = 1.0;             // 0 - 1, default 1
    bool m_normalize = false;       // default false
    int m_transform = 0;            // 0-6, default di
    int m_precision = -1;           // -1=auto

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Tab 1 - Treble
    QDoubleSpinBox* frequencySpinBox = nullptr;
    QSlider* frequencySlider = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QSlider* gainSlider = nullptr;
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
