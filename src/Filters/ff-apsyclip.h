#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFApsyclip - Wraps FFmpeg's 'apsyclip' audio filter
 * Audio Psychoacoustic Clipper
 * FFmpeg filter: apsyclip=level_in=<l>:level_out=<l>:clip=<c>:diff=<d>:adaptive=<a>:iterations=<i>:level=<l>
 */
class FFApsyclip : public BaseFilter {
    Q_OBJECT
public:
    FFApsyclip();
    ~FFApsyclip() override = default;

    QString displayName() const override { return "Psychoacoustic Clipper"; }
    QString filterType() const override { return "ff-apsyclip"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;      // 0.015625 - 64, default 1
    double m_levelOut = 1.0;     // 0.015625 - 64, default 1
    double m_clip = 1.0;         // 0.015625 - 1, default 1
    bool m_diff = false;         // default false
    double m_adaptive = 0.5;     // 0 - 1, default 0.5
    int m_iterations = 10;       // 1 - 20, default 10
    bool m_level = false;        // auto level, default false

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* clipSlider = nullptr;
    QDoubleSpinBox* clipSpinBox = nullptr;
    QCheckBox* diffCheckBox = nullptr;
    QSlider* adaptiveSlider = nullptr;
    QDoubleSpinBox* adaptiveSpinBox = nullptr;
    QSlider* iterationsSlider = nullptr;
    QDoubleSpinBox* iterationsSpinBox = nullptr;
    QCheckBox* levelCheckBox = nullptr;
};
