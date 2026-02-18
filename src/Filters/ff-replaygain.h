#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFReplaygain - Wraps FFmpeg's 'replaygain' audio filter
 * ReplayGain scanner - measures loudness for normalization.
 * FFmpeg filter: replaygain
 * 
 * This is an analysis filter that calculates ReplayGain values.
 * Audio passes through unchanged while being analyzed.
 * 
 * Output includes:
 * - track_gain: Suggested gain adjustment in dB
 * - track_peak: Peak sample value (0.0-1.0)
 */
class FFReplaygain : public BaseFilter {
    Q_OBJECT
public:
    FFReplaygain();
    ~FFReplaygain() override = default;

    QString displayName() const override { return "ReplayGain"; }
    QString filterType() const override { return "ff-replaygain"; }
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
