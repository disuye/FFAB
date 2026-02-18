#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;

/**
 * FFAstats - Wraps FFmpeg's 'astats' audio filter
 * Show time domain statistics about audio frames
 * FFmpeg filter: astats=length=<l>:metadata=<m>:reset=<r>
 */
class FFAstats : public BaseFilter {
    Q_OBJECT
public:
    FFAstats();
    ~FFAstats() override = default;

    QString displayName() const override { return "Audio Stats"; }
    QString filterType() const override { return "ff-astats"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    double m_length = 0.05;    // 0 - 10 seconds, window length, default 0.05
    bool m_metadata = false;   // inject metadata, default false
    int m_reset = 0;           // frames before reset (0 = never), default 0

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* lengthSpinBox = nullptr;
    QSlider* lengthSlider = nullptr;
    QCheckBox* metadataCheck = nullptr;
    QSpinBox* resetSpinBox = nullptr;
};
