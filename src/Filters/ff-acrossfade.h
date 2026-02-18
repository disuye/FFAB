#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QCheckBox;

/**
 * FFAcrossfade - Wraps FFmpeg's 'acrossfade' audio filter
 * Cross fade two input audio streams
 * FFmpeg filter: acrossfade=d=<duration>:o=<overlap>:c1=<curve1>:c2=<curve2>
 * Note: This filter requires two inputs
 */
class FFAcrossfade : public BaseFilter {
    Q_OBJECT
public:
    FFAcrossfade();
    ~FFAcrossfade() override = default;

    QString displayName() const override { return "Crossfade (acrossfade)"; }
    QString filterType() const override { return "ff-acrossfade"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    // Multi-input filter support
    void setSidechainInputIndex(int index) { m_sidechainInputIndex = index; }
    int getSidechainInputIndex() const { return m_sidechainInputIndex; }

private:
    void updateFFmpegFlags();
    QString formatDuration(double value, const QString& unit) const;

    double m_durationValue = 0.0;
    QString m_durationUnit = "s";
    bool m_overlap = true;
    int m_curve1 = 0; // tri
    int m_curve2 = 0; // tri
    int m_sidechainInputIndex = -1;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* durationSpinBox = nullptr;
    QComboBox* durationUnitCombo = nullptr;
    QCheckBox* overlapCheckBox = nullptr;
    QComboBox* curve1Combo = nullptr;
    QComboBox* curve2Combo = nullptr;
};
