#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFVirtualbass - Wraps FFmpeg's 'virtualbass' audio filter
 * Audio Virtual Bass - generates psychoacoustic bass enhancement
 * FFmpeg filter: virtualbass=cutoff=<c>:strength=<s>
 */
class FFVirtualbass : public BaseFilter {
    Q_OBJECT
public:
    FFVirtualbass();
    ~FFVirtualbass() override = default;

    QString displayName() const override { return "Virtual Bass"; }
    QString filterType() const override { return "ff-virtualbass"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_cutoff = 250.0;    // 100 - 500 Hz, default 250
    double m_strength = 3.0;    // 0.5 - 3, default 3

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* cutoffSlider = nullptr;
    QDoubleSpinBox* cutoffSpinBox = nullptr;
    QSlider* strengthSlider = nullptr;
    QDoubleSpinBox* strengthSpinBox = nullptr;
};
