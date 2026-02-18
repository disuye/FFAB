#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;

/**
 * FFBiquad - Wraps FFmpeg's 'biquad' audio filter
 * Generic biquad IIR filter with direct coefficient input
 * FFmpeg filter: biquad=a0=<a0>:a1=<a1>:a2=<a2>:b0=<b0>:b1=<b1>:b2=<b2>:mix=<m>:normalize=<n>
 */
class FFBiquad : public BaseFilter {
    Q_OBJECT
public:
    FFBiquad();
    ~FFBiquad() override = default;

    QString displayName() const override { return "Biquad"; }
    QString filterType() const override { return "ff-biquad"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters - transfer function: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (a0 + a1*z^-1 + a2*z^-2)
    double m_a0 = 1.0;       // default 1
    double m_a1 = 0.0;       // default 0
    double m_a2 = 0.0;       // default 0
    double m_b0 = 1.0;       // default 0 (changed to 1 for passthrough)
    double m_b1 = 0.0;       // default 0
    double m_b2 = 0.0;       // default 0
    double m_mix = 1.0;      // 0 - 1, default 1
    bool m_normalize = false; // default false
    int m_transform = 0;      // 0=di, 1=dii, 2=tdi, 3=tdii, 4=latt, 5=svf, 6=zdf

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* a0SpinBox = nullptr;
    QDoubleSpinBox* a1SpinBox = nullptr;
    QDoubleSpinBox* a2SpinBox = nullptr;
    QDoubleSpinBox* b0SpinBox = nullptr;
    QDoubleSpinBox* b1SpinBox = nullptr;
    QDoubleSpinBox* b2SpinBox = nullptr;
    QDoubleSpinBox* mixSpinBox = nullptr;
    QSlider* mixSlider = nullptr;
    QCheckBox* normalizeCheck = nullptr;
    QComboBox* transformCombo = nullptr;
};
