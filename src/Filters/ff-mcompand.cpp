#include "ff-mcompand.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTextEdit>

FFMcompand::FFMcompand() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFMcompand::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel("Multiband compression/expansion with complex filter string.");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Args parameter (complex string)
    auto argsGroup = new QGroupBox("Multiband Arguments");
    auto argsLayout = new QVBoxLayout(argsGroup);

    auto formatLabel = new QLabel("Format: attack,decay segments slope crossfreq | ...");
    formatLabel->setWordWrap(true);
    formatLabel->setStyleSheet("font-style: italic; color: gray;");
    argsLayout->addWidget(formatLabel);

    argsEdit = new QLineEdit();
    argsEdit->setText(m_args);
    argsEdit->setPlaceholderText("0.005,0.1 6 -47/-40,-34/-34,-17/-33 100 | ...");
    
    connect(argsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_args = text;
        updateFFmpegFlags();
    });

    argsLayout->addWidget(argsEdit);

    // Example text
    auto exampleLabel = new QLabel(
        "<b>Default Example:</b><br>"
        "Five bands with attack/decay, segments, transfer points, and crossover frequencies.<br>"
        "Format: <i>attack,decay segments transfer_points crossfreq</i>"
    );
    exampleLabel->setWordWrap(true);
    exampleLabel->setTextFormat(Qt::RichText);
    exampleLabel->setStyleSheet("font-size: 10px; color: #666;");
    argsLayout->addWidget(exampleLabel);

    mainLayout->addWidget(argsGroup);
    mainLayout->addStretch();

    return parametersWidget;
}

void FFMcompand::updateFFmpegFlags() {
    if (m_args.isEmpty()) {
        // Use default if empty
        ffmpegFlags = "mcompand";
    } else {
        // Escape the args string for FFmpeg
        QString escapedArgs = m_args;
        escapedArgs.replace("'", "'\\''");
        ffmpegFlags = QString("mcompand='%1'").arg(escapedArgs);
    }
}

QString FFMcompand::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFMcompand::toJSON(QJsonObject& json) const {
    json["type"] = "ff-mcompand";
    json["args"] = m_args;
}

void FFMcompand::fromJSON(const QJsonObject& json) {
    m_args = json["args"].toString("0.005,0.1 6 -47/-40,-34/-34,-17/-33 100 | 0.003,0.05 6 -47/-40,-34/-34,-17/-33 400 | 0.000625,0.0125 6 -47/-40,-34/-34,-15/-33 1600 | 0.0001,0.025 6 -47/-40,-34/-34,-31/-31,-0/-30 6400 | 0,0.025 6 -38/-31,-28/-28,-0/-25 22000");
    updateFFmpegFlags();
}
