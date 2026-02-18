#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;

/**
 * FFAsubcut - Wraps FFmpeg's 'asubcut' audio filter
 * Cut subwoofer/subsonic frequencies with steep Butterworth filter
 * FFmpeg filter: asubcut=cutoff=<f>:order=<o>:level=<l>
 */
class FFAsubcut : public BaseFilter {
    Q_OBJECT
public:
    FFAsubcut();
    ~FFAsubcut() override = default;

    QString displayName() const override { return "Sub Cut"; }
    QString filterType() const override { return "ff-asubcut"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_cutoff = 20.0;  // 2 - 200 Hz, default 20
    int m_order = 10;        // 3 - 20, filter order, default 10
    double m_level = 1.0;    // 0 - 1, input level, default 1

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
