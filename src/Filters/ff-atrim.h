#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QDoubleSpinBox;
class QComboBox;

/**
 * FFAtrim - Wraps FFmpeg's 'atrim' audio filter
 * Pick one continuous section from the input, drop the rest
 * FFmpeg filter: atrim=start:end:duration
 */
class FFAtrim : public BaseFilter {
    Q_OBJECT
public:
    FFAtrim();
    ~FFAtrim() override = default;

    QString displayName() const override { return "Trim"; }
    QString filterType() const override { return "ff-atrim"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    QString formatDuration(double value, const QString& unit) const;

    double m_startValue = 0.0;
    QString m_startUnit = "s";
    double m_endValue = 0.0;
    QString m_endUnit = "s";
    double m_durationValue = 0.0;
    QString m_durationUnit = "s";
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* startSpinBox = nullptr;
    QComboBox* startUnitCombo = nullptr;
    QDoubleSpinBox* endSpinBox = nullptr;
    QComboBox* endUnitCombo = nullptr;
    QDoubleSpinBox* durationSpinBox = nullptr;
    QComboBox* durationUnitCombo = nullptr;
};
