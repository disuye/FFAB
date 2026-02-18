#include "ff-asetrate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>

FFAsetrate::FFAsetrate() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsetrate::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Set Rate"));

    // Sample rate input
    auto rateLayout = new QHBoxLayout();
    rateLayout->addWidget(new QLabel("Sample Rate:"));
    
    sampleRateSpinBox = new QSpinBox();
    sampleRateSpinBox->setMinimum(1);
    sampleRateSpinBox->setMaximum(384000);
    sampleRateSpinBox->setValue(m_sampleRate);
    sampleRateSpinBox->setSuffix(" Hz");
    rateLayout->addWidget(sampleRateSpinBox);
    rateLayout->addStretch();
    
    layout->addLayout(rateLayout);

    // Connect signal
    connect(sampleRateSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_sampleRate = value;
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

void FFAsetrate::updateFFmpegFlags() {
    ffmpegFlags = QString("asetrate=%1").arg(m_sampleRate);
}

QString FFAsetrate::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsetrate::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asetrate";
    json["sample_rate"] = m_sampleRate;
}

void FFAsetrate::fromJSON(const QJsonObject& json) {
    m_sampleRate = json["sample_rate"].toInt(44100);
    updateFFmpegFlags();
}
