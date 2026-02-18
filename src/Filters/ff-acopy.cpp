#include "ff-acopy.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAcopy::FFAcopy() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcopy::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Audio Copy");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Passes audio through unchanged.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto hintLabel = new QLabel("This filter creates a copy of the audio stream without any "
                                "modification. Useful for forcing a filter graph node or "
                                "branching the signal path.");
    hintLabel->setStyleSheet("color: gray; font-size: 11px;");
    hintLabel->setWordWrap(true);
    infoLayout->addWidget(hintLabel);

    mainLayout->addWidget(infoGroup);
    mainLayout->addStretch();

    return parametersWidget;
}

void FFAcopy::updateFFmpegFlags() {
    ffmpegFlags = "acopy";
}

QString FFAcopy::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcopy::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acopy";
}

void FFAcopy::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
