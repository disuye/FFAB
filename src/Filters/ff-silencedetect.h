#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;
class QLabel;

/**
 * FFSilencedetect - Wraps FFmpeg's 'silencedetect' audio filter
 * Detect silence in audio.
 * FFmpeg filter: silencedetect=n=<noise>:d=<duration>:mono=<m>
 * 
 * This is an analysis filter that detects silent sections in audio.
 * Audio passes through unchanged while being analyzed.
 * 
 * Output includes silence_start and silence_end timestamps to stderr.
 */
class FFSilencedetect : public BaseFilter {
    Q_OBJECT
public:
    FFSilencedetect();
    ~FFSilencedetect() override = default;

    QString displayName() const override { return "Silence Detect"; }
    QString filterType() const override { return "ff-silencedetect"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_noise = 0.001;     // Noise tolerance (linear), default 0.001 (~-60dB)
    double m_duration = 2.0;   // Minimum silence duration in seconds, default 2
    bool m_mono = false;       // Check each channel separately, default false

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* noiseSlider = nullptr;
    QDoubleSpinBox* noiseSpinBox = nullptr;
    QSlider* durationSlider = nullptr;
    QDoubleSpinBox* durationSpinBox = nullptr;
    QCheckBox* monoCheck = nullptr;
};
