#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFTremolo - Wraps FFmpeg's 'tremolo' audio filter
 * Apply tremolo effect
 * FFmpeg filter: tremolo=f=<frequency>:d=<depth>
 */
class FFTremolo : public BaseFilter {
    Q_OBJECT
public:
    FFTremolo();
    ~FFTremolo() override = default;

    QString displayName() const override { return "Tremolo"; }
    QString filterType() const override { return "ff-tremolo"; }
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
