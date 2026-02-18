#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;

/**
 * FFAsuperpass - Wraps FFmpeg's 'asuperpass' audio filter
 * High-order Butterworth band-pass filter
 * FFmpeg filter: asuperpass=centerf=<f>:order=<o>:qfactor=<q>:level=<l>
 */
class FFAsuperpass : public BaseFilter {
    Q_OBJECT
public:
    FFAsuperpass();
    ~FFAsuperpass() override = default;

    QString displayName() const override { return "Super Bandpass"; }
    QString filterType() const override { return "ff-asuperpass"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_centerf = 1000.0;  // 2 - 999999 Hz, center frequency, default 1000
    int m_order = 4;            // 4 - 20, filter order, default 4
    double m_qfactor = 1.0;     // 0.01 - 100, Q factor, default 1
    double m_level = 1.0;       // 0 - 2, input level, default 1

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* centerfSpinBox = nullptr;
    QSlider* centerfSlider = nullptr;
    QSpinBox* orderSpinBox = nullptr;
    QSlider* orderSlider = nullptr;
    QDoubleSpinBox* qfactorSpinBox = nullptr;
    QSlider* qfactorSlider = nullptr;
    QDoubleSpinBox* levelSpinBox = nullptr;
    QSlider* levelSlider = nullptr;
};
