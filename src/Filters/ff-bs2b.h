#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QSpinBox;
class QComboBox;

/**
 * FFBs2b - Wraps FFmpeg's 'bs2b' audio filter
 * Bauer stereophonic-to-binaural DSP for improved headphone listening
 * Requires libbs2b to be compiled into FFmpeg
 * FFmpeg filter: bs2b=profile=<p>:fcut=<f>:feed=<f>
 */
class FFBs2b : public BaseFilter {
    Q_OBJECT
public:
    FFBs2b();
    ~FFBs2b() override = default;

    QString displayName() const override { return "BS2B Crossfeed"; }
    QString filterType() const override { return "ff-bs2b"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    int m_profile = 0;       // 0=default, 1=cmoy, 2=jmeier
    int m_fcut = 700;        // 300 - 2000 Hz, crossfeed cutoff frequency
    int m_feed = 45;         // 10 - 150 (x10 = 1.0 - 15.0 dB), crossfeed level

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QComboBox* profileCombo = nullptr;
    QSpinBox* fcutSpinBox = nullptr;
    QSlider* fcutSlider = nullptr;
    QSpinBox* feedSpinBox = nullptr;
    QSlider* feedSlider = nullptr;
};
