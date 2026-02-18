#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFCrystalizer - Wraps FFmpeg's 'crystalizer' audio filter
 * Simple audio noise sharpening filter
 * FFmpeg filter: crystalizer=i=<intensity>:c=<clip>
 */
class FFCrystalizer : public BaseFilter {
    Q_OBJECT
public:
    FFCrystalizer();
    ~FFCrystalizer() override = default;

    QString displayName() const override { return "Crystalizer"; }
    QString filterType() const override { return "ff-crystalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_intensity = 2.0;  // -10 - 10, default 2
    bool m_clip = true;        // enable clipping, default true

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* intensitySlider = nullptr;
    QDoubleSpinBox* intensitySpinBox = nullptr;
    QCheckBox* clipCheckBox = nullptr;
};
