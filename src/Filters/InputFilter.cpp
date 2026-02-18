#include "InputFilter.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>

InputFilter::InputFilter() {
    position = Position::INPUT;
}

QWidget* InputFilter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("inputFilterTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    // Simple info label - no file selection
    auto infoLabel = new QLabel(
        "<b>Input Source:</b><br><br>"
        "Files are loaded from the <b>Input & Batch Admin</b> panel below.<br><br>"
        "Use [Add Folder] or [Add Files] to select audio files for processing."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { color: #808080; }");
    layout->addWidget(infoLabel);

    layout->addStretch();

    return parametersWidget;
}

QString InputFilter::buildFFmpegFlags() const {
    // Input file is provided by BatchProcessor at runtime
    // This is just a placeholder for the filter chain structure
    return "-i \"{INPUT}\"";
}

QString InputFilter::getDefaultCustomCommandTemplate() const {
    return "-i input.wav";
}

void InputFilter::toJSON(QJsonObject& json) const {
    json["type"] = "input";
}

void InputFilter::fromJSON(const QJsonObject& json) {
    // Nothing to load anymore
}
