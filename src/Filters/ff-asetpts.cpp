#include "ff-asetpts.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>

FFAsetpts::FFAsetpts() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsetpts::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Set PTS / Presentation Time Stamp"));
    
    // Expression input
    auto exprLayout = new QHBoxLayout();
    exprLayout->addWidget(new QLabel("Expression:"));
    
    exprEdit = new QLineEdit();
    exprEdit->setText(m_expr);
    exprEdit->setPlaceholderText("N/SR/TB");
    exprLayout->addWidget(exprEdit, 1);
    
    layout->addLayout(exprLayout);

    // Connect signal
    connect(exprEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_expr = text;
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

void FFAsetpts::updateFFmpegFlags() {
    if (!m_expr.isEmpty()) {
        ffmpegFlags = QString("asetpts=%1").arg(m_expr);
    } else {
        ffmpegFlags = "asetpts=PTS";
    }
}

QString FFAsetpts::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsetpts::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asetpts";
    json["expr"] = m_expr;
}

void FFAsetpts::fromJSON(const QJsonObject& json) {
    m_expr = json["expr"].toString("PTS");
    updateFFmpegFlags();
}
