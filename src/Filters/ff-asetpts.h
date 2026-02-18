#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLineEdit;

/**
 * FFAsetpts - Wraps FFmpeg's 'asetpts' audio filter
 * Set PTS for the output audio frame
 * FFmpeg filter: asetpts=<expr>
 * Default: "PTS" (no change)
 * Common expressions: PTS, PTS-STARTPTS, PTS+0.1/TB
 */
class FFAsetpts : public BaseFilter {
    Q_OBJECT
public:
    FFAsetpts();
    ~FFAsetpts() override = default;

    QString displayName() const override { return "Set PTS"; }
    QString filterType() const override { return "ff-asetpts"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString m_expr = "PTS";
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QLineEdit* exprEdit = nullptr;
};
