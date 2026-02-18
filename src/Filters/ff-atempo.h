#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QLabel;

/**
 * FFAtempo - Wraps FFmpeg's 'atempo' audio filter
 * Adjusts audio tempo without changing pitch
 * FFmpeg filter: atempo=<tempo>
 * Range: 0.5 to 100 (default: 1.0)
 */
class FFAtempo : public BaseFilter {
    Q_OBJECT
public:
    FFAtempo();
    ~FFAtempo() override = default;

    QString displayName() const override { return "Tempo"; }
    QString filterType() const override { return "ff-atempo"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    double m_tempo = 1.0;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QSlider* tempoSlider = nullptr;
    QDoubleSpinBox* tempoSpinBox = nullptr;
    QLabel* tempoLabel = nullptr;
};
