#pragma once

#include "BaseFilter.h"

class QLabel;

/**
 * FFAnullsink - Audio Null Sink (Destroy Audio)
 * 
 * Consumes and discards the audio stream. Useful when you only want
 * non-audio outputs (like waveform images) without producing an audio file.
 * 
 * When this filter is at the end of the main chain, FilterChain will:
 * - Not generate a [out] label
 * - Skip the main audio output mapping entirely
 * 
 * FFmpeg docs: https://ffmpeg.org/ffmpeg-filters.html#anullsink
 */
class FFAnullsink : public BaseFilter {
    Q_OBJECT
public:
    FFAnullsink();
    ~FFAnullsink() override = default;

    QString displayName() const override { return "Null Sink (Discard Audio)"; }
    QString filterType() const override { return "ff-anullsink"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;
    
    // Identifies this as a sink filter (no output)
    bool isSinkFilter() const { return true; }

private:
    QWidget* parametersWidget = nullptr;
};
