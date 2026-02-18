#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFAexciter - Wraps FFmpeg's 'aexciter' audio filter
 * Enhance high frequency part of audio (harmonic exciter)
 * FFmpeg filter: aexciter=level_in=<l>:level_out=<l>:amount=<a>:drive=<d>:blend=<b>:freq=<f>:ceil=<c>:listen=<l>
 */
class FFAexciter : public BaseFilter {
    Q_OBJECT
public:
    FFAexciter();
    ~FFAexciter() override = default;

    QString displayName() const override { return "Exciter"; }
    QString filterType() const override { return "ff-aexciter"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;     // 0 - 64, default 1
    double m_levelOut = 1.0;    // 0 - 64, default 1
    double m_amount = 1.0;      // 0 - 64, default 1
    double m_drive = 8.5;       // 0.1 - 10, default 8.5
    double m_blend = 0.0;       // -10 - 10, default 0
    double m_freq = 7500.0;     // 2000 - 12000 Hz, default 7500
    double m_ceil = 9999.0;     // 9999 - 20000 Hz, default 9999
    bool m_listen = false;      // listen mode, default false

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* amountSlider = nullptr;
    QDoubleSpinBox* amountSpinBox = nullptr;
    QSlider* driveSlider = nullptr;
    QDoubleSpinBox* driveSpinBox = nullptr;
    QSlider* blendSlider = nullptr;
    QDoubleSpinBox* blendSpinBox = nullptr;
    QSlider* freqSlider = nullptr;
    QDoubleSpinBox* freqSpinBox = nullptr;
    QSlider* ceilSlider = nullptr;
    QDoubleSpinBox* ceilSpinBox = nullptr;
    QCheckBox* listenCheckBox = nullptr;
};
