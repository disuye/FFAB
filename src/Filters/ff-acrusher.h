#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFAcrusher - Wraps FFmpeg's 'acrusher' audio filter
 * Reduce audio bit resolution (bit crusher/sample rate reducer)
 * FFmpeg filter: acrusher=level_in=<l>:level_out=<l>:bits=<b>:mix=<m>:mode=<m>:dc=<d>:aa=<a>:samples=<s>:lfo=<l>:lforange=<r>:lforate=<r>
 */
class FFAcrusher : public BaseFilter {
    Q_OBJECT
public:
    FFAcrusher();
    ~FFAcrusher() override = default;

    QString displayName() const override { return "Bit Crusher"; }
    QString filterType() const override { return "ff-acrusher"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;     // 0.015625 - 64, default 1
    double m_levelOut = 1.0;    // 0.015625 - 64, default 1
    double m_bits = 8.0;        // 1 - 64, default 8
    double m_mix = 0.5;         // 0 - 1, default 0.5
    int m_mode = 0;             // 0=lin, 1=log, default lin
    double m_dc = 1.0;          // 0.25 - 4, default 1
    double m_aa = 0.5;          // 0 - 1, default 0.5
    double m_samples = 1.0;     // 1 - 250, default 1
    bool m_lfo = false;         // enable LFO, default false
    double m_lforange = 20.0;   // 1 - 250, default 20
    double m_lforate = 0.3;     // 0.01 - 200, default 0.3

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* bitsSlider = nullptr;
    QDoubleSpinBox* bitsSpinBox = nullptr;
    QSlider* mixSlider = nullptr;
    QDoubleSpinBox* mixSpinBox = nullptr;
    QComboBox* modeCombo = nullptr;
    QSlider* dcSlider = nullptr;
    QDoubleSpinBox* dcSpinBox = nullptr;
    QSlider* aaSlider = nullptr;
    QDoubleSpinBox* aaSpinBox = nullptr;
    QSlider* samplesSlider = nullptr;
    QDoubleSpinBox* samplesSpinBox = nullptr;
    QCheckBox* lfoCheckBox = nullptr;
    QSlider* lforangeSlider = nullptr;
    QDoubleSpinBox* lforangeSpinBox = nullptr;
    QSlider* lforateSlider = nullptr;
    QDoubleSpinBox* lforateSpinBox = nullptr;
};
