#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QTextEdit;
class QLabel;

/**
 * FFFirequalizer - Wraps FFmpeg's 'firequalizer' audio filter
 * Finite Impulse Response Equalizer.
 * FFmpeg filter: firequalizer=gain=<expr>:gain_entry=<e>:delay=<d>:accuracy=<a>:...
 * 
 * A powerful FIR-based equalizer that uses expressions to define
 * the frequency response curve. Supports arbitrary EQ curves,
 * linear phase operation, and minimum phase modes.
 */
class FFFirequalizer : public BaseFilter {
    Q_OBJECT
public:
    FFFirequalizer();
    ~FFFirequalizer() override = default;

    QString displayName() const override { return "FIR Equalizer"; }
    QString filterType() const override { return "ff-firequalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    void applyPreset(int presetIndex);

    // Parameters
    QString m_gainEntry = "";         // Gain entry points (freq|gain pairs)
    double m_delay = 0.01;            // Filter delay in seconds
    double m_accuracy = 5.0;          // Accuracy (affects filter length)
    int m_wfunc = 1;                  // Window function
    bool m_fixed = false;             // Fixed frame samples
    bool m_multi = false;             // Multi-channel mode
    bool m_zeroPhase = false;         // Zero phase mode
    bool m_minPhase = false;          // Minimum phase mode
    int m_scale = 0;                  // Gain scale

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QTextEdit* gainEntryEdit = nullptr;
    QComboBox* presetCombo = nullptr;
    QSlider* delaySlider = nullptr;
    QDoubleSpinBox* delaySpinBox = nullptr;
    QSlider* accuracySlider = nullptr;
    QDoubleSpinBox* accuracySpinBox = nullptr;
    QComboBox* wfuncCombo = nullptr;
    QComboBox* scaleCombo = nullptr;
    QCheckBox* fixedCheck = nullptr;
    QCheckBox* multiCheck = nullptr;
    QCheckBox* zeroPhaseCheck = nullptr;
    QCheckBox* minPhaseCheck = nullptr;
};
