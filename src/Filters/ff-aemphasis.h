#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QLabel;

/**
 * FFAemphasis - Wraps FFmpeg's 'aemphasis' audio filter
 * Audio pre-emphasis and de-emphasis filter.
 * FFmpeg filter: aemphasis=level_in=<l>:level_out=<l>:mode=<m>:type=<t>
 * 
 * Applies standard emphasis curves used in various audio formats
 * (vinyl, tape, CD, FM radio, etc.) for noise reduction.
 */
class FFAemphasis : public BaseFilter {
    Q_OBJECT
public:
    FFAemphasis();
    ~FFAemphasis() override = default;

    QString displayName() const override { return "Emphasis"; }
    QString filterType() const override { return "ff-aemphasis"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;   // 0 - 64, default 1 (linear gain)
    double m_levelOut = 1.0;  // 0 - 64, default 1 (linear gain)
    int m_mode = 0;           // 0=reproduction (de-emphasis), 1=production (pre-emphasis)
    int m_type = 0;           // 0=cd, 1=50fm, 2=75fm, 3=50kf, 4=75kf, 5=riaa, 6=emi, 7=emi2, 8=col

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QComboBox* modeCombo = nullptr;
    QComboBox* typeCombo = nullptr;
};
