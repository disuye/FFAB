#include "ff-aderivative.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAderivative::FFAderivative() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAderivative::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Audio Derivative");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Computes the derivative (rate of change) of the audio signal.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Effect description group
    auto effectGroup = new QGroupBox("Effect");
    auto effectLayout = new QVBoxLayout(effectGroup);

    auto effectLabel = new QLabel(
        "The derivative operation:\n\n"
        "• Acts as a high-pass filter\n"
        "• Emphasizes transients and attacks\n"
        "• Removes DC offset completely\n"
        "• Attenuates low frequencies\n"
        "• Boosts high frequencies\n\n"
        "Mathematical: output[n] = input[n] - input[n-1]"
    );
    effectLabel->setWordWrap(true);
    effectLayout->addWidget(effectLabel);

    mainLayout->addWidget(effectGroup);

    // Use cases group
    auto usesGroup = new QGroupBox("Use Cases");
    auto usesLayout = new QVBoxLayout(usesGroup);

    auto usesLabel = new QLabel(
        "• Transient detection and enhancement\n"
        "• Edge detection in audio analysis\n"
        "• Removing low-frequency rumble\n"
        "• Creating aggressive, trebly textures\n"
        "• Paired with aintegral for analysis pipelines"
    );
    usesLabel->setWordWrap(true);
    usesLayout->addWidget(usesLabel);

    auto hintLabel = new QLabel("Note: Multiple applications will progressively remove more low-frequency content.");
    hintLabel->setStyleSheet("color: gray; font-size: 11px;");
    hintLabel->setWordWrap(true);
    usesLayout->addWidget(hintLabel);

    mainLayout->addWidget(usesGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAderivative::updateFFmpegFlags() {
    ffmpegFlags = "aderivative";
}

QString FFAderivative::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAderivative::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aderivative";
}

void FFAderivative::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
