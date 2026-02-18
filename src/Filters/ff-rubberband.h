#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QLabel;

/**
 * FFRubberband - Wraps FFmpeg's 'rubberband' audio filter
 * Apply time-stretching and pitch-shifting.
 * FFmpeg filter: rubberband=tempo=<t>:pitch=<p>:transients=<tr>:...
 * 
 * High-quality time-stretching and pitch-shifting using the
 * Rubber Band library. Can change tempo without affecting pitch
 * or change pitch without affecting tempo.
 */
class FFRubberband : public BaseFilter {
    Q_OBJECT
public:
    FFRubberband();
    ~FFRubberband() override = default;

    QString displayName() const override { return "Rubberband"; }
    QString filterType() const override { return "ff-rubberband"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    double semitonesToPitchFactor(double semitones) const;
    double pitchFactorToSemitones(double factor) const;

    // Parameters
    double m_tempo = 1.0;        // Tempo scale: 0.01-100, default 1
    double m_pitch = 1.0;        // Pitch scale: 0.01-100, default 1
    int m_transients = 0;        // 0=crisp, 1=mixed, 2=smooth
    int m_detector = 0;          // 0=compound, 1=percussive, 2=soft
    int m_phase = 0;             // 0=laminar, 1=independent
    int m_window = 0;            // 0=standard, 1=short, 2=long
    int m_smoothing = 0;         // 0=off, 1=on
    int m_formant = 0;           // 0=shifted, 1=preserved
    int m_pitchq = 0;            // 0=quality, 1=speed, 2=consistency
    int m_channels = 0;          // 0=apart, 1=together

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* tempoSlider = nullptr;
    QDoubleSpinBox* tempoSpinBox = nullptr;
    QSlider* pitchSlider = nullptr;
    QDoubleSpinBox* pitchSpinBox = nullptr;
    QDoubleSpinBox* semitonesSpinBox = nullptr;
    QComboBox* transientsCombo = nullptr;
    QComboBox* detectorCombo = nullptr;
    QComboBox* phaseCombo = nullptr;
    QComboBox* windowCombo = nullptr;
    QComboBox* smoothingCombo = nullptr;
    QComboBox* formantCombo = nullptr;
    QComboBox* pitchqCombo = nullptr;
    QComboBox* channelsCombo = nullptr;
};
