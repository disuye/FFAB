#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLineEdit;

/**
 * FFAdrc - Wraps FFmpeg's 'adrc' audio filter
 * Audio Spectral Dynamic Range Controller
 * FFmpeg filter: adrc=transfer=<expr>:attack=<a>:release=<r>
 */
class FFAdrc : public BaseFilter {
    Q_OBJECT
public:
    FFAdrc();
    ~FFAdrc() override = default;

    QString displayName() const override { return "DRC (Spectral)"; }
    QString filterType() const override { return "ff-adrc"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_transfer = "p";    // Transfer function expression, default "p" (pass-through)
    double m_attack = 50.0;      // 1 - 1000 ms, default 50
    double m_release = 100.0;    // 5 - 2000 ms, default 100

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QLineEdit* transferEdit = nullptr;
    QDoubleSpinBox* attackSpinBox = nullptr;
    QSlider* attackSlider = nullptr;
    QDoubleSpinBox* releaseSpinBox = nullptr;
    QSlider* releaseSlider = nullptr;
};
