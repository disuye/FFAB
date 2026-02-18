#include "ff-earwax.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFEarwax::FFEarwax() {
    position = Position::MIDDLE;
    ffmpegFlags = "earwax";
}

QWidget* FFEarwax::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Information group
    auto infoGroup = new QGroupBox("About Earwax");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto description = new QLabel(
        "Earwax is a fixed stereo enhancement effect designed for headphone listening.\n\n"
        "It adds acoustic cues that make the stereo image appear to come from "
        "outside your head, rather than from within. This creates a more natural "
        "and less fatiguing listening experience.\n\n"
        "This filter has no adjustable parameters — it applies a fixed HRTF-like "
        "processing to the stereo signal.\n\n"
        "Best suited for: Headphone listening, reducing listener fatigue, "
        "creating a more speaker-like presentation from headphones."
    );
    description->setWordWrap(true);
    description->setStyleSheet("padding: 8px;");
    infoLayout->addWidget(description);
    
    mainLayout->addWidget(infoGroup);

    // Technical note
    auto noteGroup = new QGroupBox("Technical Note");
    auto noteLayout = new QVBoxLayout(noteGroup);

    auto techNote = new QLabel(
        "Input: Stereo audio only (2 channels)\n"
        "Output: Stereo audio\n"
        "Latency: Minimal"
    );
    techNote->setStyleSheet("color: gray; font-size: 11px; padding: 4px;");
    noteLayout->addWidget(techNote);
    
    mainLayout->addWidget(noteGroup);

    mainLayout->addStretch();

    return parametersWidget;
}

QString FFEarwax::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFEarwax::toJSON(QJsonObject& json) const {
    json["type"] = "ff-earwax";
}

void FFEarwax::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    // No parameters to restore
}
