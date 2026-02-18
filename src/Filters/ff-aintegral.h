#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFAintegral - Wraps FFmpeg's 'aintegral' audio filter
 * Compute integral of input audio.
 * FFmpeg filter: aintegral
 * 
 * Calculates the integral (cumulative sum) of the audio signal.
 * This is a low-pass filter effect that smooths the signal and
 * attenuates high frequencies. The inverse operation of aderivative.
 */
class FFAintegral : public BaseFilter {
    Q_OBJECT
public:
    FFAintegral();
    ~FFAintegral() override = default;

    QString displayName() const override { return "Integral"; }
    QString filterType() const override { return "ff-aintegral"; }
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
