#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QSpinBox;
class QComboBox;
class QLabel;

/**
 * FFAxcorrelate - Wraps FFmpeg's 'axcorrelate' audio filter
 * Cross-correlate two audio streams.
 * FFmpeg filter: axcorrelate=size=<s>:algo=<a>
 * 
 * This is a 2-input filter that computes the cross-correlation between
 * two audio streams. Useful for finding time delays, measuring similarity,
 * or detecting patterns.
 * 
 * Requires 2 audio inputs of the same format.
 */
class FFAxcorrelate : public BaseFilter {
    Q_OBJECT
public:
    FFAxcorrelate();
    ~FFAxcorrelate() override = default;

    QString displayName() const override { return "Cross-Correlate"; }
    QString filterType() const override { return "ff-axcorrelate"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    int m_size = 256;   // Segment size: 2-131072, default 256
    int m_algo = 2;     // Algorithm: 0=slow, 1=fast, 2=best, default best

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* sizeSlider = nullptr;
    QSpinBox* sizeSpinBox = nullptr;
    QComboBox* algoCombo = nullptr;
};
