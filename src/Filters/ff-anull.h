#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

/**
 * FFAnull - Wraps FFmpeg's 'anull' audio filter
 * Passes the source unchanged to the output
 * FFmpeg filter: anull
 */
class FFAnull : public BaseFilter {
    Q_OBJECT
public:
    FFAnull();
    ~FFAnull() override = default;

    QString displayName() const override { return "Null (Pass-through)"; }
    QString filterType() const override { return "ff-anull"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString ffmpegFlags;
    QWidget* parametersWidget = nullptr;
};
