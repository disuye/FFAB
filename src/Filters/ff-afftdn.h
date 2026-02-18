#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFAfftdn - Wraps FFmpeg's 'afftdn' audio filter
 * Denoise audio samples using FFT
 * FFmpeg filter: afftdn=nr=<n>:nf=<n>:nt=<t>:rf=<r>:tn=<t>:tr=<t>:om=<o>
 */
class FFAfftdn : public BaseFilter {
    Q_OBJECT
public:
    FFAfftdn();
    ~FFAfftdn() override = default;

    QString displayName() const override { return "FFT Denoise"; }
    QString filterType() const override { return "ff-afftdn"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_noiseReduction = 12.0;  // 0.01 - 97 dB, default 12
    double m_noiseFloor = -50.0;     // -80 - -20 dB, default -50
    int m_noiseType = 0;             // 0=white, 1=vinyl, 2=shellac, 3=custom, default white
    double m_residualFloor = -38.0;  // -80 - -20 dB, default -38
    bool m_trackNoise = false;       // track noise, default false
    bool m_trackResidual = false;    // track residual, default false
    int m_outputMode = 0;            // 0=output, 1=input, 2=noise, default output

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* noiseReductionSlider = nullptr;
    QDoubleSpinBox* noiseReductionSpinBox = nullptr;
    QSlider* noiseFloorSlider = nullptr;
    QDoubleSpinBox* noiseFloorSpinBox = nullptr;
    QComboBox* noiseTypeCombo = nullptr;
    QSlider* residualFloorSlider = nullptr;
    QDoubleSpinBox* residualFloorSpinBox = nullptr;
    QCheckBox* trackNoiseCheckBox = nullptr;
    QCheckBox* trackResidualCheckBox = nullptr;
    QComboBox* outputModeCombo = nullptr;
};
