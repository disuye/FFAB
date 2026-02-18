#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class InputFilter : public BaseFilter {
    Q_OBJECT
public:
    InputFilter();
    ~InputFilter() override = default;

    QString displayName() const override { return "INPUT"; }
    QString filterType() const override { return "input"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    QWidget* parametersWidget = nullptr;
};
