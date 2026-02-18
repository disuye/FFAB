#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFApsnr - Wraps FFmpeg's 'apsnr' audio filter
 * Measure Audio Peak Signal-to-Noise Ratio.
 * FFmpeg filter: apsnr
 * 
 * This is a 2-input analysis filter that compares a reference signal
 * (first input) against a potentially distorted signal (second input)
 * and measures the peak SNR between them.
 * 
 * Requires 2 audio inputs of the same format/length.
 * Outputs first input unchanged while measuring.
 */
class FFApsnr : public BaseFilter {
    Q_OBJECT
public:
    FFApsnr();
    ~FFApsnr() override = default;

    QString displayName() const override { return "PSNR Measure"; }
    QString filterType() const override { return "ff-apsnr"; }
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
