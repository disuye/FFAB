#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;

/**
 * FFSurround - Wraps FFmpeg's 'surround' audio filter
 * Apply audio surround upmix filter.
 * FFmpeg filter: surround=chl_out=<layout>:chl_in=<layout>:level_in=<l>:...
 * 
 * Upmixes stereo (or other formats) to surround sound formats like 5.1 or 7.1.
 * Uses spectral analysis to extract ambient/directional components and
 * distribute them to surround channels.
 */
class FFSurround : public BaseFilter {
    Q_OBJECT
public:
    FFSurround();
    ~FFSurround() override = default;

    QString displayName() const override { return "Surround Upmix"; }
    QString filterType() const override { return "ff-surround"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_chlOut = "5.1";       // Output channel layout
    QString m_chlIn = "stereo";    // Input channel layout
    double m_levelIn = 1.0;        // Input level (0-10)
    double m_levelOut = 1.0;       // Output level (0-10)
    bool m_lfe = true;             // Output LFE
    int m_lfeLow = 128;            // LFE low cutoff Hz
    int m_lfeHigh = 256;           // LFE high cutoff Hz
    int m_lfeMode = 0;             // LFE mode: 0=add, 1=sub
    double m_smooth = 0.0;         // Temporal smoothness (0-1)
    double m_angle = 90.0;         // Soundfield angle (0-360)
    double m_focus = 0.0;          // Soundfield focus (-1 to 1)
    
    // Channel levels (all default 1.0, range 0-10)
    double m_fcIn = 1.0, m_fcOut = 1.0;
    double m_flIn = 1.0, m_flOut = 1.0;
    double m_frIn = 1.0, m_frOut = 1.0;
    double m_slIn = 1.0, m_slOut = 1.0;
    double m_srIn = 1.0, m_srOut = 1.0;
    double m_blIn = 1.0, m_blOut = 1.0;
    double m_brIn = 1.0, m_brOut = 1.0;
    double m_lfeIn = 1.0, m_lfeOut = 1.0;

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QComboBox* chlOutCombo = nullptr;
    QComboBox* chlInCombo = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QCheckBox* lfeCheck = nullptr;
    QSpinBox* lfeLowSpinBox = nullptr;
    QSpinBox* lfeHighSpinBox = nullptr;
    QComboBox* lfeModeCombo = nullptr;
    QSlider* smoothSlider = nullptr;
    QDoubleSpinBox* smoothSpinBox = nullptr;
    QSlider* angleSlider = nullptr;
    QDoubleSpinBox* angleSpinBox = nullptr;
    QSlider* focusSlider = nullptr;
    QDoubleSpinBox* focusSpinBox = nullptr;
};
