#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;

/**
 * FFAsetrate - Wraps FFmpeg's 'asetrate' audio filter
 * Changes the sample rate without altering the data
 * FFmpeg filter: asetrate=<rate>
 * Range: 1 to INT_MAX (default: 44100)
 */
class FFAsetrate : public BaseFilter {
    Q_OBJECT
public:
    FFAsetrate();
    ~FFAsetrate() override = default;

    QString displayName() const override { return "Set Rate"; }
    QString filterType() const override { return "ff-asetrate"; }
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
