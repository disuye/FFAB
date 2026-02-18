#include "ff-aintegral.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAintegral::FFAintegral() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAintegral::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Audio Integral");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Computes the integral (cumulative sum) of the audio signal.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Effect description group
    auto effectGroup = new QGroupBox("Effect");
    auto effectLayout = new QVBoxLayout(effectGroup);

    auto effectLabel = new QLabel(
        "The integral operation:\n\n"
        "• Acts as a low-pass filter\n"
        "• Smooths transients and attacks\n"
        "• May introduce DC offset\n"
        "• Attenuates high frequencies\n"
        "• Boosts low frequencies\n\n"
        "Mathematical: output[n] = output[n-1] + input[n]"
    );
    effectLabel->setWordWrap(true);
    effectLayout->addWidget(effectLabel);

    mainLayout->addWidget(effectGroup);

    // Use cases group
    auto usesGroup = new QGroupBox("Use Cases");
    auto usesLayout = new QVBoxLayout(usesGroup);

    auto usesLabel = new QLabel(
        "• Smoothing harsh audio signals\n"
        "• Creating muffled or underwater effects\n"
        "• Undoing the aderivative operation\n"
        "• Low-frequency emphasis\n"
        "• Signal reconstruction in analysis"
    );
    usesLabel->setWordWrap(true);
    usesLayout->addWidget(usesLabel);

    auto hintLabel = new QLabel(
        "Warning: Integration can cause DC offset buildup and amplitude growth. "
        "Consider following with a high-pass filter or DC removal."
    );
    hintLabel->setStyleSheet("color: #ff9944; font-size: 11px;");
    hintLabel->setWordWrap(true);
    usesLayout->addWidget(hintLabel);

    mainLayout->addWidget(usesGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAintegral::updateFFmpegFlags() {
    ffmpegFlags = "aintegral";
}

QString FFAintegral::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAintegral::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aintegral";
}

void FFAintegral::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
