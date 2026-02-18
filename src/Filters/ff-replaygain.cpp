#include "ff-replaygain.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFReplaygain::FFReplaygain() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFReplaygain::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("ReplayGain Analysis");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Calculates ReplayGain values for loudness normalization.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("Note: Audio passes through unchanged");
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // About ReplayGain group
    auto aboutGroup = new QGroupBox("About ReplayGain");
    auto aboutLayout = new QVBoxLayout(aboutGroup);

    auto aboutLabel = new QLabel(
        "ReplayGain is a standard for measuring and normalizing perceived "
        "loudness of audio. It analyzes psychoacoustic loudness to suggest "
        "a gain adjustment that brings the track to a reference level of "
        "-18 LUFS (roughly 89 dB SPL).\n\n"
        "This allows playback of different tracks at consistent perceived volume "
        "without clipping."
    );
    aboutLabel->setWordWrap(true);
    aboutLayout->addWidget(aboutLabel);

    mainLayout->addWidget(aboutGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output Values");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "Results are output to FFmpeg's stderr after processing:\n\n"
        "• track_gain: Suggested gain adjustment in dB\n"
        "  (positive = boost needed, negative = reduce)\n\n"
        "• track_peak: Maximum sample value (0.0-1.0)\n"
        "  (use to prevent clipping when applying gain)"
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    auto hintLabel = new QLabel(
        "To apply the measured gain, use the volume filter with the "
        "track_gain value, being careful not to exceed track_peak."
    );
    hintLabel->setStyleSheet("color: gray; font-size: 11px;");
    hintLabel->setWordWrap(true);
    outputLayout->addWidget(hintLabel);

    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFReplaygain::updateFFmpegFlags() {
    ffmpegFlags = "replaygain";
}

QString FFReplaygain::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFReplaygain::toJSON(QJsonObject& json) const {
    json["type"] = "ff-replaygain";
}

void FFReplaygain::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
