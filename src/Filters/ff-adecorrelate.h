#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QSpinBox;

/**
 * FFAdecorrelate - Wraps FFmpeg's 'adecorrelate' audio filter
 * Apply decorrelation to stereo audio for enhanced width
 * FFmpeg filter: adecorrelate=stages=<s>:seed=<seed>
 */
class FFAdecorrelate : public BaseFilter {
    Q_OBJECT
public:
    FFAdecorrelate();
    ~FFAdecorrelate() override = default;

    QString displayName() const override { return "Decorrelate"; }
    QString filterType() const override { return "ff-adecorrelate"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    int m_stages = 6;   // 1 - 16, filtering stages, default 6
    int m_seed = -1;    // -1 = random, random seed

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* stagesSpinBox = nullptr;
    QSlider* stagesSlider = nullptr;
    QSpinBox* seedSpinBox = nullptr;
};
