#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLineEdit;
class QComboBox;

/**
 * FFChannelmap - Wraps FFmpeg's 'channelmap' audio filter
 * Remap audio channels (reorder, duplicate, drop channels)
 * FFmpeg filter: channelmap=map=<map>:channel_layout=<layout>
 */
class FFChannelmap : public BaseFilter {
    Q_OBJECT
public:
    FFChannelmap();
    ~FFChannelmap() override = default;

    QString displayName() const override { return "Channel Map"; }
    QString filterType() const override { return "ff-channelmap"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    void applyPreset(int index);

    // Parameters
    QString m_map = "0|1";              // channel mapping string
    QString m_channelLayout = "stereo"; // output channel layout

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QLineEdit* mapEdit = nullptr;
    QComboBox* layoutCombo = nullptr;
    QComboBox* presetCombo = nullptr;
};
