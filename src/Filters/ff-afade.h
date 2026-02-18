#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;

/**
 * FFAfade - Wraps FFmpeg's 'afade' audio filter
 * Fade in/out input audio
 * FFmpeg filter: afade=t=<type>:st=<start>:d=<duration>:c=<curve>
 */
class FFAfade : public BaseFilter {
    Q_OBJECT
public:
    FFAfade();
    ~FFAfade() override = default;

    QString displayName() const override { return "Fade"; }
    QString filterType() const override { return "ff-afade"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    QString formatDuration(double value, const QString& unit) const;

    int m_type = 0; // 0=in, 1=out
    double m_startTimeValue = 0.0;
    QString m_startTimeUnit = "s";
    double m_durationValue = 0.0;
    QString m_durationUnit = "s";
    int m_curve = 0; // tri
    double m_silence = 0.0;
    double m_unity = 1.0;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QComboBox* typeCombo = nullptr;
    QDoubleSpinBox* startTimeSpinBox = nullptr;
    QComboBox* startTimeUnitCombo = nullptr;
    QDoubleSpinBox* durationSpinBox = nullptr;
    QComboBox* durationUnitCombo = nullptr;
    QComboBox* curveCombo = nullptr;
    QDoubleSpinBox* silenceSpinBox = nullptr;
    QDoubleSpinBox* unitySpinBox = nullptr;
};
