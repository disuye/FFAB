#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLabel;

/**
 * FFAdynamicsmooth - Wraps FFmpeg's 'adynamicsmooth' audio filter
 * Apply Dynamic Smoothing of input audio.
 * FFmpeg filter: adynamicsmooth=sensitivity=<s>:basefreq=<f>
 * 
 * Applies an adaptive low-pass filter that smooths the audio
 * while preserving transients. The cutoff frequency adapts
 * based on the signal's rate of change.
 */
class FFAdynamicsmooth : public BaseFilter {
    Q_OBJECT
public:
    FFAdynamicsmooth();
    ~FFAdynamicsmooth() override = default;

    QString displayName() const override { return "Dynamic Smooth"; }
    QString filterType() const override { return "ff-adynamicsmooth"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_sensitivity = 2.0;   // 0 - 1000000, default 2
    double m_basefreq = 22050.0;  // 2 - 1000000 Hz, default 22050

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* sensitivitySlider = nullptr;
    QDoubleSpinBox* sensitivitySpinBox = nullptr;
    QSlider* basefreqSlider = nullptr;
    QDoubleSpinBox* basefreqSpinBox = nullptr;
};
