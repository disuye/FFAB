#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLabel;

/**
 * FFDrmeter - Wraps FFmpeg's 'drmeter' audio filter
 * Measure audio dynamic range.
 * FFmpeg filter: drmeter=length=<l>
 * 
 * This is an analysis filter that measures and outputs dynamic range statistics.
 * Audio passes through unchanged while being measured.
 * 
 * Output includes DR (Dynamic Range) value per channel.
 */
class FFDrmeter : public BaseFilter {
    Q_OBJECT
public:
    FFDrmeter();
    ~FFDrmeter() override = default;

    QString displayName() const override { return "DR Meter"; }
    QString filterType() const override { return "ff-drmeter"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_length = 3.0;  // 0.01 - 10 seconds, default 3

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* lengthSlider = nullptr;
    QDoubleSpinBox* lengthSpinBox = nullptr;
};
