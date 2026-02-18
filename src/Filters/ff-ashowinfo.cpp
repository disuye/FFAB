#include "ff-ashowinfo.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAshowinfo::FFAshowinfo() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAshowinfo::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Audio Frame Info");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Displays detailed information about each audio frame to the FFmpeg log.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("Note: Audio passes through unchanged");
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output Information");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "For each audio frame, outputs to stderr:\n\n"
        "• n: Frame sequence number\n"
        "• pts: Presentation timestamp (time base units)\n"
        "• pts_time: PTS converted to seconds\n"
        "• fmt: Sample format (s16, s32, flt, etc.)\n"
        "• chlayout: Channel layout (stereo, 5.1, etc.)\n"
        "• rate: Sample rate in Hz\n"
        "• nb_samples: Number of samples in frame\n"
        "• checksum: CRC32 checksum of audio data"
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    mainLayout->addWidget(outputGroup);

    // Use cases group
    auto usesGroup = new QGroupBox("Use Cases");
    auto usesLayout = new QVBoxLayout(usesGroup);

    auto usesLabel = new QLabel(
        "• Debugging audio pipeline issues\n"
        "• Verifying frame timing and sync\n"
        "• Checking format conversions\n"
        "• Analyzing frame sizes and checksums\n"
        "• Diagnosing discontinuities"
    );
    usesLabel->setWordWrap(true);
    usesLayout->addWidget(usesLabel);

    auto hintLabel = new QLabel(
        "Warning: Generates a lot of output! Each frame produces one line. "
        "For a 48kHz stream with 1024-sample frames, that's ~47 lines per second."
    );
    hintLabel->setStyleSheet("color: #ff9944; font-size: 11px;");
    hintLabel->setWordWrap(true);
    usesLayout->addWidget(hintLabel);

    mainLayout->addWidget(usesGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAshowinfo::updateFFmpegFlags() {
    ffmpegFlags = "ashowinfo";
}

QString FFAshowinfo::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAshowinfo::toJSON(QJsonObject& json) const {
    json["type"] = "ff-ashowinfo";
}

void FFAshowinfo::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    updateFFmpegFlags();
}
