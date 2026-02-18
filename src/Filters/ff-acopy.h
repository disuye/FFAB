#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLabel;

/**
 * FFAcopy - Wraps FFmpeg's 'acopy' audio filter
 * Copy the input audio unchanged to the output.
 * FFmpeg filter: acopy
 * 
 * This is a pass-through filter with no parameters.
 * Useful for forcing a filter graph node without modification.
 */
class FFAcopy : public BaseFilter {
    Q_OBJECT
public:
    FFAcopy();
    ~FFAcopy() override = default;

    QString displayName() const override { return "Audio Copy"; }
    QString filterType() const override { return "ff-acopy"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
};
