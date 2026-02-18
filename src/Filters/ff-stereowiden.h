#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFStereowiden - Wraps FFmpeg's 'stereowiden' audio filter
 * Apply stereo widening effect using delay-based processing
 * FFmpeg filter: stereowiden=delay=<d>:feedback=<f>:crossfeed=<c>:drymix=<m>
 */
class FFStereowiden : public BaseFilter {
    Q_OBJECT
public:
    FFStereowiden();
    ~FFStereowiden() override = default;

    QString displayName() const override { return "Stereo Widen"; }
    QString filterType() const override { return "ff-stereowiden"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_delay = 20.0;          // 1 - 100 ms, default 20
    double m_feedback = 0.3;        // 0 - 0.9, default 0.3
    double m_crossfeed = 0.3;       // 0 - 0.8, default 0.3
    double m_drymix = 0.8;          // 0 - 1, default 0.8

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* feedbackSpinBox = nullptr;
    QSlider* feedbackSlider = nullptr;
    QDoubleSpinBox* crossfeedSpinBox = nullptr;
    QSlider* crossfeedSlider = nullptr;
    QDoubleSpinBox* drymixSpinBox = nullptr;
    QSlider* drymixSlider = nullptr;
};
