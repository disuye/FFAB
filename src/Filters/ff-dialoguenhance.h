#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;

/**
 * FFDialoguenhance - Wraps FFmpeg's 'dialoguenhance' audio filter
 * Enhances dialogue/voice clarity in stereo audio
 * FFmpeg filter: dialoguenhance=original=<o>:enhance=<e>:voice=<v>
 */
class FFDialoguenhance : public BaseFilter {
    Q_OBJECT
public:
    FFDialoguenhance();
    ~FFDialoguenhance() override = default;

    QString displayName() const override { return "Dialogue Enhance"; }
    QString filterType() const override { return "ff-dialoguenhance"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_original = 1.0;   // 0 - 1, original center factor, default 1
    double m_enhance = 1.0;    // 0 - 3, dialogue enhance factor, default 1
    double m_voice = 2.0;      // 2 - 32, voice detection factor, default 2

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* originalSpinBox = nullptr;
    QSlider* originalSlider = nullptr;
    QDoubleSpinBox* enhanceSpinBox = nullptr;
    QSlider* enhanceSlider = nullptr;
    QDoubleSpinBox* voiceSpinBox = nullptr;
    QSlider* voiceSlider = nullptr;
};
