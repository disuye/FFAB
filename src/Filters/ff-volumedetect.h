#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFVolumedetect - Wraps FFmpeg's 'volumedetect' audio filter
 * Detect audio volume statistics.
 * FFmpeg filter: volumedetect
 * 
 * This is an analysis filter that outputs volume statistics to stderr.
 * It does not modify the audio - it passes through unchanged while measuring.
 * 
 * Output includes:
 * - n_samples: Number of samples
 * - mean_volume: Mean volume in dB
 * - max_volume: Maximum volume in dB
 * - histogram: Volume histogram data
 */
class FFVolumedetect : public BaseFilter {
    Q_OBJECT
public:
    FFVolumedetect();
    ~FFVolumedetect() override = default;

    QString displayName() const override { return "Volume Detect"; }
    QString filterType() const override { return "ff-volumedetect"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
};
