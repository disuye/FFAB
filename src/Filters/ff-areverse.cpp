#include "ff-areverse.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>

FFAreverse::FFAreverse() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAreverse::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Reverse"));
    layout->addWidget(new QLabel("Reverses the entire audio clip."));
    
    layout->addStretch();
    return parametersWidget;
}

void FFAreverse::updateFFmpegFlags() {
    ffmpegFlags = "areverse";
}

QString FFAreverse::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAreverse::toJSON(QJsonObject& json) const {
    json["type"] = "ff-areverse";
}

void FFAreverse::fromJSON(const QJsonObject& json) {
    updateFFmpegFlags();
}
