#include "ff-volume.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFVolume::FFVolume() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFVolume::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Volume adjustment section
    auto volumeGroup = new QGroupBox("Volume Adjustment");
    auto volumeLayout = new QVBoxLayout(volumeGroup);

    // Slider with endpoint labels
    auto sliderLayout = new QHBoxLayout();
    
    auto quieterLabel = new QLabel("-20 dB");
    quieterLabel->setToolTip("Reduce volume");
    sliderLayout->addWidget(quieterLabel);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setMinimum(-200);
    volumeSlider->setMaximum(200);
    volumeSlider->setValue(static_cast<int>(decibels * 10));
    volumeSlider->setToolTip("Adjust volume in decibels");
    sliderLayout->addWidget(volumeSlider, 1);

    auto louderLabel = new QLabel("+20 dB");
    louderLabel->setToolTip("Increase volume");
    sliderLayout->addWidget(louderLabel);

    volumeLayout->addLayout(sliderLayout);

    // Value display and precise control
    auto valueLayout = new QHBoxLayout();
    
    decibelLabel = new QLabel(QString("%1 dB").arg(decibels, 0, 'f', 1));
    valueLayout->addWidget(decibelLabel);
    
    valueLayout->addStretch();
    
    auto preciseLabel = new QLabel("Precise:");
    valueLayout->addWidget(preciseLabel);
    
    decibelSpinBox = new QDoubleSpinBox();
    decibelSpinBox->setMinimum(-20.0);
    decibelSpinBox->setMaximum(20.0);
    decibelSpinBox->setValue(decibels);
    decibelSpinBox->setSingleStep(0.1);
    decibelSpinBox->setDecimals(1);
    decibelSpinBox->setSuffix(" dB");
    decibelSpinBox->setToolTip("Enter exact decibel value");
    valueLayout->addWidget(decibelSpinBox);
    
    volumeLayout->addLayout(valueLayout);

    // Connect slider
    connect(volumeSlider, &QSlider::valueChanged, [this](int value) {
        decibels = value / 10.0;
        updateFFmpegFlags();
        if (decibelSpinBox) {
            decibelSpinBox->blockSignals(true);
            decibelSpinBox->setValue(decibels);
            decibelSpinBox->blockSignals(false);
        }
        if (decibelLabel) {
            decibelLabel->setText(QString("%1 dB").arg(decibels, 0, 'f', 1));
        }
    });

    // Connect spinbox
    connect(decibelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        decibels = value;
        updateFFmpegFlags();
        
        volumeSlider->blockSignals(true);
        volumeSlider->setValue(static_cast<int>(decibels * 10));
        volumeSlider->blockSignals(false);
        
        if (decibelLabel) {
            decibelLabel->setText(QString("%1 dB").arg(decibels, 0, 'f', 1));
        }
    });

    mainLayout->addWidget(volumeGroup);
    mainLayout->addStretch();

    return parametersWidget;
}

void FFVolume::updateFFmpegFlags() {
    ffmpegFlags = QString("volume=%1dB").arg(decibels);
}

QString FFVolume::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFVolume::toJSON(QJsonObject& json) const {
    json["type"] = "ff-volume";
    json["decibels"] = decibels;
}

void FFVolume::fromJSON(const QJsonObject& json) {
    decibels = json["decibels"].toDouble(-0.3);
    updateFFmpegFlags();
}