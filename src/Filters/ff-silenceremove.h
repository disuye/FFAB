#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;

/**
 * FFSilenceremove - Wraps FFmpeg's 'silenceremove' audio filter
 * Remove silence from audio
 * FFmpeg filter: silenceremove with multiple parameters
 */
class FFSilenceremove : public BaseFilter {
    Q_OBJECT
public:
    FFSilenceremove();
    ~FFSilenceremove() override = default;

    QString displayName() const override { return "Silence Remove"; }
    QString filterType() const override { return "ff-silenceremove"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    QString formatDuration(double value, const QString& unit) const;

    int m_startPeriods = 0;
    double m_startDurationValue = 0.0;
    QString m_startDurationUnit = "s";
    double m_startThreshold = 0.0;
    double m_startSilenceValue = 0.0;
    QString m_startSilenceUnit = "s";
    int m_startMode = 0; // 0=any, 1=all
    
    int m_stopPeriods = 0;
    double m_stopDurationValue = 0.0;
    QString m_stopDurationUnit = "s";
    double m_stopThreshold = 0.0;
    double m_stopSilenceValue = 0.0;
    QString m_stopSilenceUnit = "s";
    int m_stopMode = 1; // 0=any, 1=all
    
    int m_detection = 1; // 0=avg, 1=rms, 2=peak, 3=median, 4=ptp, 5=dev
    double m_windowValue = 0.02;
    QString m_windowUnit = "s";
    int m_timestamp = 0; // 0=write, 1=copy
    
    QString ffmpegFlags;
    QWidget* parametersWidget = nullptr;
};
