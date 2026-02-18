#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFAlimiter - Wraps FFmpeg's 'alimiter' audio filter
 * Audio lookahead limiter
 * FFmpeg filter: alimiter=level_in=<l>:level_out=<l>:limit=<l>:attack=<a>:release=<r>:asc=<a>:asc_level=<a>:level=<l>:latency=<l>
 */
class FFAlimiter : public BaseFilter {
    Q_OBJECT
public:
    FFAlimiter();
    ~FFAlimiter() override = default;

    QString displayName() const override { return "Limiter"; }
    QString filterType() const override { return "ff-alimiter"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_levelIn = 1.0;      // 0.015625 - 64, default 1
    double m_levelOut = 1.0;     // 0.015625 - 64, default 1
    double m_limit = 1.0;        // 0.0625 - 1, default 1
    double m_attack = 5.0;       // 0.1 - 80 ms, default 5
    double m_release = 50.0;     // 1 - 8000 ms, default 50
    bool m_asc = false;          // Auto sidechain, default false
    double m_ascLevel = 0.5;     // 0 - 1, default 0.5
    bool m_level = true;         // Auto level, default true
    bool m_latency = false;      // Compensate delay, default false

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* limitSlider = nullptr;
    QDoubleSpinBox* limitSpinBox = nullptr;
    QSlider* attackSlider = nullptr;
    QDoubleSpinBox* attackSpinBox = nullptr;
    QSlider* releaseSlider = nullptr;
    QDoubleSpinBox* releaseSpinBox = nullptr;
    QCheckBox* ascCheckBox = nullptr;
    QSlider* ascLevelSlider = nullptr;
    QDoubleSpinBox* ascLevelSpinBox = nullptr;
    QCheckBox* levelCheckBox = nullptr;
    QCheckBox* latencyCheckBox = nullptr;
};
