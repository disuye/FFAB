#include "ff-volumedetect.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFVolumedetect::FFVolumedetect() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFVolumedetect::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Volume Detection");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Analyzes audio volume and outputs statistics.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("Note: Audio passes through unchanged");
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output Statistics");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "This filter outputs to FFmpeg's stderr:\n\n"
        "• n_samples: Total number of samples\n"
        "• mean_volume: Average volume in dB\n"
        "• max_volume: Peak volume in dB\n"
        "• histogram: Distribution of volume levels"
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    auto hintLabel = new QLabel("Check the FFmpeg log output after processing to see the measured values.");
    hintLabel->setStyleSheet("color: gray; font-size: 11px;");
    hintLabel->setWordWrap(true);
    outputLayout->addWidget(hintLabel);

    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFVolumedetect::updateFFmpegFlags() {
    ffmpegFlags = "volumedetect";
}

QString FFVolumedetect::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFVolumedetect::toJSON(QJsonObject& json) const {
    json["type"] = "ff-volumedetect";
}

void FFVolumedetect::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
