#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFAsoftclip - Wraps FFmpeg's 'asoftclip' audio filter
 * Audio Soft Clipper
 * FFmpeg filter: asoftclip=type=<type>:threshold=<threshold>:output=<output>:param=<param>:oversample=<oversample>
 */
class FFAsoftclip : public BaseFilter {
    Q_OBJECT
public:
    FFAsoftclip();
    ~FFAsoftclip() override = default;

    QString displayName() const override { return "Soft Clipper"; }
    QString filterType() const override { return "ff-asoftclip"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    int m_type = 0;          // -1 to 7, default tanh(0): tanh, atan, cubic, exp, alg, quintic, sin, erf
    double m_threshold = 1.0;  // 0.000001 - 1, default 1
    double m_output = 1.0;     // 0.000001 - 16, default 1
    double m_param = 1.0;      // 0.01 - 3, default 1
    int m_oversample = 1;      // 1 - 64, default 1

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QComboBox* typeCombo = nullptr;
    QSlider* thresholdSlider = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QSlider* outputSlider = nullptr;
    QDoubleSpinBox* outputSpinBox = nullptr;
    QSlider* paramSlider = nullptr;
    QDoubleSpinBox* paramSpinBox = nullptr;
    QSlider* oversampleSlider = nullptr;
    QDoubleSpinBox* oversampleSpinBox = nullptr;
};
