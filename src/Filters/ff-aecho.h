#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLineEdit;

/**
 * FFAecho - Wraps FFmpeg's 'aecho' audio filter
 * Add echoing to the audio
 * FFmpeg filter: aecho=in_gain:out_gain:delays:decays
 * Example: aecho=0.6:0.3:1000|1800:0.5|0.3
 */
class FFAecho : public BaseFilter {
    Q_OBJECT
public:
    FFAecho();
    ~FFAecho() override = default;

    QString displayName() const override { return "Echo"; }
    QString filterType() const override { return "ff-aecho"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    double m_inGain = 0.6;
    double m_outGain = 0.3;
    QString m_delays = "1000|1800";
    QString m_decays = "0.5|0.3";
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QSlider* inGainSlider = nullptr;
    QDoubleSpinBox* inGainSpinBox = nullptr;
    QSlider* outGainSlider = nullptr;
    QDoubleSpinBox* outGainSpinBox = nullptr;
    QLineEdit* delaysEdit = nullptr;
    QLineEdit* decaysEdit = nullptr;
};
