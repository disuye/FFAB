#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFAsubboost - Wraps FFmpeg's 'asubboost' audio filter
 * Boost subwoofer frequencies
 * FFmpeg filter: asubboost=dry=<d>:wet=<w>:boost=<b>:decay=<d>:feedback=<f>:cutoff=<c>:slope=<s>:delay=<d>
 */
class FFAsubboost : public BaseFilter {
    Q_OBJECT
public:
    FFAsubboost();
    ~FFAsubboost() override = default;

    QString displayName() const override { return "Sub Boost"; }
    QString filterType() const override { return "ff-asubboost"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_dry = 1.0;       // 0 - 1, default 1
    double m_wet = 1.0;       // 0 - 1, default 1
    double m_boost = 2.0;     // 1 - 12, default 2
    double m_decay = 0.0;     // 0 - 1, default 0
    double m_feedback = 0.9;  // 0 - 1, default 0.9
    double m_cutoff = 100.0;  // 50 - 900 Hz, default 100
    double m_slope = 0.5;     // 0.0001 - 1, default 0.5
    double m_delay = 20.0;    // 1 - 100 ms, default 20

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* drySlider = nullptr;
    QDoubleSpinBox* drySpinBox = nullptr;
    QSlider* wetSlider = nullptr;
    QDoubleSpinBox* wetSpinBox = nullptr;
    QSlider* boostSlider = nullptr;
    QDoubleSpinBox* boostSpinBox = nullptr;
    QSlider* decaySlider = nullptr;
    QDoubleSpinBox* decaySpinBox = nullptr;
    QSlider* feedbackSlider = nullptr;
    QDoubleSpinBox* feedbackSpinBox = nullptr;
    QSlider* cutoffSlider = nullptr;
    QDoubleSpinBox* cutoffSpinBox = nullptr;
    QSlider* slopeSlider = nullptr;
    QDoubleSpinBox* slopeSpinBox = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
};
