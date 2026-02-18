#include "ff-asdr.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAsdr::FFAsdr() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsdr::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Multi-input warning
    auto warningGroup = new QGroupBox("⚠️ Multi-Input Filter");
    auto warningLayout = new QVBoxLayout(warningGroup);
    warningGroup->setStyleSheet("QGroupBox { color: #ff9944; font-weight: bold; }");

    auto warningLabel = new QLabel(
        "This filter requires TWO audio inputs:\n"
        "• [0:a] Main input (reference signal)\n"
        "• [1:a] Audio Input (test signal)\n\n"
        "Place an Audio Input filter before this filter in the chain.\n"
        "Multiple analysis filters can share the same Audio Input."
    );
    warningLabel->setWordWrap(true);
    warningLayout->addWidget(warningLabel);

    mainLayout->addWidget(warningGroup);

    // Info group
    auto infoGroup = new QGroupBox("Signal-to-Distortion Ratio");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Measures the Signal-to-Distortion Ratio (SDR) between a reference "
        "audio signal and a test signal. SDR quantifies how much of the "
        "original signal energy is preserved vs. distortion energy."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("⚡ Reference signal [0:a] passes through to subsequent filters, but is cropped to the duration of the [n:a] test signal. Consider running analysis filters on Asplit sub-chains if you wish to avoid unexpected cropping. Sequential analysis filters will work but not if you wish to apply processing to the test signal, in which case only one analysis filter will ingest the processed test signal.");
    passLabel->setStyleSheet("color: #4a9eff; font-weight: bold;");
    passLabel->setWordWrap(true);
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // Formula group
    auto formulaGroup = new QGroupBox("Formula");
    auto formulaLayout = new QVBoxLayout(formulaGroup);

    auto formulaLabel = new QLabel(
        "SDR = 10 × log₁₀(‖reference‖² / ‖reference - test‖²)\n\n"
        "Where the denominator represents the distortion (difference "
        "between reference and test signals)."
    );
    formulaLabel->setWordWrap(true);
    formulaLayout->addWidget(formulaLabel);

    mainLayout->addWidget(formulaGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "Results are output to FFmpeg's stderr:\n\n"
        "• SDR value in dB (higher = better quality)\n"
        "• Per-channel measurements\n\n"
        "Common in audio source separation evaluation."
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAsdr::updateFFmpegFlags() {
    ffmpegFlags = "asdr";
}

QString FFAsdr::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsdr::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asdr";
}

void FFAsdr::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
