#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFDeesser - Wraps FFmpeg's 'deesser' audio filter
 * Reduces sibilance in vocal recordings
 * FFmpeg filter: deesser=i=<intensity>:m=<max>:f=<freq>:s=<mode>
 */
class FFDeesser : public BaseFilter {
    Q_OBJECT
public:
    FFDeesser();
    ~FFDeesser() override = default;

    QString displayName() const override { return "De-esser"; }
    QString filterType() const override { return "ff-deesser"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_intensity = 0.0;    // 0 - 1, default 0
    double m_max = 0.5;          // 0 - 1, default 0.5
    double m_frequency = 0.5;    // 0 - 1 (normalized), default 0.5
    int m_outputMode = 0;        // 0=output, 1=input, 2=noise, default 0

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* intensitySpinBox = nullptr;
    QSlider* intensitySlider = nullptr;
    QDoubleSpinBox* maxSpinBox = nullptr;
    QSlider* maxSlider = nullptr;
    QDoubleSpinBox* frequencySpinBox = nullptr;
    QSlider* frequencySlider = nullptr;
    QComboBox* outputModeCombo = nullptr;
};
