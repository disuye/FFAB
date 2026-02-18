#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QComboBox;
class QLineEdit;
class QLabel;

/**
 * FFJoin - Wraps FFmpeg's 'join' audio filter
 * Join multiple audio streams into multi-channel output.
 * FFmpeg filter: join=inputs=<n>:channel_layout=<layout>:map=<mapping>
 * 
 * This is a multi-input filter that combines separate mono or stereo
 * streams into a single multi-channel stream (e.g., combining 6 mono
 * inputs into a 5.1 surround stream).
 * 
 * Requires multiple audio inputs.
 */
class FFJoin : public BaseFilter {
    Q_OBJECT
public:
    FFJoin();
    ~FFJoin() override = default;

    QString displayName() const override { return "Join Channels"; }
    QString filterType() const override { return "ff-join"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    int m_inputs = 2;                    // Number of inputs (1+), default 2
    QString m_channelLayout = "stereo"; // Output channel layout
    QString m_map = "";                  // Optional explicit channel mapping

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* inputsSpinBox = nullptr;
    QComboBox* layoutCombo = nullptr;
    QLineEdit* mapEdit = nullptr;
};
