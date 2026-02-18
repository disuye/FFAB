#include "ff-adelay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>

FFAdelay::FFAdelay() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdelay::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Delay"));

    // Delays string input
    auto delaysLayout = new QHBoxLayout();
    delaysLayout->addWidget(new QLabel("Delays (ms):"));
    
    delaysEdit = new QLineEdit();
    delaysEdit->setText(m_delays);
    delaysEdit->setPlaceholderText("400|800|1200");
    delaysLayout->addWidget(delaysEdit, 1);
    
    layout->addLayout(delaysLayout);

    // All checkbox
    allCheckBox = new QCheckBox("Apply last delay to remaining channels");
    allCheckBox->setChecked(m_all);
    layout->addWidget(allCheckBox);

    // Connect signals
    connect(delaysEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_delays = text;
        updateFFmpegFlags();
    });

    connect(allCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_all = checked;
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

void FFAdelay::updateFFmpegFlags() {
    QStringList params;
    
    if (!m_delays.isEmpty()) {
        params << QString("delays=%1").arg(m_delays);
    }
    
    if (m_all) {
        params << "all=1";
    }
    
    if (params.isEmpty()) {
        ffmpegFlags = "adelay";
    } else {
        ffmpegFlags = "adelay=" + params.join(":");
    }
}

QString FFAdelay::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdelay::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adelay";
    json["delays"] = m_delays;
    json["all"] = m_all;
}

void FFAdelay::fromJSON(const QJsonObject& json) {
    m_delays = json["delays"].toString("400|800|1200");
    m_all = json["all"].toBool(false);
    updateFFmpegFlags();
}
