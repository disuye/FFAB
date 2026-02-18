#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFAloop - Wraps FFmpeg's 'aloop' audio filter
 * Loop audio samples
 * FFmpeg filter: aloop=loop:size:start[:time]
 */
class FFAloop : public BaseFilter {
    Q_OBJECT
public:
    FFAloop();
    ~FFAloop() override = default;

    QString displayName() const override { return "Loop"; }
    QString filterType() const override { return "ff-aloop"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    QString formatDuration(double value, const QString& unit) const;

    int m_loop = 0;
    qint64 m_size = 48000;
    qint64 m_start = 0;
    double m_timeValue = 0.0;
    QString m_timeUnit = "s";
    QString ffmpegFlags;
    QCheckBox* loopEntireFileCheckBox = nullptr; // custom helper

    QWidget* parametersWidget = nullptr;
    QSpinBox* loopSpinBox = nullptr;
    QSpinBox* sizeSpinBox = nullptr;
    QSpinBox* startSpinBox = nullptr;
    QDoubleSpinBox* timeSpinBox = nullptr;
    QComboBox* timeUnitCombo = nullptr;
};
