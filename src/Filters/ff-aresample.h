#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;

/**
 * FFAresample - Wraps FFmpeg's 'aresample' audio filter
 * Resamples audio data
 * FFmpeg filter: aresample=<rate>
 * Range: 0 to INT_MAX (default: 0, meaning no change)
 */
class FFAresample : public BaseFilter {
    Q_OBJECT
public:
    FFAresample();
    ~FFAresample() override = default;

    QString displayName() const override { return "Resample"; }
    QString filterType() const override { return "ff-aresample"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    int m_sampleRate = 44100;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QSpinBox* sampleRateSpinBox = nullptr;
};
