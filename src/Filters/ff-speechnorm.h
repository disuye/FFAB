#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFSpeechnorm - Wraps FFmpeg's 'speechnorm' audio filter
 * Speech-specific normalization for podcasts and voice recordings
 * FFmpeg filter: speechnorm=peak=<p>:expansion=<e>:compression=<c>:threshold=<t>:raise=<r>:fall=<f>:invert=<i>
 */
class FFSpeechnorm : public BaseFilter {
    Q_OBJECT
public:
    FFSpeechnorm();
    ~FFSpeechnorm() override = default;

    QString displayName() const override { return "Speech Norm"; }
    QString filterType() const override { return "ff-speechnorm"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_peak = 0.95;        // 0 - 1, target peak level, default 0.95
    double m_expansion = 2.0;    // 1 - 50, max expansion factor, default 2
    double m_compression = 2.0;  // 1 - 50, max compression factor, default 2
    double m_threshold = 0.0;    // 0 - 1, silence threshold, default 0
    double m_raise = 0.001;      // 0 - 1, expansion rate, default 0.001
    double m_fall = 0.001;       // 0 - 1, compression rate, default 0.001
    bool m_invert = false;       // invert filtering, default false

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* peakSpinBox = nullptr;
    QSlider* peakSlider = nullptr;
    QDoubleSpinBox* expansionSpinBox = nullptr;
    QSlider* expansionSlider = nullptr;
    QDoubleSpinBox* compressionSpinBox = nullptr;
    QSlider* compressionSlider = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QSlider* thresholdSlider = nullptr;
    QDoubleSpinBox* raiseSpinBox = nullptr;
    QSlider* raiseSlider = nullptr;
    QDoubleSpinBox* fallSpinBox = nullptr;
    QSlider* fallSlider = nullptr;
    QCheckBox* invertCheck = nullptr;
};
