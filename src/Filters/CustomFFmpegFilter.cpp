#include "CustomFFmpegFilter.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QLabel>
#include <QCheckBox>
#include <QJsonObject>
#include <QFont>

CustomFFmpegFilter::CustomFFmpegFilter() : CustomFFmpegFilter(Position::MIDDLE) {
}

CustomFFmpegFilter::CustomFFmpegFilter(BaseFilter::Position slot) {
    position = slot;
    customCommand = getTemplateForPosition();
}

QString CustomFFmpegFilter::getTemplateForPosition() const {
    switch (position) {
        case Position::INPUT:
            return "-i input.wav";
        case Position::MIDDLE:
            return "aformat=channel_layouts=stereo,\npan=stereo|c0=1.116*c0|c1=0.884*c1,\nadelay=0|6400S,\nafreqshift=shift=200,\ndynaudnorm=f=10:g=7:p=0.99:m=20.0:r=1.0:s=28";
            case Position::OUTPUT:
            return "-c:a pcm_s24le -ar 48000";
    }
    return "";
}

QWidget* CustomFFmpegFilter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("customFFmpegFilterTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    QString title = "Custom FFmpeg Command";
    switch (position) {
        case Position::INPUT: title += ": INPUT"; break;
        case Position::MIDDLE: title += ""; break;
        case Position::OUTPUT: title += ": OUTPUT"; break;
    }
    layout->addWidget(new QLabel(title));

    textEdit = new QPlainTextEdit();
    textEdit->setPlainText(customCommand);
    QFont monoFont("Fira Code");
    monoFont.setPointSize(10);
    textEdit->setFont(monoFont);
    textEdit->setMinimumHeight(100);
    textEdit->setPlaceholderText(getTemplateForPosition());

    connect(textEdit, &QPlainTextEdit::textChanged, [this]() {
        customCommand = textEdit->toPlainText();
    });
    
    layout->addWidget(textEdit);
    
    // Add checkbox for manual output label control
    auto checkBox = new QCheckBox("Manage output labels manually (advanced)");
    checkBox->setChecked(manualOutputLabels);
    checkBox->setToolTip(
        "When enabled, FFAB won't automatically append or replace output stream labels.\n"
        "Use this for complex multi-output filters like asplit=3[s1][s2][s3].\n"
        "Warning: Incorrect labels will cause FFmpeg errors!"
    );
    connect(checkBox, &QCheckBox::toggled, [this](bool checked) {
        manualOutputLabels = checked;
    });
    layout->addWidget(checkBox);

    // Collapsible help section at bottom
    auto helpSection = new CollapsibleHelpSection(
        "<b>What is this?</b><br>"
        "Write custom ffmpeg commands, either without stream labels:<br><br>format=channel_layouts=stereo,pan=stereo|c0=1.116*c0|c1=0.884*c1,adelay=0|6400S,<br>eval=val(0)|-val(1),afreqshift=shift=200,dynaudnorm=f=10:g=7:p=0.99:m=20.0:r=1.0:s=28<br><br>Or combine this filter with Audio Inputs and stream labels:<br><br>[0:a][1:a][2:a]amix=inputs=3:duration=longest<br><br>"
        
        "<b>Quote handling:</b><br>"
        "Use either single quotes (') or double quotes (\") for parameter values.<br>"
        "Example: weights='1 1 0' or weights=\"1 1 0\" (both work the same)<br><br>"
        
        "<b>Common uses:</b><br>"
        "• Expert users can leverage FFAB for rapid protoyping of complex filter chains.<br>"
        "• Noob user can learn about signal flow without getting lost in the Terminal.<br>"
        "• Sound designers and audio artists can go nuts!"

    );
    layout->addWidget(helpSection);

    layout->addStretch();

    return parametersWidget;
}

QString CustomFFmpegFilter::buildFFmpegFlags() const {
    // Idiot proof the user input ... 
    QString cleaned = customCommand;
    cleaned.replace(", ", ",");      // Remove spaces after commas
    cleaned.replace("\n", "");       // Remove newlines
    cleaned.replace("\r", "");       // Remove carriage returns (Windows)
    
    // Convert double quotes to single quotes to avoid breaking command parsing
    // FFmpeg accepts both " and ' for parameter values (e.g., weights="1 1 0" -> weights='1 1 0')
    // This prevents nested quotes from breaking the -filter_complex "..." wrapper
    cleaned.replace("\"", "'");
    
    return cleaned.trimmed();
}
QString CustomFFmpegFilter::getDefaultCustomCommandTemplate() const {
    return getTemplateForPosition();
}

void CustomFFmpegFilter::toJSON(QJsonObject& json) const {
    json["type"] = "custom";
    json["position"] = static_cast<int>(position);
    json["command"] = customCommand;
    json["manualOutputLabels"] = manualOutputLabels;
}

void CustomFFmpegFilter::fromJSON(const QJsonObject& json) {
    position = static_cast<Position>(json["position"].toInt(1));
    customCommand = json["command"].toString(getTemplateForPosition());
    manualOutputLabels = json["manualOutputLabels"].toBool(false);
}
