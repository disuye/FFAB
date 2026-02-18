#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QLabel;

/**
 * FFAdenorm - Wraps FFmpeg's 'adenorm' audio filter
 * Remedy denormals by adding extremely low-level noise.
 * FFmpeg filter: adenorm=level=<l>:type=<t>
 * 
 * Denormal numbers can cause CPU performance issues in DSP.
 * This filter adds imperceptible noise to prevent denormals.
 */
class FFAdenorm : public BaseFilter {
    Q_OBJECT
public:
    FFAdenorm();
    ~FFAdenorm() override = default;

    QString displayName() const override { return "Anti-Denorm"; }
    QString filterType() const override { return "ff-adenorm"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_level = -351.0;  // -451 to -90 dB, default -351
    int m_type = 0;           // 0=dc, 1=ac, 2=square, 3=pulse, default dc

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelSlider = nullptr;
    QDoubleSpinBox* levelSpinBox = nullptr;
    QComboBox* typeCombo = nullptr;
};
