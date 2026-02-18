#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLineEdit;

/**
 * FFChorus - Wraps FFmpeg's 'chorus' audio filter
 * Add a chorus effect to the audio
 * FFmpeg filter: chorus=in_gain=<g>:out_gain=<g>:delays=<d>:decays=<d>:speeds=<s>:depths=<d>
 */
class FFChorus : public BaseFilter {
    Q_OBJECT
public:
    FFChorus();
    ~FFChorus() override = default;

    QString displayName() const override { return "Chorus"; }
    QString filterType() const override { return "ff-chorus"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_inGain = 0.4;                  // 0 - 1, default 0.4
    double m_outGain = 0.4;                 // 0 - 1, default 0.4
    QString m_delays = "500|1200|2000";     // pipe-separated milliseconds
    QString m_decays = "0.8|0.2|0.67";      // pipe-separated decay values
    QString m_speeds = "1|5|10";            // pipe-separated Hz values
    QString m_depths = "100|10|1";          // pipe-separated depth values

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* inGainSlider = nullptr;
    QDoubleSpinBox* inGainSpinBox = nullptr;
    QSlider* outGainSlider = nullptr;
    QDoubleSpinBox* outGainSpinBox = nullptr;
    QLineEdit* delaysEdit = nullptr;
    QLineEdit* decaysEdit = nullptr;
    QLineEdit* speedsEdit = nullptr;
    QLineEdit* depthsEdit = nullptr;
};
