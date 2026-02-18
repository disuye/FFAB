#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QLineEdit;
class QLabel;

/**
 * FFAcrossover - Wraps FFmpeg's 'acrossover' audio filter
 * Split audio into per-band streams.
 * FFmpeg filter: acrossover=split=<freqs>:order=<o>:level=<l>:gain=<g>:precision=<p>
 * 
 * This is a multi-output filter that splits the input audio into
 * frequency bands using Linkwitz-Riley crossover filters. Each band
 * is output as a separate stream.
 * 
 * WARNING: Produces MULTIPLE OUTPUT STREAMS!
 */
class FFAcrossover : public BaseFilter {
    Q_OBJECT
public:
    FFAcrossover();
    ~FFAcrossover() override = default;

    QString displayName() const override { return "Crossover"; }
    QString filterType() const override { return "ff-acrossover"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_split = "500";       // Split frequencies (space or | separated)
    int m_order = 4;               // Filter order: 0=2nd, 1=4th, 2=6th, 3=8th, 4=10th...9=20th
    double m_level = 1.0;          // Input gain (0-1)
    QString m_gain = "1.f";        // Per-band output gains
    int m_precision = 0;           // 0=auto, 1=float, 2=double

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QLineEdit* splitEdit = nullptr;
    QComboBox* orderCombo = nullptr;
    QSlider* levelSlider = nullptr;
    QDoubleSpinBox* levelSpinBox = nullptr;
    QLineEdit* gainEdit = nullptr;
    QComboBox* precisionCombo = nullptr;
};
