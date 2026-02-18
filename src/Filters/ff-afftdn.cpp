#include "ff-afftdn.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFAfftdn::FFAfftdn() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAfftdn::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: DENOISE ==========
    auto denoiseTab = new QWidget();
    auto denoiseLayout = new QVBoxLayout(denoiseTab);

    // Noise Reduction parameter (0.01 - 97 dB)
    auto nrGroup = new QGroupBox("Noise Reduction (dB)");
    auto nrLayout = new QVBoxLayout(nrGroup);

    noiseReductionSpinBox = new QDoubleSpinBox();
    noiseReductionSpinBox->setMinimum(0.01);
    noiseReductionSpinBox->setMaximum(97.0);
    noiseReductionSpinBox->setValue(m_noiseReduction);
    noiseReductionSpinBox->setDecimals(1);
    noiseReductionSpinBox->setSingleStep(1);
    noiseReductionSpinBox->setSuffix(" dB");

    auto nrControlLayout = new QHBoxLayout();
    nrControlLayout->addWidget(noiseReductionSpinBox);
    nrControlLayout->addStretch();
    nrControlLayout->addWidget(new QLabel("0"));

    noiseReductionSlider = new QSlider(Qt::Horizontal);
    noiseReductionSlider->setMinimum(1);
    noiseReductionSlider->setMaximum(970);
    noiseReductionSlider->setValue(static_cast<int>(m_noiseReduction * 10));
    nrControlLayout->addWidget(noiseReductionSlider, 1);
    nrControlLayout->addWidget(new QLabel("97"));

    nrLayout->addLayout(nrControlLayout);
    denoiseLayout->addWidget(nrGroup);

    // Noise Floor parameter (-80 - -20 dB)
    auto nfGroup = new QGroupBox("Noise Floor (dB)");
    auto nfLayout = new QVBoxLayout(nfGroup);

    noiseFloorSpinBox = new QDoubleSpinBox();
    noiseFloorSpinBox->setMinimum(-80.0);
    noiseFloorSpinBox->setMaximum(-20.0);
    noiseFloorSpinBox->setValue(m_noiseFloor);
    noiseFloorSpinBox->setDecimals(1);
    noiseFloorSpinBox->setSingleStep(1);
    noiseFloorSpinBox->setSuffix(" dB");

    auto nfControlLayout = new QHBoxLayout();
    nfControlLayout->addWidget(noiseFloorSpinBox);
    nfControlLayout->addStretch();
    nfControlLayout->addWidget(new QLabel("-80"));

    noiseFloorSlider = new QSlider(Qt::Horizontal);
    noiseFloorSlider->setMinimum(-800);
    noiseFloorSlider->setMaximum(-200);
    noiseFloorSlider->setValue(static_cast<int>(m_noiseFloor * 10));
    nfControlLayout->addWidget(noiseFloorSlider, 1);
    nfControlLayout->addWidget(new QLabel("-20"));

    nfLayout->addLayout(nfControlLayout);
    denoiseLayout->addWidget(nfGroup);

    // Noise Type combo
    auto ntGroup = new QGroupBox("Noise Type");
    auto ntLayout = new QVBoxLayout(ntGroup);
    noiseTypeCombo = new QComboBox();
    noiseTypeCombo->addItem("White Noise", 0);
    noiseTypeCombo->addItem("Vinyl (Crackle)", 1);
    noiseTypeCombo->addItem("Shellac (78 RPM)", 2);
    noiseTypeCombo->addItem("Custom", 3);
    noiseTypeCombo->setCurrentIndex(m_noiseType);
    ntLayout->addWidget(noiseTypeCombo);
    denoiseLayout->addWidget(ntGroup);

    denoiseLayout->addStretch();
    tabWidget->addTab(denoiseTab, "Denoise");

    // ========== TAB 2: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Residual Floor parameter (-80 - -20 dB)
    auto rfGroup = new QGroupBox("Residual Floor (dB)");
    auto rfLayout = new QVBoxLayout(rfGroup);

    residualFloorSpinBox = new QDoubleSpinBox();
    residualFloorSpinBox->setMinimum(-80.0);
    residualFloorSpinBox->setMaximum(-20.0);
    residualFloorSpinBox->setValue(m_residualFloor);
    residualFloorSpinBox->setDecimals(1);
    residualFloorSpinBox->setSingleStep(1);
    residualFloorSpinBox->setSuffix(" dB");

    auto rfControlLayout = new QHBoxLayout();
    rfControlLayout->addWidget(residualFloorSpinBox);
    rfControlLayout->addStretch();
    rfControlLayout->addWidget(new QLabel("-80"));

    residualFloorSlider = new QSlider(Qt::Horizontal);
    residualFloorSlider->setMinimum(-800);
    residualFloorSlider->setMaximum(-200);
    residualFloorSlider->setValue(static_cast<int>(m_residualFloor * 10));
    rfControlLayout->addWidget(residualFloorSlider, 1);
    rfControlLayout->addWidget(new QLabel("-20"));

    rfLayout->addLayout(rfControlLayout);
    optionsLayout->addWidget(rfGroup);

    // Track Noise checkbox
    auto trackGroup = new QGroupBox("Adaptive Tracking");
    auto trackLayout = new QVBoxLayout(trackGroup);
    trackNoiseCheckBox = new QCheckBox("Track Noise (adaptive noise floor)");
    trackNoiseCheckBox->setChecked(m_trackNoise);
    trackLayout->addWidget(trackNoiseCheckBox);
    trackResidualCheckBox = new QCheckBox("Track Residual (adaptive residual)");
    trackResidualCheckBox->setChecked(m_trackResidual);
    trackLayout->addWidget(trackResidualCheckBox);
    optionsLayout->addWidget(trackGroup);

    // Output Mode combo
    auto omGroup = new QGroupBox("Output Mode");
    auto omLayout = new QVBoxLayout(omGroup);
    outputModeCombo = new QComboBox();
    outputModeCombo->addItem("Output (denoised audio)", 0);
    outputModeCombo->addItem("Input (original audio)", 1);
    outputModeCombo->addItem("Noise (removed noise only)", 2);
    outputModeCombo->setCurrentIndex(m_outputMode);
    omLayout->addWidget(outputModeCombo);
    optionsLayout->addWidget(omGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Noise Reduction
    connect(noiseReductionSlider, &QSlider::valueChanged, [this](int value) {
        m_noiseReduction = value / 10.0;
        noiseReductionSpinBox->blockSignals(true);
        noiseReductionSpinBox->setValue(m_noiseReduction);
        noiseReductionSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(noiseReductionSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_noiseReduction = value;
        noiseReductionSlider->blockSignals(true);
        noiseReductionSlider->setValue(static_cast<int>(value * 10));
        noiseReductionSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Noise Floor
    connect(noiseFloorSlider, &QSlider::valueChanged, [this](int value) {
        m_noiseFloor = value / 10.0;
        noiseFloorSpinBox->blockSignals(true);
        noiseFloorSpinBox->setValue(m_noiseFloor);
        noiseFloorSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(noiseFloorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_noiseFloor = value;
        noiseFloorSlider->blockSignals(true);
        noiseFloorSlider->setValue(static_cast<int>(value * 10));
        noiseFloorSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Noise Type
    connect(noiseTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_noiseType = index;
        updateFFmpegFlags();
    });

    // Residual Floor
    connect(residualFloorSlider, &QSlider::valueChanged, [this](int value) {
        m_residualFloor = value / 10.0;
        residualFloorSpinBox->blockSignals(true);
        residualFloorSpinBox->setValue(m_residualFloor);
        residualFloorSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(residualFloorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_residualFloor = value;
        residualFloorSlider->blockSignals(true);
        residualFloorSlider->setValue(static_cast<int>(value * 10));
        residualFloorSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Track Noise
    connect(trackNoiseCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_trackNoise = checked;
        updateFFmpegFlags();
    });

    // Track Residual
    connect(trackResidualCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_trackResidual = checked;
        updateFFmpegFlags();
    });

    // Output Mode
    connect(outputModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_outputMode = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAfftdn::updateFFmpegFlags() {
    QStringList noiseTypeNames = {"white", "vinyl", "shellac", "custom"};
    QStringList outputModeNames = {"output", "input", "noise"};
    
    QString ntName = (m_noiseType >= 0 && m_noiseType < noiseTypeNames.size()) ? noiseTypeNames[m_noiseType] : "white";
    QString omName = (m_outputMode >= 0 && m_outputMode < outputModeNames.size()) ? outputModeNames[m_outputMode] : "output";
    
    ffmpegFlags = QString("afftdn=nr=%1:nf=%2:nt=%3:rf=%4:tn=%5:tr=%6:om=%7")
                      .arg(m_noiseReduction, 0, 'f', 1)
                      .arg(m_noiseFloor, 0, 'f', 1)
                      .arg(ntName)
                      .arg(m_residualFloor, 0, 'f', 1)
                      .arg(m_trackNoise ? "true" : "false")
                      .arg(m_trackResidual ? "true" : "false")
                      .arg(omName);
}

QString FFAfftdn::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAfftdn::toJSON(QJsonObject& json) const {
    json["type"] = "ff-afftdn";
    json["noise_reduction"] = m_noiseReduction;
    json["noise_floor"] = m_noiseFloor;
    json["noise_type"] = m_noiseType;
    json["residual_floor"] = m_residualFloor;
    json["track_noise"] = m_trackNoise;
    json["track_residual"] = m_trackResidual;
    json["output_mode"] = m_outputMode;
}

void FFAfftdn::fromJSON(const QJsonObject& json) {
    m_noiseReduction = json["noise_reduction"].toDouble(12.0);
    m_noiseFloor = json["noise_floor"].toDouble(-50.0);
    m_noiseType = json["noise_type"].toInt(0);
    m_residualFloor = json["residual_floor"].toDouble(-38.0);
    m_trackNoise = json["track_noise"].toBool(false);
    m_trackResidual = json["track_residual"].toBool(false);
    m_outputMode = json["output_mode"].toInt(0);
    updateFFmpegFlags();
}
