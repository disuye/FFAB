#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFAderivative - Wraps FFmpeg's 'aderivative' audio filter
 * Compute derivative of input audio.
 * FFmpeg filter: aderivative
 * 
 * Calculates the derivative (rate of change) of the audio signal.
 * This is a high-pass filter effect that emphasizes transients and
 * removes DC offset and low frequencies. Useful for edge detection
 * in audio analysis or creating special effects.
 */
class FFAderivative : public BaseFilter {
    Q_OBJECT
public:
    FFAderivative();
    ~FFAderivative() override = default;

    QString displayName() const override { return "Derivative"; }
    QString filterType() const override { return "ff-aderivative"; }
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
