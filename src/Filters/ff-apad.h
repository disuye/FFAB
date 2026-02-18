#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QRadioButton;

/**
 * FFApad - Wraps FFmpeg's 'apad' audio filter
 * Pad audio with silence
 * Two modes: Add Silence OR Target Total Length (mutually exclusive)
 */
class FFApad : public BaseFilter {
    Q_OBJECT
public:
    enum class PadMode {
        AddSilence,      // Use pad_len or pad_dur
        TargetLength     // Use whole_len or whole_dur
    };

    FFApad();
    ~FFApad() override = default;

    QString displayName() const override { return "Pad"; }
    QString filterType() const override { return "ff-apad"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    QString formatDuration(double value, const QString& unit) const;

    int m_packetSize = 4096;
    PadMode m_mode = PadMode::AddSilence;
    double m_value = 1.0;
    QString m_unit = "s";  // "s", "ms", or "samples"
    
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QSpinBox* packetSizeSpinBox = nullptr;
    QRadioButton* addSilenceRadio = nullptr;
    QRadioButton* targetLengthRadio = nullptr;
    QDoubleSpinBox* valueSpinBox = nullptr;
    QComboBox* unitCombo = nullptr;
};