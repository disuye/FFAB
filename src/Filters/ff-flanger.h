#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFFlanger - Wraps FFmpeg's 'flanger' audio filter
 * Apply a flanging effect to the audio
 * FFmpeg filter: flanger=delay=<d>:depth=<d>:regen=<r>:width=<w>:speed=<s>:shape=<s>:phase=<p>:interp=<i>
 */
class FFFlanger : public BaseFilter {
    Q_OBJECT
public:
    FFFlanger();
    ~FFFlanger() override = default;

    QString displayName() const override { return "Flanger"; }
    QString filterType() const override { return "ff-flanger"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_delay = 0.0;     // 0 - 30 ms, default 0
    double m_depth = 2.0;     // 0 - 10 ms, default 2
    double m_regen = 0.0;     // -95 - 95 %, default 0
    double m_width = 71.0;    // 0 - 100 %, default 71
    double m_speed = 0.5;     // 0.1 - 10 Hz, default 0.5
    int m_shape = 1;          // 0=triangular, 1=sinusoidal, default 1
    double m_phase = 25.0;    // 0 - 100 %, default 25
    int m_interp = 0;         // 0=linear, 1=quadratic, default 0

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
    QSlider* depthSlider = nullptr;
    QDoubleSpinBox* depthSpinBox = nullptr;
    QSlider* regenSlider = nullptr;
    QDoubleSpinBox* regenSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QSlider* speedSlider = nullptr;
    QDoubleSpinBox* speedSpinBox = nullptr;
    QComboBox* shapeCombo = nullptr;
    QSlider* phaseSlider = nullptr;
    QDoubleSpinBox* phaseSpinBox = nullptr;
    QComboBox* interpCombo = nullptr;
};
