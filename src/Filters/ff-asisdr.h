#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFAsisdr - Wraps FFmpeg's 'asisdr' audio filter
 * Measure Audio Scale-Invariant Signal-to-Distortion Ratio.
 * FFmpeg filter: asisdr
 * 
 * This is a 2-input analysis filter that compares a reference signal
 * (first input) against a test signal (second input) using scale-invariant
 * SDR measurement - compensates for gain differences between signals.
 * 
 * Requires 2 audio inputs of the same format/length.
 * Outputs first input unchanged while measuring.
 */
class FFAsisdr : public BaseFilter {
    Q_OBJECT
public:
    FFAsisdr();
    ~FFAsisdr() override = default;

    QString displayName() const override { return "SI-SDR Measure"; }
    QString filterType() const override { return "ff-asisdr"; }
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
