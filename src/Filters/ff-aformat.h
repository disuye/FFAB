#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QComboBox;
class QCheckBox;
class QSpinBox;

/**
 * FFAformat - Wraps FFmpeg's 'aformat' audio filter
 * Convert the input audio to one of the specified formats
 * FFmpeg filter: aformat=sample_rates=<rate>
 */
class FFAformat : public BaseFilter {
    Q_OBJECT
public:
    FFAformat();
    ~FFAformat() override = default;

    QString displayName() const override { return "Format"; }
    QString filterType() const override { return "ff-aformat"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    bool m_useCustomRate = false;
    int m_presetRate = 44100;
    int m_customRate = 44100;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QComboBox* presetCombo = nullptr;
    QCheckBox* customCheckBox = nullptr;
    QSpinBox* customSpinBox = nullptr;
};
