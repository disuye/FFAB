#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QComboBox;
class QLabel;

/**
 * FFAmerge - Wraps FFmpeg's 'amerge' audio filter
 * Merge two or more audio streams into a single multi-channel stream
 * 
 * This is a MULTI-INPUT filter that requires:
 * - Input [0:a]: First audio stream
 * - Input [1:a]: Second audio stream (from AudioInput filter)
 * - Optional [2:a], [3:a], etc. for more channels
 * 
 * Use cases:
 * - Combine mono files into stereo
 * - Create multichannel outputs (5.1, 7.1, etc.)
 * - Merge separated instrument tracks
 * 
 * FFmpeg filter: [0:a][1:a]amerge=inputs=2
 */
class FFAmerge : public BaseFilter {
    Q_OBJECT
public:
    FFAmerge();
    ~FFAmerge() override = default;

    QString displayName() const override { return "Channel Merge (amerge))"; }
    QString filterType() const override { return "ff-amerge"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    void updateInfoLabel();

    // Parameters
    int inputCount = 2;         // Number of inputs to merge (2-64)

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* inputCountSpinBox = nullptr;
    QComboBox* presetCombo = nullptr;
    QLabel* infoLabel = nullptr;
    QLabel* channelLayoutLabel = nullptr;
};
