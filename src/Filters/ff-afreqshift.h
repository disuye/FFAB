#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFAfreqshift - Wraps FFmpeg's 'afreqshift' audio filter
 * Apply frequency shifting to input audio
 * FFmpeg filter: afreqshift=shift=<s>:level=<l>:order=<o>
 */
class FFAfreqshift : public BaseFilter {
    Q_OBJECT
public:
    FFAfreqshift();
    ~FFAfreqshift() override = default;

    QString displayName() const override { return "Frequency Shift"; }
    QString filterType() const override { return "ff-afreqshift"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_shift = 0.0;    // -2147480000 - max Hz, default 0
    double m_level = 1.0;    // 0 - 1, default 1
    int m_order = 8;         // 1 - 16, default 8

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* shiftSlider = nullptr;
    QDoubleSpinBox* shiftSpinBox = nullptr;
    QSlider* levelSlider = nullptr;
    QDoubleSpinBox* levelSpinBox = nullptr;
    QSlider* orderSlider = nullptr;
    QDoubleSpinBox* orderSpinBox = nullptr;
};
