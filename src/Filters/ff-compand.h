#pragma once

#include "BaseFilter.h"
#include "CompandBandData.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class CompandCurveWidget;

/**
 * FFCompand - Wraps FFmpeg's 'compand' audio filter
 * Compress or expand audio dynamic range
 * FFmpeg filter: compand=attacks=<a>:decays=<d>:points=<p>:soft-knee=<k>:gain=<g>:volume=<v>:delay=<d>
 */
class FFCompand : public BaseFilter {
    Q_OBJECT
public:
    FFCompand();
    ~FFCompand() override = default;

    QString displayName() const override { return "Compand"; }
    QString filterType() const override { return "ff-compand"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    // Public API (used by legacy code paths / direct callers)
    const QList<CompandPoint>& transferPoints() const { return m_bandData.points; }
    double softKnee() const { return m_bandData.softKnee; }
    void setSoftKnee(double knee);

private:
    void updateFFmpegFlags();
    QString pointsToString() const;
    void parsePointsString(const QString& str);

    // Parameters
    CompandBandData m_bandData;
    double m_attack = 0.05;             // seconds (50ms default)
    double m_decay = 0.03;              // seconds (30ms default)
    double m_gain = -12.0;              // -900 to 900 dB
    double m_volume = -90.0;            // -900 to 0 dB
    double m_delay = 0.01;              // 0 to 20 seconds

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    CompandCurveWidget* curveWidget = nullptr;
    QSlider* attackSlider = nullptr;
    QDoubleSpinBox* attackSpinBox = nullptr;
    QSlider* decaySlider = nullptr;
    QDoubleSpinBox* decaySpinBox = nullptr;
    QSlider* softKneeSlider = nullptr;
    QDoubleSpinBox* softKneeSpinBox = nullptr;
    QSlider* gainSlider = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QSlider* volumeSlider = nullptr;
    QDoubleSpinBox* volumeSpinBox = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
};
