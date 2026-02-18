#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFCrossfeed - Wraps FFmpeg's 'crossfeed' audio filter
 * Apply headphone crossfeed filter for more natural stereo imaging
 * FFmpeg filter: crossfeed=strength=<s>:range=<r>:slope=<sl>:level_in=<li>:level_out=<lo>
 */
class FFCrossfeed : public BaseFilter {
    Q_OBJECT
public:
    FFCrossfeed();
    ~FFCrossfeed() override = default;

    QString displayName() const override { return "Crossfeed"; }
    QString filterType() const override { return "ff-crossfeed"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_strength = 0.2;        // 0 - 1, default 0.2 (crossfeed strength)
    double m_range = 0.5;           // 0 - 1, default 0.5 (soundstage wideness)
    double m_slope = 0.5;           // 0.01 - 1, default 0.5 (curve slope)
    double m_levelIn = 0.9;         // 0 - 1, default 0.9 (input level)
    double m_levelOut = 1.0;        // 0 - 1, default 1.0 (output level)

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    QDoubleSpinBox* strengthSpinBox = nullptr;
    QSlider* strengthSlider = nullptr;
    QDoubleSpinBox* rangeSpinBox = nullptr;
    QSlider* rangeSlider = nullptr;
    QDoubleSpinBox* slopeSpinBox = nullptr;
    QSlider* slopeSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
};
