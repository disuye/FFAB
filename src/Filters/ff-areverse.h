#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

/**
 * FFAreverse - Wraps FFmpeg's 'areverse' audio filter
 * Reverses an audio clip
 * FFmpeg filter: areverse
 */
class FFAreverse : public BaseFilter {
    Q_OBJECT
public:
    FFAreverse();
    ~FFAreverse() override = default;

    QString displayName() const override { return "Reverse"; }
    QString filterType() const override { return "ff-areverse"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString ffmpegFlags;
    QWidget* parametersWidget = nullptr;
};
