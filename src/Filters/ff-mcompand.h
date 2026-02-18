#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLineEdit;

/**
 * FFMcompand - Wraps FFmpeg's 'mcompand' audio filter
 * Multiband Compress or expand audio dynamic range
 * FFmpeg filter: mcompand=args=<complex_string>
 */
class FFMcompand : public BaseFilter {
    Q_OBJECT
public:
    FFMcompand();
    ~FFMcompand() override = default;

    QString displayName() const override { return "Multiband Compand"; }
    QString filterType() const override { return "ff-mcompand"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    QString m_args = "0.005,0.1 6 -47/-40,-34/-34,-17/-33 100 | 0.003,0.05 6 -47/-40,-34/-34,-17/-33 400 | 0.000625,0.0125 6 -47/-40,-34/-34,-15/-33 1600 | 0.0001,0.025 6 -47/-40,-34/-34,-31/-31,-0/-30 6400 | 0,0.025 6 -38/-31,-28/-28,-0/-25 22000";

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QLineEdit* argsEdit = nullptr;
};
