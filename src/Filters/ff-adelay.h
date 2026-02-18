#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QLineEdit;
class QCheckBox;

/**
 * FFAdelay - Wraps FFmpeg's 'adelay' audio filter
 * Delay one or more audio channels
 * FFmpeg filter: adelay=delays=<list>[:all=<bool>]
 * Example: delays=1000|1500|2000 (milliseconds, pipe-separated)
 */
class FFAdelay : public BaseFilter {
    Q_OBJECT
public:
    FFAdelay();
    ~FFAdelay() override = default;

    QString displayName() const override { return "Delay"; }
    QString filterType() const override { return "ff-adelay"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    QString m_delays = "400|800|1200";
    bool m_all = false;
    QString ffmpegFlags;

    QWidget* parametersWidget = nullptr;
    QLineEdit* delaysEdit = nullptr;
    QCheckBox* allCheckBox = nullptr;
};
