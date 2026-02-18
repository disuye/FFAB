#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFAgate - Wraps FFmpeg's 'agate' audio filter
 * Audio gate
 * FFmpeg filter: agate=level_in=<l>:mode=<m>:range=<r>:threshold=<t>:ratio=<r>:attack=<a>:release=<r>:makeup=<m>:knee=<k>:detection=<d>:link=<l>:level_sc=<l>
 */
class FFAgate : public BaseFilter {
    Q_OBJECT
public:
    FFAgate();
    ~FFAgate() override = default;

    QString displayName() const override { return "Gate"; }
    QString filterType() const override { return "ff-agate"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;      // 0.015625 - 64, default 1
    int m_mode = 0;              // 0=downward, 1=upward, default 0
    double m_range = 0.06125;    // 0 - 1, default 0.06125
    double m_threshold = 0.125;  // 0 - 1, default 0.125
    double m_ratio = 2.0;        // 1 - 9000, default 2
    double m_attack = 20.0;      // 0.01 - 9000 ms, default 20
    double m_release = 250.0;    // 0.01 - 9000 ms, default 250
    double m_makeup = 1.0;       // 1 - 64, default 1
    double m_knee = 2.82843;     // 1 - 8, default 2.82843
    int m_detection = 1;         // 0=peak, 1=rms, default 1
    int m_link = 0;              // 0=average, 1=maximum, default 0
    double m_levelSc = 1.0;      // 0.015625 - 64, default 1

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QComboBox* modeCombo = nullptr;
    QSlider* rangeSlider = nullptr;
    QDoubleSpinBox* rangeSpinBox = nullptr;
    QSlider* thresholdSlider = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QSlider* ratioSlider = nullptr;
    QDoubleSpinBox* ratioSpinBox = nullptr;
    QSlider* attackSlider = nullptr;
    QDoubleSpinBox* attackSpinBox = nullptr;
    QSlider* releaseSlider = nullptr;
    QDoubleSpinBox* releaseSpinBox = nullptr;
    QSlider* makeupSlider = nullptr;
    QDoubleSpinBox* makeupSpinBox = nullptr;
    QSlider* kneeSlider = nullptr;
    QDoubleSpinBox* kneeSpinBox = nullptr;
    QComboBox* detectionCombo = nullptr;
    QComboBox* linkCombo = nullptr;
    QSlider* levelScSlider = nullptr;
    QDoubleSpinBox* levelScSpinBox = nullptr;
};
