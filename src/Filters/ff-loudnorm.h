#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;

/**
 * FFLoudnorm - Wraps FFmpeg's 'loudnorm' audio filter
 * EBU R128 loudness normalization for broadcast/streaming compliance
 * FFmpeg filter: loudnorm=I=<i>:LRA=<lra>:TP=<tp>:offset=<o>:linear=<l>:dual_mono=<d>:print_format=<p>
 */
class FFLoudnorm : public BaseFilter {
    Q_OBJECT
public:
    FFLoudnorm();
    ~FFLoudnorm() override = default;

    QString displayName() const override { return "Loudness Norm"; }
    QString filterType() const override { return "ff-loudnorm"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_integratedLoudness = -24.0;  // -70 to -5 LUFS, default -24
    double m_loudnessRange = 7.0;          // 1 - 50 LU, default 7
    double m_truePeak = -2.0;              // -9 to 0 dBTP, default -2
    double m_offset = 0.0;                 // -99 to 99 dB, default 0
    bool m_linear = true;                  // default true
    bool m_dualMono = false;               // default false
    int m_printFormat = 0;                 // 0=none, 1=json, 2=summary

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* integratedSpinBox = nullptr;
    QSlider* integratedSlider = nullptr;
    QDoubleSpinBox* lraSpinBox = nullptr;
    QSlider* lraSlider = nullptr;
    QDoubleSpinBox* truePeakSpinBox = nullptr;
    QSlider* truePeakSlider = nullptr;
    QDoubleSpinBox* offsetSpinBox = nullptr;
    QSlider* offsetSlider = nullptr;
    QCheckBox* linearCheck = nullptr;
    QCheckBox* dualMonoCheck = nullptr;
    QComboBox* printFormatCombo = nullptr;
};
