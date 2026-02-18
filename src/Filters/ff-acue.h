#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QDoubleSpinBox;
class QLabel;

/**
 * FFAcue - Wraps FFmpeg's 'acue' audio filter
 * Delay filtering to match a cue point.
 * FFmpeg filter: acue=cue=<timestamp>:preroll=<duration>:buffer=<duration>
 * 
 * Used for synchronized playback. The filter delays output until 
 * the system clock matches the specified cue timestamp, allowing
 * precise synchronization with external events or other streams.
 */
class FFAcue : public BaseFilter {
    Q_OBJECT
public:
    FFAcue();
    ~FFAcue() override = default;

    QString displayName() const override { return "Audio Cue"; }
    QString filterType() const override { return "ff-acue"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    qint64 m_cue = 0;         // Unix timestamp in microseconds, default 0
    double m_preroll = 0.0;   // Preroll duration in seconds, default 0
    double m_buffer = 0.0;    // Buffer duration in seconds, default 0

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* cueSpinBox = nullptr;
    QDoubleSpinBox* prerollSpinBox = nullptr;
    QDoubleSpinBox* bufferSpinBox = nullptr;
};
