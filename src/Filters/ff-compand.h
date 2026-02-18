#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLineEdit;
class QSlider;
class QDoubleSpinBox;

/**
 * FFCompand - Wraps FFmpeg's 'compand' audio filter
 * Compress or expand audio dynamic range
 * FFmpeg filter: compand=attacks=<a>:decays=<d>:points=<p>:gain=<g>:volume=<v>:delay=<d>
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

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_attacks = "0";                      // string: attack times
    QString m_decays = "0.8";                     // string: decay times
    QString m_points = "-70/-70|-60/-20|1/0";     // string: transfer function points
    double m_gain = 0.0;                          // -900 to 900 dB, default 0
    double m_volume = 0.0;                        // -900 to 0 dB, default 0
    double m_delay = 0.0;                         // 0 to 20 seconds, default 0

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QLineEdit* attacksEdit = nullptr;
    QLineEdit* decaysEdit = nullptr;
    QLineEdit* pointsEdit = nullptr;
    QSlider* gainSlider = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QSlider* volumeSlider = nullptr;
    QDoubleSpinBox* volumeSpinBox = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
};
