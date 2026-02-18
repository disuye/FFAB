#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFDcshift - Wraps FFmpeg's 'dcshift' audio filter
 * Apply a DC shift to the audio
 * FFmpeg filter: dcshift=shift:limitergain
 * shift: -1.0 to 1.0 (default: 0)
 * limitergain: 0.0 to 1.0 (default: 0)
 */
class FFDcshift : public BaseFilter {
    Q_OBJECT
public:
    FFDcshift();
    ~FFDcshift() override = default;

    QString displayName() const override { return "DC Shift"; }
    QString filterType() const override { return "ff-dcshift"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    double m_shift = 0.0;
    double m_limiterGain = 0.0;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QSlider* shiftSlider = nullptr;
    QDoubleSpinBox* shiftSpinBox = nullptr;
    QSlider* limiterSlider = nullptr;
    QDoubleSpinBox* limiterSpinBox = nullptr;
};
