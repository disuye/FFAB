#include "ff-extrastereo.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFExtrastereo::FFExtrastereo() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFExtrastereo::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Coefficient parameter (stereo difference multiplier)
    auto coefficientGroup = new QGroupBox("Stereo Difference");
    auto coefficientLayout = new QVBoxLayout(coefficientGroup);

    auto coefficientDescription = new QLabel(
        "Multiplier for the difference between channels.\n"
        "1.0 = no change, >1.0 = wider stereo, <1.0 = narrower, negative = inverted"
    );
    coefficientDescription->setWordWrap(true);
    coefficientDescription->setStyleSheet("color: gray; font-size: 11px;");
    coefficientLayout->addWidget(coefficientDescription);

    coefficientSpinBox = new QDoubleSpinBox();
    coefficientSpinBox->setMinimum(-10.0);
    coefficientSpinBox->setMaximum(10.0);
    coefficientSpinBox->setValue(m_coefficient);
    coefficientSpinBox->setDecimals(2);
    coefficientSpinBox->setSingleStep(0.1);
    coefficientSpinBox->setSuffix("x");

    auto coefficientControlLayout = new QHBoxLayout();
    coefficientControlLayout->addWidget(coefficientSpinBox);
    coefficientControlLayout->addStretch();
    
    auto coefficientMinLabel = new QLabel("-10x");
    coefficientControlLayout->addWidget(coefficientMinLabel);

    coefficientSlider = new QSlider(Qt::Horizontal);
    coefficientSlider->setMinimum(-1000);
    coefficientSlider->setMaximum(1000);
    coefficientSlider->setValue(static_cast<int>(m_coefficient * 100));
    coefficientControlLayout->addWidget(coefficientSlider, 1);

    auto coefficientMaxLabel = new QLabel("+10x");
    coefficientControlLayout->addWidget(coefficientMaxLabel);

    coefficientLayout->addLayout(coefficientControlLayout);
    mainLayout->addWidget(coefficientGroup);

    // Clipping checkbox
    auto clippingGroup = new QGroupBox("Output Clipping");
    auto clippingLayout = new QVBoxLayout(clippingGroup);

    clippingCheck = new QCheckBox("Enable clipping (prevents distortion)");
    clippingCheck->setChecked(m_clipping);

    clippingLayout->addWidget(clippingCheck);
    mainLayout->addWidget(clippingGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    connect(coefficientSlider, &QSlider::valueChanged, [this](int value) {
        m_coefficient = value / 100.0;
        coefficientSpinBox->blockSignals(true);
        coefficientSpinBox->setValue(m_coefficient);
        coefficientSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(coefficientSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_coefficient = value;
        coefficientSlider->blockSignals(true);
        coefficientSlider->setValue(static_cast<int>(value * 100));
        coefficientSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(clippingCheck, &QCheckBox::toggled, [this](bool checked) {
        m_clipping = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFExtrastereo::updateFFmpegFlags() {
    ffmpegFlags = QString("extrastereo=m=%1:c=%2")
        .arg(m_coefficient, 0, 'f', 2)
        .arg(m_clipping ? "true" : "false");
}

QString FFExtrastereo::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFExtrastereo::toJSON(QJsonObject& json) const {
    json["type"] = "ff-extrastereo";
    json["coefficient"] = m_coefficient;
    json["clipping"] = m_clipping;
}

void FFExtrastereo::fromJSON(const QJsonObject& json) {
    m_coefficient = json["coefficient"].toDouble(2.5);
    m_clipping = json["clipping"].toBool(true);
    updateFFmpegFlags();
}
