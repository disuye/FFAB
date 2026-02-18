#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFAphaseshift - Wraps FFmpeg's 'aphaseshift' audio filter
 * Apply phase shifting to input audio
 * FFmpeg filter: aphaseshift=shift=<s>:level=<l>:order=<o>
 */
class FFAphaseshift : public BaseFilter {
    Q_OBJECT
public:
    FFAphaseshift();
    ~FFAphaseshift() override = default;

    QString displayName() const override { return "Phase Shift"; }
    QString filterType() const override { return "ff-aphaseshift"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_shift = 0.0;    // -1 to 1, default 0
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
