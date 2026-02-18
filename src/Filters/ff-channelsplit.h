#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QComboBox;
class QLineEdit;

/**
 * FFChannelsplit - Wraps FFmpeg's 'channelsplit' audio filter
 * Split audio into separate per-channel streams
 * FFmpeg filter: channelsplit=channel_layout=<layout>:channels=<ch>
 * 
 * NOTE: This filter produces multiple outputs. In a typical filter chain,
 * you may need to select which output to use downstream.
 */
class FFChannelsplit : public BaseFilter {
    Q_OBJECT
public:
    FFChannelsplit();
    ~FFChannelsplit() override = default;

    QString displayName() const override { return "Channel Split"; }
    QString filterType() const override { return "ff-channelsplit"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_channelLayout = "stereo";  // input channel layout
    QString m_channels = "all";          // which channels to extract

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QComboBox* layoutCombo = nullptr;
    QLineEdit* channelsEdit = nullptr;
};
