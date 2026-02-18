#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;

/**
 * FFAsupercut - Wraps FFmpeg's 'asupercut' audio filter
 * Cut supersonic/ultrasonic frequencies with steep Butterworth filter
 * FFmpeg filter: asupercut=cutoff=<f>:order=<o>:level=<l>
 */
class FFAsupercut : public BaseFilter {
    Q_OBJECT
public:
    FFAsupercut();
    ~FFAsupercut() override = default;

    QString displayName() const override { return "Super Cut"; }
    QString filterType() const override { return "ff-asupercut"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_cutoff = 20000.0;  // 20000 - 192000 Hz, default 20000
    int m_order = 10;           // 3 - 20, filter order, default 10
    double m_level = 1.0;       // 0 - 1, input level, default 1

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* cutoffSpinBox = nullptr;
    QSlider* cutoffSlider = nullptr;
    QSpinBox* orderSpinBox = nullptr;
    QSlider* orderSlider = nullptr;
    QDoubleSpinBox* levelSpinBox = nullptr;
    QSlider* levelSlider = nullptr;
};
