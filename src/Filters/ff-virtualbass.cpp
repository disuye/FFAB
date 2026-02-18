#include "ff-virtualbass.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFVirtualbass::FFVirtualbass() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFVirtualbass::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Cutoff parameter (100 - 500 Hz)
    auto cutoffGroup = new QGroupBox("Cutoff Frequency (Hz)");
    auto cutoffLayout = new QVBoxLayout(cutoffGroup);

    cutoffSpinBox = new QDoubleSpinBox();
    cutoffSpinBox->setMinimum(100.0);
    cutoffSpinBox->setMaximum(500.0);
    cutoffSpinBox->setValue(m_cutoff);
    cutoffSpinBox->setDecimals(0);
    cutoffSpinBox->setSingleStep(10);
    cutoffSpinBox->setSuffix(" Hz");

    auto cutoffControlLayout = new QHBoxLayout();
    cutoffControlLayout->addWidget(cutoffSpinBox);
    cutoffControlLayout->addStretch();
    cutoffControlLayout->addWidget(new QLabel("100"));

    cutoffSlider = new QSlider(Qt::Horizontal);
    cutoffSlider->setMinimum(100);
    cutoffSlider->setMaximum(500);
    cutoffSlider->setValue(static_cast<int>(m_cutoff));
    cutoffControlLayout->addWidget(cutoffSlider, 1);
    cutoffControlLayout->addWidget(new QLabel("500"));

    cutoffLayout->addLayout(cutoffControlLayout);

    auto cutoffHintLabel = new QLabel("Frequencies below this will be enhanced with harmonics");
    cutoffHintLabel->setStyleSheet("color: gray; font-size: 11px;");
    cutoffLayout->addWidget(cutoffHintLabel);

    mainLayout->addWidget(cutoffGroup);

    // Strength parameter (0.5 - 3)
    auto strengthGroup = new QGroupBox("Strength");
    auto strengthLayout = new QVBoxLayout(strengthGroup);

    strengthSpinBox = new QDoubleSpinBox();
    strengthSpinBox->setMinimum(0.5);
    strengthSpinBox->setMaximum(3.0);
    strengthSpinBox->setValue(m_strength);
    strengthSpinBox->setDecimals(2);
    strengthSpinBox->setSingleStep(0.1);

    auto strengthControlLayout = new QHBoxLayout();
    strengthControlLayout->addWidget(strengthSpinBox);
    strengthControlLayout->addStretch();
    strengthControlLayout->addWidget(new QLabel("0.5"));

    strengthSlider = new QSlider(Qt::Horizontal);
    strengthSlider->setMinimum(50);
    strengthSlider->setMaximum(300);
    strengthSlider->setValue(static_cast<int>(m_strength * 100));
    strengthControlLayout->addWidget(strengthSlider, 1);
    strengthControlLayout->addWidget(new QLabel("3.0"));

    strengthLayout->addLayout(strengthControlLayout);

    auto strengthHintLabel = new QLabel("Amount of psychoacoustic bass enhancement");
    strengthHintLabel->setStyleSheet("color: gray; font-size: 11px;");
    strengthLayout->addWidget(strengthHintLabel);

    mainLayout->addWidget(strengthGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Cutoff
    connect(cutoffSlider, &QSlider::valueChanged, [this](int value) {
        m_cutoff = value;
        cutoffSpinBox->blockSignals(true);
        cutoffSpinBox->setValue(m_cutoff);
        cutoffSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(cutoffSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_cutoff = value;
        cutoffSlider->blockSignals(true);
        cutoffSlider->setValue(static_cast<int>(value));
        cutoffSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Strength
    connect(strengthSlider, &QSlider::valueChanged, [this](int value) {
        m_strength = value / 100.0;
        strengthSpinBox->blockSignals(true);
        strengthSpinBox->setValue(m_strength);
        strengthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(strengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_strength = value;
        strengthSlider->blockSignals(true);
        strengthSlider->setValue(static_cast<int>(value * 100));
        strengthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFVirtualbass::updateFFmpegFlags() {
    ffmpegFlags = QString("virtualbass=cutoff=%1:strength=%2")
                      .arg(m_cutoff, 0, 'f', 0)
                      .arg(m_strength, 0, 'f', 2);
}

QString FFVirtualbass::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFVirtualbass::toJSON(QJsonObject& json) const {
    json["type"] = "ff-virtualbass";
    json["cutoff"] = m_cutoff;
    json["strength"] = m_strength;
}

void FFVirtualbass::fromJSON(const QJsonObject& json) {
    m_cutoff = json["cutoff"].toDouble(250.0);
    m_strength = json["strength"].toDouble(3.0);
    updateFFmpegFlags();
}
