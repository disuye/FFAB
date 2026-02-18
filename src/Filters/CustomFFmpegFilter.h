#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QPlainTextEdit;

class CustomFFmpegFilter : public BaseFilter {
    Q_OBJECT

public:
    CustomFFmpegFilter();
    explicit CustomFFmpegFilter(BaseFilter::Position slot);
    ~CustomFFmpegFilter() override = default;

    QString displayName() const override { return "Custom FFmpeg"; }
    QString filterType() const override { return "custom"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    void setCustomCommand(const QString& cmd) { customCommand = cmd; }
    QString getCustomCommand() const { return customCommand; }
    
    void setManualOutputLabels(bool manual) { manualOutputLabels = manual; }
    bool getManualOutputLabels() const { return manualOutputLabels; }

private:
    QString customCommand;
    QString getTemplateForPosition() const;
    bool manualOutputLabels = false;  // If true, skip automatic output label append/replace

    QWidget* parametersWidget = nullptr;
    QPlainTextEdit* textEdit = nullptr;
};
