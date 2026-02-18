#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;

/**
 * FFAdeclip - Wraps FFmpeg's 'adeclip' audio filter
 * Removes clipping distortion from audio
 * FFmpeg filter: adeclip=window=<w>:overlap=<o>:arorder=<a>:threshold=<t>:hsize=<n>:method=<m>
 */
class FFAdeclip : public BaseFilter {
    Q_OBJECT
public:
    FFAdeclip();
    ~FFAdeclip() override = default;

    QString displayName() const override { return "Declip"; }
    QString filterType() const override { return "ff-adeclip"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_window = 55.0;      // 10 - 100 ms, default 55
    double m_overlap = 75.0;     // 50 - 95 %, default 75
    double m_arorder = 8.0;      // 0 - 25, default 8
    double m_threshold = 10.0;   // 1 - 100, default 10
    int m_hsize = 1000;          // 100 - 9999, default 1000
    int m_method = 0;            // 0=add, 1=save, default 0

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* windowSpinBox = nullptr;
    QSlider* windowSlider = nullptr;
    QDoubleSpinBox* overlapSpinBox = nullptr;
    QSlider* overlapSlider = nullptr;
    QDoubleSpinBox* arorderSpinBox = nullptr;
    QSlider* arorderSlider = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QSlider* thresholdSlider = nullptr;
    QSpinBox* hsizeSpinBox = nullptr;
    QSlider* hsizeSlider = nullptr;
    QComboBox* methodCombo = nullptr;
};
