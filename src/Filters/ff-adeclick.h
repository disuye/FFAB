#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFAdeclick - Wraps FFmpeg's 'adeclick' audio filter
 * Remove impulsive noise (clicks, pops) from input audio
 * FFmpeg filter: adeclick=window=<w>:overlap=<o>:arorder=<a>:threshold=<t>:burst=<b>:method=<m>
 */
class FFAdeclick : public BaseFilter {
    Q_OBJECT
public:
    FFAdeclick();
    ~FFAdeclick() override = default;

    QString displayName() const override { return "De-Click"; }
    QString filterType() const override { return "ff-adeclick"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_window = 55.0;      // 10 - 100 ms, default 55
    double m_overlap = 75.0;     // 50 - 95 %, default 75
    double m_arorder = 2.0;      // 0 - 25, default 2
    double m_threshold = 2.0;    // 1 - 100, default 2
    double m_burst = 2.0;        // 0 - 10, default 2
    int m_method = 0;            // 0=add, 1=a, default add

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* windowSlider = nullptr;
    QDoubleSpinBox* windowSpinBox = nullptr;
    QSlider* overlapSlider = nullptr;
    QDoubleSpinBox* overlapSpinBox = nullptr;
    QSlider* arorderSlider = nullptr;
    QDoubleSpinBox* arorderSpinBox = nullptr;
    QSlider* thresholdSlider = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QSlider* burstSlider = nullptr;
    QDoubleSpinBox* burstSpinBox = nullptr;
    QComboBox* methodCombo = nullptr;
};
