#include "ff-aresample.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>

FFAresample::FFAresample() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAresample::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Resample"));

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

void FFAresample::updateFFmpegFlags() {
    if (m_sampleRate > 0) {
        ffmpegFlags = QString("aresample=%1").arg(m_sampleRate);
    } else {
        ffmpegFlags = "aresample";
    }
}

QString FFAresample::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAresample::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aresample";
    json["sample_rate"] = m_sampleRate;
}

void FFAresample::fromJSON(const QJsonObject& json) {
    m_sampleRate = json["sample_rate"].toInt(44100);
    updateFFmpegFlags();
}
