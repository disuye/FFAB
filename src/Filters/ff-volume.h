#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLabel;

/**
 * FFVolume - Wraps FFmpeg's 'volume' audio filter
 * Adjusts audio volume in decibels
 * FFmpeg filter: volume=<volume>dB
 */
class FFVolume : public BaseFilter {
    Q_OBJECT
public:
    FFVolume();
    ~FFVolume() override = default;

    QString displayName() const override { return "Volume"; }
    QString filterType() const override { return "ff-volume"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    void setDecibels(double db) { decibels = db; updateFFmpegFlags(); }
    double getDecibels() const { return decibels; }

private:
    void updateFFmpegFlags();

    double decibels = -0.3;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QSlider* volumeSlider = nullptr;
    QDoubleSpinBox* decibelSpinBox = nullptr;
    QLabel* decibelLabel = nullptr;
};
