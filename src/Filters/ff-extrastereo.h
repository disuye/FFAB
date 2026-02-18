#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFExtrastereo - Wraps FFmpeg's 'extrastereo' audio filter
 * Increase difference between stereo audio channels
 * FFmpeg filter: extrastereo=m=<coefficient>:c=<clipping>
 */
class FFExtrastereo : public BaseFilter {
    Q_OBJECT
public:
    FFExtrastereo();
    ~FFExtrastereo() override = default;

    QString displayName() const override { return "Extra Stereo"; }
    QString filterType() const override { return "ff-extrastereo"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_coefficient = 2.5;     // -10 to 10, default 2.5 (stereo difference multiplier)
    bool m_clipping = true;         // enable clipping, default true

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* coefficientSpinBox = nullptr;
    QSlider* coefficientSlider = nullptr;
    QCheckBox* clippingCheck = nullptr;
};
