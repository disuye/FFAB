#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QLabel;

/**
 * FFSofalizer - Wraps FFmpeg's 'sofalizer' audio filter
 * SOFAlizer - Spatially Oriented Format for Acoustics.
 * FFmpeg filter: sofalizer=sofa=<file>:gain=<g>:rotation=<r>:elevation=<e>:...
 * 
 * Applies HRTF (Head-Related Transfer Function) spatialization using
 * SOFA format impulse response files. Creates binaural 3D audio for headphones.
 * 
 * Requires a SOFA (.sofa) file containing HRTF measurements.
 */
class FFSofalizer : public BaseFilter {
    Q_OBJECT
public:
    FFSofalizer();
    ~FFSofalizer() override = default;

    QString displayName() const override { return "SOFA Spatializer"; }
    QString filterType() const override { return "ff-sofalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_sofa = "";            // SOFA file path (required)
    double m_gain = 0.0;            // Gain in dB (-20 to 40)
    double m_rotation = 0.0;        // Rotation (-360 to 360)
    double m_elevation = 0.0;       // Elevation (-90 to 90)
    double m_radius = 1.0;          // Radius (0 to 5)
    int m_type = 0;                 // Processing type: 0=freq, 1=time
    double m_lfeGain = 0.0;         // LFE gain in dB (-20 to 40)
    int m_framesize = 1024;         // Frame size (1024-96000)
    bool m_normalize = true;        // Normalize IRs
    bool m_interpolate = false;     // Interpolate IRs
    bool m_minphase = false;        // Minimum phase IRs
    double m_anglestep = 0.5;       // Neighbor search angle step (0.01-10)

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QLineEdit* sofaEdit = nullptr;
    QSlider* gainSlider = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QSlider* rotationSlider = nullptr;
    QDoubleSpinBox* rotationSpinBox = nullptr;
    QSlider* elevationSlider = nullptr;
    QDoubleSpinBox* elevationSpinBox = nullptr;
    QSlider* radiusSlider = nullptr;
    QDoubleSpinBox* radiusSpinBox = nullptr;
    QComboBox* typeCombo = nullptr;
    QSlider* lfeGainSlider = nullptr;
    QDoubleSpinBox* lfeGainSpinBox = nullptr;
    QSpinBox* framesizeSpinBox = nullptr;
    QCheckBox* normalizeCheck = nullptr;
    QCheckBox* interpolateCheck = nullptr;
    QCheckBox* minphaseCheck = nullptr;
};
