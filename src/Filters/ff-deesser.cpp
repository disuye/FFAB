#include "ff-deesser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFDeesser::FFDeesser() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFDeesser::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Intensity parameter (0 - 100%)
    auto intensityGroup = new QGroupBox("Intensity");
    auto intensityLayout = new QVBoxLayout(intensityGroup);

    intensitySpinBox = new QDoubleSpinBox();
    intensitySpinBox->setMinimum(0.0);
    intensitySpinBox->setMaximum(100.0);
    intensitySpinBox->setValue(m_intensity * 100.0);
    intensitySpinBox->setDecimals(1);
    intensitySpinBox->setSingleStep(1.0);
    intensitySpinBox->setSuffix(" %");

    auto intensityControlLayout = new QHBoxLayout();
    intensityControlLayout->addWidget(intensitySpinBox);
    intensityControlLayout->addStretch();
    
    auto intensityMinLabel = new QLabel("0%");
    intensityControlLayout->addWidget(intensityMinLabel);

    intensitySlider = new QSlider(Qt::Horizontal);
    intensitySlider->setMinimum(0);
    intensitySlider->setMaximum(100);
    intensitySlider->setValue(static_cast<int>(m_intensity * 100));
    intensityControlLayout->addWidget(intensitySlider, 1);

    auto intensityMaxLabel = new QLabel("100%");
    intensityControlLayout->addWidget(intensityMaxLabel);

    auto intensityHint = new QLabel("How aggressively to reduce sibilance");
    intensityHint->setStyleSheet("color: gray; font-size: 11px;");
    intensityLayout->addLayout(intensityControlLayout);
    intensityLayout->addWidget(intensityHint);
    mainLayout->addWidget(intensityGroup);

    // Max de-essing parameter (0 - 100%)
    auto maxGroup = new QGroupBox("Maximum Reduction");
    auto maxLayout = new QVBoxLayout(maxGroup);

    maxSpinBox = new QDoubleSpinBox();
    maxSpinBox->setMinimum(0.0);
    maxSpinBox->setMaximum(100.0);
    maxSpinBox->setValue(m_max * 100.0);
    maxSpinBox->setDecimals(1);
    maxSpinBox->setSingleStep(1.0);
    maxSpinBox->setSuffix(" %");

    auto maxControlLayout = new QHBoxLayout();
    maxControlLayout->addWidget(maxSpinBox);
    maxControlLayout->addStretch();
    
    auto maxMinLabel = new QLabel("0%");
    maxControlLayout->addWidget(maxMinLabel);

    maxSlider = new QSlider(Qt::Horizontal);
    maxSlider->setMinimum(0);
    maxSlider->setMaximum(100);
    maxSlider->setValue(static_cast<int>(m_max * 100));
    maxControlLayout->addWidget(maxSlider, 1);

    auto maxMaxLabel = new QLabel("100%");
    maxControlLayout->addWidget(maxMaxLabel);

    auto maxHint = new QLabel("Maximum gain reduction limit");
    maxHint->setStyleSheet("color: gray; font-size: 11px;");
    maxLayout->addLayout(maxControlLayout);
    maxLayout->addWidget(maxHint);
    mainLayout->addWidget(maxGroup);

    // Frequency parameter (0 - 100% - normalized frequency)
    auto frequencyGroup = new QGroupBox("Frequency");
    auto frequencyLayout = new QVBoxLayout(frequencyGroup);

    frequencySpinBox = new QDoubleSpinBox();
    frequencySpinBox->setMinimum(0.0);
    frequencySpinBox->setMaximum(100.0);
    frequencySpinBox->setValue(m_frequency * 100.0);
    frequencySpinBox->setDecimals(1);
    frequencySpinBox->setSingleStep(1.0);
    frequencySpinBox->setSuffix(" %");

    auto frequencyControlLayout = new QHBoxLayout();
    frequencyControlLayout->addWidget(frequencySpinBox);
    frequencyControlLayout->addStretch();
    
    auto frequencyMinLabel = new QLabel("Low");
    frequencyControlLayout->addWidget(frequencyMinLabel);

    frequencySlider = new QSlider(Qt::Horizontal);
    frequencySlider->setMinimum(0);
    frequencySlider->setMaximum(100);
    frequencySlider->setValue(static_cast<int>(m_frequency * 100));
    frequencyControlLayout->addWidget(frequencySlider, 1);

    auto frequencyMaxLabel = new QLabel("High");
    frequencyControlLayout->addWidget(frequencyMaxLabel);

    auto frequencyHint = new QLabel("Target frequency band (normalized 0-1)");
    frequencyHint->setStyleSheet("color: gray; font-size: 11px;");
    frequencyLayout->addLayout(frequencyControlLayout);
    frequencyLayout->addWidget(frequencyHint);
    mainLayout->addWidget(frequencyGroup);

    // Output Mode combo
    auto outputModeGroup = new QGroupBox("Output Mode");
    auto outputModeLayout = new QVBoxLayout(outputModeGroup);

    outputModeCombo = new QComboBox();
    outputModeCombo->addItem("Output (processed)", 0);
    outputModeCombo->addItem("Input (original)", 1);
    outputModeCombo->addItem("Noise (removed sibilance)", 2);
    outputModeCombo->setCurrentIndex(m_outputMode);

    auto outputModeHint = new QLabel("Listen to processed audio, original, or isolated sibilance");
    outputModeHint->setStyleSheet("color: gray; font-size: 11px;");
    outputModeLayout->addWidget(outputModeCombo);
    outputModeLayout->addWidget(outputModeHint);
    mainLayout->addWidget(outputModeGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Intensity
    connect(intensitySlider, &QSlider::valueChanged, [this](int value) {
        m_intensity = value / 100.0;
        intensitySpinBox->blockSignals(true);
        intensitySpinBox->setValue(value);
        intensitySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(intensitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_intensity = value / 100.0;
        intensitySlider->blockSignals(true);
        intensitySlider->setValue(static_cast<int>(value));
        intensitySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Max
    connect(maxSlider, &QSlider::valueChanged, [this](int value) {
        m_max = value / 100.0;
        maxSpinBox->blockSignals(true);
        maxSpinBox->setValue(value);
        maxSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(maxSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_max = value / 100.0;
        maxSlider->blockSignals(true);
        maxSlider->setValue(static_cast<int>(value));
        maxSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Frequency
    connect(frequencySlider, &QSlider::valueChanged, [this](int value) {
        m_frequency = value / 100.0;
        frequencySpinBox->blockSignals(true);
        frequencySpinBox->setValue(value);
        frequencySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(frequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_frequency = value / 100.0;
        frequencySlider->blockSignals(true);
        frequencySlider->setValue(static_cast<int>(value));
        frequencySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Output Mode combo
    connect(outputModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_outputMode = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFDeesser::updateFFmpegFlags() {
    QStringList outputModeNames = {"o", "i", "n"};
    QString outputModeName = (m_outputMode >= 0 && m_outputMode < outputModeNames.size()) ? outputModeNames[m_outputMode] : "o";
    
    ffmpegFlags = QString("deesser=i=%1:m=%2:f=%3:s=%4")
                      .arg(m_intensity, 0, 'f', 3)
                      .arg(m_max, 0, 'f', 3)
                      .arg(m_frequency, 0, 'f', 3)
                      .arg(outputModeName);
}

QString FFDeesser::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFDeesser::toJSON(QJsonObject& json) const {
    json["type"] = "ff-deesser";
    json["intensity"] = m_intensity;
    json["max"] = m_max;
    json["frequency"] = m_frequency;
    json["outputMode"] = m_outputMode;
}

void FFDeesser::fromJSON(const QJsonObject& json) {
    m_intensity = json["intensity"].toDouble(0.0);
    m_max = json["max"].toDouble(0.5);
    m_frequency = json["frequency"].toDouble(0.5);
    m_outputMode = json["outputMode"].toInt(0);
    updateFFmpegFlags();
}
