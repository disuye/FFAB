#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QLabel;
class QTextEdit;

/**
 * FFPan - Wraps FFmpeg's 'pan' audio filter
 * Remix channels with coefficients (panning).
 * FFmpeg filter: pan=<layout>|<output_def>|<output_def>|...
 * 
 * Allows arbitrary mixing of input channels to output channels
 * with gain coefficients. Extremely flexible for channel routing,
 * downmixing, upmixing, and custom spatial mixes.
 * 
 * Syntax: pan=<output_layout>|c0=<mix>|c1=<mix>|...
 * Where <mix> can be: <gain>*<input_channel> or <gain>*c<n>
 */
class FFPan : public BaseFilter {
    Q_OBJECT
public:
    FFPan();
    ~FFPan() override = default;

    QString displayName() const override { return "Pan / Remix"; }
    QString filterType() const override { return "ff-pan"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    void applyPreset(int presetIndex);

    // Parameters
    QString m_outputLayout = "stereo";
    QString m_channelMap = "c0=c0|c1=c1";

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QComboBox* layoutCombo = nullptr;
    QComboBox* presetCombo = nullptr;
    QTextEdit* channelMapEdit = nullptr;
};
