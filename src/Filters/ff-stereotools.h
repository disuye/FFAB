#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFStereotools - Wraps FFmpeg's 'stereotools' audio filter
 * Apply various stereo tools including M/S processing, balance, phase rotation
 * FFmpeg filter: stereotools=level_in=<li>:level_out=<lo>:balance_in=<bi>:balance_out=<bo>:...
 */
class FFStereotools : public BaseFilter {
    Q_OBJECT
public:
    FFStereotools();
    ~FFStereotools() override = default;

    QString displayName() const override { return "Stereo Tools"; }
    QString filterType() const override { return "ff-stereotools"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    double dbToLinear(double db) const;
    double linearToDb(double linear) const;

    // Parameters - Tab 1: Levels
    double m_levelIn = 1.0;         // 0.015625 - 64 (display as dB)
    double m_levelOut = 1.0;        // 0.015625 - 64 (display as dB)
    double m_balanceIn = 0.0;       // -1 to 1 (display as %)
    double m_balanceOut = 0.0;      // -1 to 1 (display as %)
    int m_bmodeIn = 0;              // 0=balance, 1=amplitude, 2=power
    int m_bmodeOut = 0;             // 0=balance, 1=amplitude, 2=power
    
    // Parameters - Tab 2: M/S Processing
    int m_mode = 0;                 // 0=lr>lr, 1=lr>ms, 2=ms>lr, 3=lr>ll, etc.
    double m_slev = 1.0;            // side level, 0.015625-64 (dB)
    double m_sbal = 0.0;            // side balance, -1 to 1
    double m_mlev = 1.0;            // middle level, 0.015625-64 (dB)
    double m_mpan = 0.0;            // middle pan, -1 to 1
    double m_base = 0.0;            // stereo base, -1 to 1
    double m_delay = 0.0;           // -20 to 20 ms
    double m_phase = 0.0;           // 0 to 360 degrees
    double m_sclevel = 1.0;         // S/C level, 1 to 100
    
    // Parameters - Tab 3: Options
    bool m_softclip = false;
    bool m_mutel = false;
    bool m_muter = false;
    bool m_phasel = false;
    bool m_phaser = false;

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Tab 1 - Levels
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* balanceInSpinBox = nullptr;
    QSlider* balanceInSlider = nullptr;
    QDoubleSpinBox* balanceOutSpinBox = nullptr;
    QSlider* balanceOutSlider = nullptr;
    QComboBox* bmodeInCombo = nullptr;
    QComboBox* bmodeOutCombo = nullptr;
    
    // Tab 2 - M/S Processing
    QComboBox* modeCombo = nullptr;
    QDoubleSpinBox* slevSpinBox = nullptr;
    QSlider* slevSlider = nullptr;
    QDoubleSpinBox* sbalSpinBox = nullptr;
    QSlider* sbalSlider = nullptr;
    QDoubleSpinBox* mlevSpinBox = nullptr;
    QSlider* mlevSlider = nullptr;
    QDoubleSpinBox* mpanSpinBox = nullptr;
    QSlider* mpanSlider = nullptr;
    QDoubleSpinBox* baseSpinBox = nullptr;
    QSlider* baseSlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* phaseSpinBox = nullptr;
    QSlider* phaseSlider = nullptr;
    QDoubleSpinBox* sclevelSpinBox = nullptr;
    QSlider* sclevelSlider = nullptr;
    
    // Tab 3 - Options
    QCheckBox* softclipCheck = nullptr;
    QCheckBox* mutelCheck = nullptr;
    QCheckBox* muterCheck = nullptr;
    QCheckBox* phaselCheck = nullptr;
    QCheckBox* phaserCheck = nullptr;
};
