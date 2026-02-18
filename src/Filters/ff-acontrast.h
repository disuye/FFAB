#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFAcontrast - Wraps FFmpeg's 'acontrast' audio filter
 * Simple audio dynamic range compression/expansion
 * FFmpeg filter: acontrast=contrast=<c>
 */
class FFAcontrast : public BaseFilter {
    Q_OBJECT
public:
    FFAcontrast();
    ~FFAcontrast() override = default;

    QString displayName() const override { return "Contrast"; }
    QString filterType() const override { return "ff-acontrast"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_contrast = 33.0;  // 0 - 100, default 33

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* contrastSpinBox = nullptr;
    QSlider* contrastSlider = nullptr;
};
