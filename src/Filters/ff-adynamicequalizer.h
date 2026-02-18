#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFAdynamicequalizer - Wraps FFmpeg's 'adynamicequalizer' audio filter
 * Apply Dynamic Equalization based on detected signal levels
 * FFmpeg filter: adynamicequalizer=threshold=<t>:dfrequency=<df>:dqfactor=<dq>:tfrequency=<tf>:tqfactor=<tq>:attack=<a>:release=<r>:ratio=<r>:makeup=<m>:range=<r>:mode=<m>:dftype=<dt>:tftype=<tt>:auto=<a>
 */
class FFAdynamicequalizer : public BaseFilter {
    Q_OBJECT
public:
    FFAdynamicequalizer();
    ~FFAdynamicequalizer() override = default;

    QString displayName() const override { return "Dynamic EQ"; }
    QString filterType() const override { return "ff-adynamicequalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_threshold = 0.0;       // 0 - 100 dB, default 0
    double m_dfrequency = 1000.0;   // 2 - 1000000 Hz, default 1000
    double m_dqfactor = 1.0;        // 0.001 - 1000, default 1
    double m_tfrequency = 1000.0;   // 2 - 1000000 Hz, default 1000
    double m_tqfactor = 1.0;        // 0.001 - 1000, default 1
    double m_attack = 20.0;         // 0.01 - 2000 ms, default 20
    double m_release = 200.0;       // 0.01 - 2000 ms, default 200
    double m_ratio = 1.0;           // 0 - 30, default 1
    double m_makeup = 0.0;          // 0 - 1000 dB, default 0
    double m_range = 50.0;          // 1 - 2000 dB, default 50
    int m_mode = 0;                 // -1=listen, 0=cutbelow, 1=cutabove, 2=boostbelow, 3=boostabove
    int m_dftype = 0;               // 0=bandpass, 1=lowpass, 2=highpass, 3=peak
    int m_tftype = 0;               // 0=bell, 1=lowshelf, 2=highshelf
    int m_autoThreshold = 1;        // 1=off, 2=on, 3=adaptive, 4=disabled

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Detection tab
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QSlider* thresholdSlider = nullptr;
    QDoubleSpinBox* dfrequencySpinBox = nullptr;
    QSlider* dfrequencySlider = nullptr;
    QDoubleSpinBox* dqfactorSpinBox = nullptr;
    QSlider* dqfactorSlider = nullptr;
    QComboBox* dftypeCombo = nullptr;
    
    // Target tab
    QDoubleSpinBox* tfrequencySpinBox = nullptr;
    QSlider* tfrequencySlider = nullptr;
    QDoubleSpinBox* tqfactorSpinBox = nullptr;
    QSlider* tqfactorSlider = nullptr;
    QComboBox* tftypeCombo = nullptr;
    
    // Dynamics tab
    QDoubleSpinBox* attackSpinBox = nullptr;
    QSlider* attackSlider = nullptr;
    QDoubleSpinBox* releaseSpinBox = nullptr;
    QSlider* releaseSlider = nullptr;
    QDoubleSpinBox* ratioSpinBox = nullptr;
    QSlider* ratioSlider = nullptr;
    
    // Output tab
    QDoubleSpinBox* makeupSpinBox = nullptr;
    QSlider* makeupSlider = nullptr;
    QDoubleSpinBox* rangeSpinBox = nullptr;
    QSlider* rangeSlider = nullptr;
    QComboBox* modeCombo = nullptr;
    QComboBox* autoCombo = nullptr;
};
