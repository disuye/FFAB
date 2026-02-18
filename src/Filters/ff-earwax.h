#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

/**
 * FFEarwax - Wraps FFmpeg's 'earwax' audio filter
 * Widen the stereo image (fixed effect, no parameters)
 * FFmpeg filter: earwax
 * 
 * This filter makes audio easier to listen to on headphones by
 * adding 'cues' so that audio appears to come from outside the head.
 */
class FFEarwax : public BaseFilter {
    Q_OBJECT
public:
    FFEarwax();
    ~FFEarwax() override = default;

    QString displayName() const override { return "Earwax"; }
    QString filterType() const override { return "ff-earwax"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    QString ffmpegFlags;
    QWidget* parametersWidget = nullptr;
};
