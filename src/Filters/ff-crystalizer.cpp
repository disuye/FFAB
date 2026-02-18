#include "ff-crystalizer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFCrystalizer::FFCrystalizer() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFCrystalizer::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Intensity parameter (-10 - 10)
    auto intensityGroup = new QGroupBox("Intensity");
    auto intensityLayout = new QVBoxLayout(intensityGroup);

    intensitySpinBox = new QDoubleSpinBox();
    intensitySpinBox->setMinimum(-10.0);
    intensitySpinBox->setMaximum(10.0);
    intensitySpinBox->setValue(m_intensity);
    intensitySpinBox->setDecimals(1);
    intensitySpinBox->setSingleStep(0.1);

    auto intensityControlLayout = new QHBoxLayout();
    intensityControlLayout->addWidget(intensitySpinBox);
    intensityControlLayout->addStretch();
    intensityControlLayout->addWidget(new QLabel("-10"));

    intensitySlider = new QSlider(Qt::Horizontal);
    intensitySlider->setMinimum(-1000);
    intensitySlider->setMaximum(1000);
    intensitySlider->setValue(static_cast<int>(m_intensity * 100));
    intensityControlLayout->addWidget(intensitySlider, 1);
    intensityControlLayout->addWidget(new QLabel("+10"));

    intensityLayout->addLayout(intensityControlLayout);

    auto intensityHintLabel = new QLabel("Positive = sharpen, Negative = soften");
    intensityHintLabel->setStyleSheet("color: gray; font-size: 11px;");
    intensityLayout->addWidget(intensityHintLabel);

    mainLayout->addWidget(intensityGroup);

    // Clipping checkbox
    auto clipGroup = new QGroupBox("Options");
    auto clipLayout = new QVBoxLayout(clipGroup);
    clipCheckBox = new QCheckBox("Enable Clipping Protection");
    clipCheckBox->setChecked(m_clip);
    clipLayout->addWidget(clipCheckBox);
    mainLayout->addWidget(clipGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Intensity
    connect(intensitySlider, &QSlider::valueChanged, [this](int value) {
        m_intensity = value / 100.0;
        intensitySpinBox->blockSignals(true);
        intensitySpinBox->setValue(m_intensity);
        intensitySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(intensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_intensity = value;
        intensitySlider->blockSignals(true);
        intensitySlider->setValue(static_cast<int>(value * 100));
        intensitySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Clip
    connect(clipCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_clip = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFCrystalizer::updateFFmpegFlags() {
    ffmpegFlags = QString("crystalizer=i=%1:c=%2")
                      .arg(m_intensity, 0, 'f', 2)
                      .arg(m_clip ? "true" : "false");
}

QString FFCrystalizer::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFCrystalizer::toJSON(QJsonObject& json) const {
    json["type"] = "ff-crystalizer";
    json["intensity"] = m_intensity;
    json["clip"] = m_clip;
}

void FFCrystalizer::fromJSON(const QJsonObject& json) {
    m_intensity = json["intensity"].toDouble(2.0);
    m_clip = json["clip"].toBool(true);
    updateFFmpegFlags();
}
