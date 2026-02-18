#include "ff-anullsink.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>

FFAnullsink::FFAnullsink() {
    position = Position::MIDDLE;
}

QWidget* FFAnullsink::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto* layout = new QVBoxLayout(parametersWidget);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(12);

    auto* titleLabel = new QLabel("Null Sink (Discard Audio)");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    auto* descLabel = new QLabel(
        "Consumes and discards the audio stream.\n\n"
        "Use this when you only want non-audio outputs "
        "(such as waveform images) without producing an audio file.\n\n"
        "This filter must be at the end of the main chain. "
        "When active, no audio output file will be generated."
    );
    descLabel->setWordWrap(true);
    layout->addWidget(descLabel);

    auto* noteLabel = new QLabel(
        "<b>Note:</b> Branch outputs (Aux Output, Waveform Image) "
        "placed <i>before</i> this filter will still be generated."
    );
    noteLabel->setWordWrap(true);
    noteLabel->setStyleSheet("color: #808080; font-style: italic;");
    layout->addWidget(noteLabel);

    layout->addStretch();

    return parametersWidget;
}

QString FFAnullsink::buildFFmpegFlags() const {
    return "anullsink";
}

QString FFAnullsink::getDefaultCustomCommandTemplate() const {
    return "anullsink";
}

void FFAnullsink::toJSON(QJsonObject& json) const {
    json["type"] = "ff-anullsink";
}

void FFAnullsink::fromJSON(const QJsonObject& json) {
    Q_UNUSED(json);
    // No parameters to load
}
