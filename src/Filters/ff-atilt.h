#pragma once

#include "BaseFilter.h"
#include "FrequencySliderHelpers.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;

/**
 * FFAtilt - Wraps FFmpeg's 'atilt' audio filter
 * Apply spectral tilt (bass/treble balance in single control)
 * FFmpeg filter: atilt=freq=<f>:slope=<s>:width=<w>:order=<o>:level=<l>
 */
class FFAtilt : public BaseFilter {
    Q_OBJECT
public:
    FFAtilt();
    ~FFAtilt() override = default;

    QString displayName() const override { return "Tilt EQ"; }
    QString filterType() const override { return "ff-atilt"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_freq = 10000.0;   // 20 - 192000 Hz, center frequency, default 10000
    double m_slope = 0.0;      // -1 to 1, tilt slope, default 0
    double m_width = 1000.0;   // 100 - 10000, filter width, default 1000
    int m_order = 5;           // 2 - 30, filter order, default 5
    double m_level = 1.0;      // 0 - 4, input level, default 1

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* freqSpinBox = nullptr;
    QSlider* freqSlider = nullptr;
    QDoubleSpinBox* slopeSpinBox = nullptr;
    QSlider* slopeSlider = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    QSpinBox* orderSpinBox = nullptr;
    QSlider* orderSlider = nullptr;
    QDoubleSpinBox* levelSpinBox = nullptr;
    QSlider* levelSlider = nullptr;
};
