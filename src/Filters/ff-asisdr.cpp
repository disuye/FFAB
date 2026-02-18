#include "ff-asisdr.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAsisdr::FFAsisdr() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsisdr::getParametersWidget() {
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
    auto infoGroup = new QGroupBox("Scale-Invariant SDR");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Measures the Scale-Invariant Signal-to-Distortion Ratio (SI-SDR) "
        "between a reference and test signal. Unlike standard SDR, SI-SDR "
        "is invariant to the overall scale (gain) of the test signal."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("⚡ Reference signal [0:a] passes through to subsequent filters, but is cropped to the duration of the [n:a] test signal. Consider running analysis filters on Asplit sub-chains if you wish to avoid unexpected cropping. Sequential analysis filters will work but not if you wish to apply processing to the test signal, in which case only one analysis filter will ingest the processed test signal.");
    passLabel->setStyleSheet("color: #4a9eff; font-weight: bold;");
    passLabel->setWordWrap(true);
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // Why SI-SDR group
    auto whyGroup = new QGroupBox("Why Scale-Invariant?");
    auto whyLayout = new QVBoxLayout(whyGroup);

    auto whyLabel = new QLabel(
        "Standard SDR can be artificially inflated or deflated by "
        "gain differences between reference and test signals. SI-SDR "
        "first finds the optimal scaling factor, making it more robust "
        "for comparing audio source separation algorithms.\n\n"
        "The test signal is optimally rescaled before computing the ratio:\n"
        "α = <test, reference> / <test, test>"
    );
    whyLabel->setWordWrap(true);
    whyLayout->addWidget(whyLabel);

    mainLayout->addWidget(whyGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "Results are output to FFmpeg's stderr:\n\n"
        "• SI-SDR value in dB (higher = better quality)\n"
        "• Per-channel measurements\n\n"
        "Standard metric for audio source separation evaluation."
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAsisdr::updateFFmpegFlags() {
    ffmpegFlags = "asisdr";
}

QString FFAsisdr::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsisdr::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asisdr";
}

void FFAsisdr::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
