#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;

/**
 * FFApulsator - Wraps FFmpeg's 'apulsator' audio filter
 * Audio pulsator - rhythmic amplitude modulation
 * FFmpeg filter: apulsator=level_in=<l>:level_out=<l>:mode=<m>:amount=<a>:offset_l=<o>:offset_r=<o>:width=<w>:timing=<t>:bpm=<b>:ms=<m>:hz=<h>
 */
class FFApulsator : public BaseFilter {
    Q_OBJECT
public:
    FFApulsator();
    ~FFApulsator() override = default;

    QString displayName() const override { return "Pulsator"; }
    QString filterType() const override { return "ff-apulsator"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;    // 0.015625 - 64, default 1
    double m_levelOut = 1.0;   // 0.015625 - 64, default 1
    int m_mode = 0;            // 0=sine, 1=triangle, 2=square, 3=sawup, 4=sawdown, default sine
    double m_amount = 1.0;     // 0 - 1, default 1
    double m_offsetL = 0.0;    // 0 - 1, default 0
    double m_offsetR = 0.5;    // 0 - 1, default 0.5
    double m_width = 1.0;      // 0 - 2, default 1
    int m_timing = 0;          // 0=hz, 1=bpm, 2=ms, default hz
    double m_bpm = 120.0;      // 30 - 300, default 120
    int m_ms = 500;            // 10 - 2000, default 500
    double m_hz = 2.0;         // 0.01 - 100, default 2

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QComboBox* modeCombo = nullptr;
    QSlider* amountSlider = nullptr;
    QDoubleSpinBox* amountSpinBox = nullptr;
    QSlider* offsetLSlider = nullptr;
    QDoubleSpinBox* offsetLSpinBox = nullptr;
    QSlider* offsetRSlider = nullptr;
    QDoubleSpinBox* offsetRSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QComboBox* timingCombo = nullptr;
    QSlider* bpmSlider = nullptr;
    QDoubleSpinBox* bpmSpinBox = nullptr;
    QSlider* msSlider = nullptr;
    QSpinBox* msSpinBox = nullptr;
    QSlider* hzSlider = nullptr;
    QDoubleSpinBox* hzSpinBox = nullptr;
};
