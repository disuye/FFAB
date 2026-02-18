#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFVibrato - Wraps FFmpeg's 'vibrato' audio filter
 * Apply vibrato effect
 * FFmpeg filter: vibrato=f=<frequency>:d=<depth>
 */
class FFVibrato : public BaseFilter {
    Q_OBJECT
public:
    FFVibrato();
    ~FFVibrato() override = default;

    QString displayName() const override { return "Vibrato"; }
    QString filterType() const override { return "ff-vibrato"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_frequency = 5.0;   // 0.1 - 20000 Hz, default 5
    double m_depth = 0.5;       // 0 - 1, default 0.5

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* frequencySlider = nullptr;
    QDoubleSpinBox* frequencySpinBox = nullptr;
    QSlider* depthSlider = nullptr;
    QDoubleSpinBox* depthSpinBox = nullptr;
};
