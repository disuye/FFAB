#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFAphaser - Wraps FFmpeg's 'aphaser' audio filter
 * Add a phasing effect to the audio
 * FFmpeg filter: aphaser=in_gain=<g>:out_gain=<g>:delay=<d>:decay=<d>:speed=<s>:type=<t>
 */
class FFAphaser : public BaseFilter {
    Q_OBJECT
public:
    FFAphaser();
    ~FFAphaser() override = default;

    QString displayName() const override { return "Phaser"; }
    QString filterType() const override { return "ff-aphaser"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_inGain = 0.4;       // 0 - 1, default 0.4
    double m_outGain = 0.74;     // 0 - 1000000000, default 0.74
    double m_delay = 3.0;        // 0 - 5 ms, default 3
    double m_decay = 0.4;        // 0 - 0.99, default 0.4
    double m_speed = 0.5;        // 0.1 - 2 Hz, default 0.5
    int m_type = 1;              // 0=triangular, 1=sinusoidal, default 1

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* inGainSlider = nullptr;
    QDoubleSpinBox* inGainSpinBox = nullptr;
    QSlider* outGainSlider = nullptr;
    QDoubleSpinBox* outGainSpinBox = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
    QSlider* decaySlider = nullptr;
    QDoubleSpinBox* decaySpinBox = nullptr;
    QSlider* speedSlider = nullptr;
    QDoubleSpinBox* speedSpinBox = nullptr;
    QComboBox* typeCombo = nullptr;
};
