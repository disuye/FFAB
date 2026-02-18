#include "ff-anull.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>

FFAnull::FFAnull() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAnull::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Null (Pass-through)"));
    layout->addWidget(new QLabel("Passes audio unchanged. Useful for testing."));
    
    layout->addStretch();
    return parametersWidget;
}

void FFAnull::updateFFmpegFlags() {
    ffmpegFlags = "anull";
}

QString FFAnull::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAnull::toJSON(QJsonObject& json) const {
    json["type"] = "ff-anull";
}

void FFAnull::fromJSON(const QJsonObject& json) {
    updateFFmpegFlags();
}
