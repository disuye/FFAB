#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFAsdr - Wraps FFmpeg's 'asdr' audio filter
 * Measure Audio Signal-to-Distortion Ratio.
 * FFmpeg filter: asdr
 * 
 * This is a 2-input analysis filter that compares a reference signal
 * (first input) against a potentially distorted signal (second input)
 * and measures the signal-to-distortion ratio.
 * 
 * Requires 2 audio inputs of the same format/length.
 * Outputs first input unchanged while measuring.
 */
class FFAsdr : public BaseFilter {
    Q_OBJECT
public:
    FFAsdr();
    ~FFAsdr() override = default;

    QString displayName() const override { return "SDR Measure"; }
    QString filterType() const override { return "ff-asdr"; }
    bool isAnalysisTwoInputFilter() const override { return true; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
};
