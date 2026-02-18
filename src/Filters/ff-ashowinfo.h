#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFAshowinfo - Wraps FFmpeg's 'ashowinfo' audio filter
 * Show textual information for each audio frame.
 * FFmpeg filter: ashowinfo
 * 
 * This is an analysis/debug filter that outputs detailed information
 * about each audio frame to stderr. Audio passes through unchanged.
 * 
 * Output includes per-frame:
 * - n: Frame sequence number
 * - pts: Presentation timestamp
 * - pts_time: PTS in seconds
 * - fmt: Sample format
 * - chlayout: Channel layout
 * - rate: Sample rate
 * - nb_samples: Number of samples
 * - checksum: Audio data checksum
 */
class FFAshowinfo : public BaseFilter {
    Q_OBJECT
public:
    FFAshowinfo();
    ~FFAshowinfo() override = default;

    QString displayName() const override { return "Show Info"; }
    QString filterType() const override { return "ff-ashowinfo"; }
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
