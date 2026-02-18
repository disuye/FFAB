#include "ff-vibrato.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFVibrato::FFVibrato() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFVibrato::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Frequency parameter (0.1 - 20000 Hz) - logarithmic
    auto frequencyGroup = new QGroupBox("Frequency (Hz)");
    auto frequencyLayout = new QVBoxLayout(frequencyGroup);

    frequencySpinBox = new QDoubleSpinBox();
    frequencySpinBox->setMinimum(0.1);
    frequencySpinBox->setMaximum(20000.0);
    frequencySpinBox->setValue(m_frequency);
    frequencySpinBox->setDecimals(2);
    frequencySpinBox->setSingleStep(0.1);
    frequencySpinBox->setSuffix(" Hz");

    // Default 5 Hz -> log mapping
    double logMin = std::log(0.1);
    double logMax = std::log(20000.0);
    double logValue = std::log(5.0);
    int sliderValue = static_cast<int>(1000.0 * (logValue - logMin) / (logMax - logMin));

    auto frequencyControlLayout = new QHBoxLayout();
    frequencyControlLayout->addWidget(frequencySpinBox);
    frequencyControlLayout->addStretch();
    
    auto frequencyMinLabel = new QLabel("0.1");
    frequencyControlLayout->addWidget(frequencyMinLabel);

    frequencySlider = new QSlider(Qt::Horizontal);
    frequencySlider->setMinimum(0);    // Will map logarithmically
    frequencySlider->setMaximum(1000); // 0-1000 for smooth logarithmic mapping
    frequencySlider->setValue(sliderValue);
    frequencyControlLayout->addWidget(frequencySlider, 1);

    auto frequencyMaxLabel = new QLabel("20 kHz");
    frequencyControlLayout->addWidget(frequencyMaxLabel);

    frequencyLayout->addLayout(frequencyControlLayout);
    mainLayout->addWidget(frequencyGroup);

    // Depth parameter (0 - 1)
    auto depthGroup = new QGroupBox("Depth");
    auto depthLayout = new QVBoxLayout(depthGroup);

    depthSpinBox = new QDoubleSpinBox();
    depthSpinBox->setMinimum(0.0);
    depthSpinBox->setMaximum(1.0);
    depthSpinBox->setValue(m_depth);
    depthSpinBox->setDecimals(2);
    depthSpinBox->setSingleStep(0.01);

    auto depthControlLayout = new QHBoxLayout();
    depthControlLayout->addWidget(depthSpinBox);
    depthControlLayout->addStretch();
    
    auto depthMinLabel = new QLabel("0%");
    depthControlLayout->addWidget(depthMinLabel);

    depthSlider = new QSlider(Qt::Horizontal);
    depthSlider->setMinimum(0);
    depthSlider->setMaximum(100);
    depthSlider->setValue(50); // default 0.5
    depthControlLayout->addWidget(depthSlider, 1);

    auto depthMaxLabel = new QLabel("100%");
    depthControlLayout->addWidget(depthMaxLabel);

    depthLayout->addLayout(depthControlLayout);
    mainLayout->addWidget(depthGroup);

    // Connect frequency slider and spinbox (logarithmic)
    connect(frequencySlider, &QSlider::valueChanged, [this](int value) {
        // Logarithmic mapping: 0-1000 -> 0.1 to 20000 Hz
        double logMin = std::log(0.1);
        double logMax = std::log(20000.0);
        double logValue = logMin + (logMax - logMin) * (value / 1000.0);
        m_frequency = std::exp(logValue);
        
        frequencySpinBox->blockSignals(true);
        frequencySpinBox->setValue(m_frequency);
        frequencySpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(frequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_frequency = value;
        
        // Update slider logarithmically
        double logMin = std::log(0.1);
        double logMax = std::log(20000.0);
        double logValue = std::log(value);
        int sliderValue = static_cast<int>(1000.0 * (logValue - logMin) / (logMax - logMin));
        
        frequencySlider->blockSignals(true);
        frequencySlider->setValue(sliderValue);
        frequencySlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect depth slider and spinbox
    connect(depthSlider, &QSlider::valueChanged, [this](int value) {
        m_depth = value / 100.0;
        
        depthSpinBox->blockSignals(true);
        depthSpinBox->setValue(m_depth);
        depthSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(depthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_depth = value;
        
        depthSlider->blockSignals(true);
        depthSlider->setValue(static_cast<int>(value * 100));
        depthSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFVibrato::updateFFmpegFlags() {
    ffmpegFlags = QString("vibrato=f=%1:d=%2")
                      .arg(m_frequency, 0, 'f', 1)
                      .arg(m_depth, 0, 'f', 2);
}

QString FFVibrato::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFVibrato::toJSON(QJsonObject& json) const {
    json["type"] = "ff-vibrato";
    json["frequency"] = m_frequency;
    json["depth"] = m_depth;
}

void FFVibrato::fromJSON(const QJsonObject& json) {
    m_frequency = json["frequency"].toDouble(5.0);
    m_depth = json["depth"].toDouble(0.5);
    updateFFmpegFlags();
}
