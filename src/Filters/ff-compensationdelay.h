#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QLabel;

/**
 * FFCompensationdelay - Wraps FFmpeg's 'compensationdelay' audio filter
 * Audio Compensation Delay Line - sample-accurate delay for time alignment
 * FFmpeg filter: compensationdelay=mm=<mm>:cm=<cm>:m=<m>:dry=<d>:wet=<w>:temp=<t>
 */
class FFCompensationdelay : public BaseFilter {
    Q_OBJECT
public:
    FFCompensationdelay();
    ~FFCompensationdelay() override = default;

    QString displayName() const override { return "Compensation Delay"; }
    QString filterType() const override { return "ff-compensationdelay"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    void updateTotalDistance();

    // Parameters - distance-based delay
    int m_mm = 0;        // 0 - 10 millimeters, default 0
    int m_cm = 0;        // 0 - 100 centimeters, default 0
    int m_m = 0;         // 0 - 100 meters, default 0
    double m_dry = 0.0;  // 0 - 1, dry signal, default 0
    double m_wet = 1.0;  // 0 - 1, wet signal, default 1
    int m_temp = 20;     // -50 to 50 °C, temperature, default 20

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* mmSpinBox = nullptr;
    QSpinBox* cmSpinBox = nullptr;
    QSpinBox* mSpinBox = nullptr;
    QDoubleSpinBox* drySpinBox = nullptr;
    QSlider* drySlider = nullptr;
    QDoubleSpinBox* wetSpinBox = nullptr;
    QSlider* wetSlider = nullptr;
    QSpinBox* tempSpinBox = nullptr;
    QSlider* tempSlider = nullptr;
    QLabel* totalDistanceLabel = nullptr;
    QLabel* delayMsLabel = nullptr;
};