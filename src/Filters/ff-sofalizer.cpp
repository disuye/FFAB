#include "ff-sofalizer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>

FFSofalizer::FFSofalizer() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFSofalizer::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // SOFA file (required)
    auto fileGroup = new QGroupBox("SOFA File (Required)");
    auto fileLayout = new QVBoxLayout(fileGroup);

    auto fileInputLayout = new QHBoxLayout();
    sofaEdit = new QLineEdit();
    sofaEdit->setPlaceholderText("/path/to/hrtf.sofa");
    sofaEdit->setText(m_sofa);
    fileInputLayout->addWidget(sofaEdit, 1);
    
    auto browseBtn = new QPushButton("Browse...");
    fileInputLayout->addWidget(browseBtn);
    fileLayout->addLayout(fileInputLayout);

    auto fileHint = new QLabel(
        "SOFA (Spatially Oriented Format for Acoustics) file containing HRTF data. "
        "Free HRTFs available from sofacoustics.org and other sources."
    );
    fileHint->setStyleSheet("color: gray; font-size: 11px;");
    fileHint->setWordWrap(true);
    fileLayout->addWidget(fileHint);

    mainLayout->addWidget(fileGroup);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: POSITION ==========
    auto positionTab = new QWidget();
    auto positionLayout = new QVBoxLayout(positionTab);

    // Rotation
    auto rotationGroup = new QGroupBox("Rotation (degrees)");
    auto rotationLayout = new QVBoxLayout(rotationGroup);

    rotationSpinBox = new QDoubleSpinBox();
    rotationSpinBox->setMinimum(-360.0);
    rotationSpinBox->setMaximum(360.0);
    rotationSpinBox->setValue(m_rotation);
    rotationSpinBox->setDecimals(1);
    rotationSpinBox->setSuffix("°");

    auto rotationControlLayout = new QHBoxLayout();
    rotationControlLayout->addWidget(rotationSpinBox);
    rotationControlLayout->addWidget(new QLabel("-360"));
    rotationSlider = new QSlider(Qt::Horizontal);
    rotationSlider->setMinimum(-3600);
    rotationSlider->setMaximum(3600);
    rotationSlider->setValue(static_cast<int>(m_rotation * 10));
    rotationControlLayout->addWidget(rotationSlider, 1);
    rotationControlLayout->addWidget(new QLabel("+360"));
    rotationLayout->addLayout(rotationControlLayout);
    rotationLayout->addWidget(new QLabel("Rotate the entire soundfield horizontally."));

    positionLayout->addWidget(rotationGroup);

    // Elevation
    auto elevationGroup = new QGroupBox("Elevation (degrees)");
    auto elevationLayout = new QVBoxLayout(elevationGroup);

    elevationSpinBox = new QDoubleSpinBox();
    elevationSpinBox->setMinimum(-90.0);
    elevationSpinBox->setMaximum(90.0);
    elevationSpinBox->setValue(m_elevation);
    elevationSpinBox->setDecimals(1);
    elevationSpinBox->setSuffix("°");

    auto elevationControlLayout = new QHBoxLayout();
    elevationControlLayout->addWidget(elevationSpinBox);
    elevationControlLayout->addWidget(new QLabel("-90"));
    elevationSlider = new QSlider(Qt::Horizontal);
    elevationSlider->setMinimum(-900);
    elevationSlider->setMaximum(900);
    elevationSlider->setValue(static_cast<int>(m_elevation * 10));
    elevationControlLayout->addWidget(elevationSlider, 1);
    elevationControlLayout->addWidget(new QLabel("+90"));
    elevationLayout->addLayout(elevationControlLayout);
    elevationLayout->addWidget(new QLabel("Tilt the soundfield vertically."));

    positionLayout->addWidget(elevationGroup);

    // Radius
    auto radiusGroup = new QGroupBox("Radius");
    auto radiusLayout = new QVBoxLayout(radiusGroup);

    radiusSpinBox = new QDoubleSpinBox();
    radiusSpinBox->setMinimum(0.0);
    radiusSpinBox->setMaximum(5.0);
    radiusSpinBox->setValue(m_radius);
    radiusSpinBox->setDecimals(2);

    auto radiusControlLayout = new QHBoxLayout();
    radiusControlLayout->addWidget(radiusSpinBox);
    radiusControlLayout->addWidget(new QLabel("0"));
    radiusSlider = new QSlider(Qt::Horizontal);
    radiusSlider->setMinimum(0);
    radiusSlider->setMaximum(500);
    radiusSlider->setValue(static_cast<int>(m_radius * 100));
    radiusControlLayout->addWidget(radiusSlider, 1);
    radiusControlLayout->addWidget(new QLabel("5"));
    radiusLayout->addLayout(radiusControlLayout);
    radiusLayout->addWidget(new QLabel("Perceived distance of sound sources."));

    positionLayout->addWidget(radiusGroup);

    positionLayout->addStretch();
    tabWidget->addTab(positionTab, "Position");

    // ========== TAB 2: GAIN ==========
    auto gainTab = new QWidget();
    auto gainLayout = new QVBoxLayout(gainTab);

    // Main gain
    auto mainGainGroup = new QGroupBox("Main Gain (dB)");
    auto mainGainLayout = new QVBoxLayout(mainGainGroup);

    gainSpinBox = new QDoubleSpinBox();
    gainSpinBox->setMinimum(-20.0);
    gainSpinBox->setMaximum(40.0);
    gainSpinBox->setValue(m_gain);
    gainSpinBox->setDecimals(1);
    gainSpinBox->setSuffix(" dB");

    auto gainControlLayout = new QHBoxLayout();
    gainControlLayout->addWidget(gainSpinBox);
    gainControlLayout->addWidget(new QLabel("-20"));
    gainSlider = new QSlider(Qt::Horizontal);
    gainSlider->setMinimum(-200);
    gainSlider->setMaximum(400);
    gainSlider->setValue(static_cast<int>(m_gain * 10));
    gainControlLayout->addWidget(gainSlider, 1);
    gainControlLayout->addWidget(new QLabel("+40"));
    mainGainLayout->addLayout(gainControlLayout);

    gainLayout->addWidget(mainGainGroup);

    // LFE gain
    auto lfeGainGroup = new QGroupBox("LFE Gain (dB)");
    auto lfeGainLayout = new QVBoxLayout(lfeGainGroup);

    lfeGainSpinBox = new QDoubleSpinBox();
    lfeGainSpinBox->setMinimum(-20.0);
    lfeGainSpinBox->setMaximum(40.0);
    lfeGainSpinBox->setValue(m_lfeGain);
    lfeGainSpinBox->setDecimals(1);
    lfeGainSpinBox->setSuffix(" dB");

    auto lfeGainControlLayout = new QHBoxLayout();
    lfeGainControlLayout->addWidget(lfeGainSpinBox);
    lfeGainControlLayout->addWidget(new QLabel("-20"));
    lfeGainSlider = new QSlider(Qt::Horizontal);
    lfeGainSlider->setMinimum(-200);
    lfeGainSlider->setMaximum(400);
    lfeGainSlider->setValue(static_cast<int>(m_lfeGain * 10));
    lfeGainControlLayout->addWidget(lfeGainSlider, 1);
    lfeGainControlLayout->addWidget(new QLabel("+40"));
    lfeGainLayout->addLayout(lfeGainControlLayout);

    gainLayout->addWidget(lfeGainGroup);

    gainLayout->addStretch();
    tabWidget->addTab(gainTab, "Gain");

    // ========== TAB 3: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Processing type
    auto typeGroup = new QGroupBox("Processing Type");
    auto typeLayout = new QVBoxLayout(typeGroup);

    typeCombo = new QComboBox();
    typeCombo->addItem("Frequency domain (faster)", 0);
    typeCombo->addItem("Time domain (more accurate)", 1);
    typeCombo->setCurrentIndex(m_type);
    typeLayout->addWidget(typeCombo);

    optionsLayout->addWidget(typeGroup);

    // Frame size
    auto framesizeGroup = new QGroupBox("Frame Size");
    auto framesizeLayout = new QVBoxLayout(framesizeGroup);

    framesizeSpinBox = new QSpinBox();
    framesizeSpinBox->setMinimum(1024);
    framesizeSpinBox->setMaximum(96000);
    framesizeSpinBox->setValue(m_framesize);
    framesizeSpinBox->setSingleStep(1024);
    framesizeLayout->addWidget(framesizeSpinBox);
    framesizeLayout->addWidget(new QLabel("Larger = better quality but more latency."));

    optionsLayout->addWidget(framesizeGroup);

    // Checkboxes
    auto flagsGroup = new QGroupBox("Processing Options");
    auto flagsLayout = new QVBoxLayout(flagsGroup);

    normalizeCheck = new QCheckBox("Normalize impulse responses");
    normalizeCheck->setChecked(m_normalize);
    flagsLayout->addWidget(normalizeCheck);

    interpolateCheck = new QCheckBox("Interpolate between neighbor IRs");
    interpolateCheck->setChecked(m_interpolate);
    flagsLayout->addWidget(interpolateCheck);

    minphaseCheck = new QCheckBox("Use minimum phase IRs");
    minphaseCheck->setChecked(m_minphase);
    flagsLayout->addWidget(minphaseCheck);

    optionsLayout->addWidget(flagsGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // File browse
    connect(browseBtn, &QPushButton::clicked, [this]() {
        QString file = QFileDialog::getOpenFileName(
            parametersWidget, "Select SOFA File", QString(), "SOFA Files (*.sofa);;All Files (*)");
        if (!file.isEmpty()) {
            sofaEdit->setText(file);
        }
    });

    connect(sofaEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_sofa = text;
        updateFFmpegFlags();
    });

    // Rotation
    connect(rotationSlider, &QSlider::valueChanged, [this](int value) {
        m_rotation = value / 10.0;
        rotationSpinBox->blockSignals(true);
        rotationSpinBox->setValue(m_rotation);
        rotationSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rotationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_rotation = value;
        rotationSlider->blockSignals(true);
        rotationSlider->setValue(static_cast<int>(value * 10));
        rotationSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Elevation
    connect(elevationSlider, &QSlider::valueChanged, [this](int value) {
        m_elevation = value / 10.0;
        elevationSpinBox->blockSignals(true);
        elevationSpinBox->setValue(m_elevation);
        elevationSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(elevationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_elevation = value;
        elevationSlider->blockSignals(true);
        elevationSlider->setValue(static_cast<int>(value * 10));
        elevationSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Radius
    connect(radiusSlider, &QSlider::valueChanged, [this](int value) {
        m_radius = value / 100.0;
        radiusSpinBox->blockSignals(true);
        radiusSpinBox->setValue(m_radius);
        radiusSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(radiusSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_radius = value;
        radiusSlider->blockSignals(true);
        radiusSlider->setValue(static_cast<int>(value * 100));
        radiusSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Gains
    connect(gainSlider, &QSlider::valueChanged, [this](int value) {
        m_gain = value / 10.0;
        gainSpinBox->blockSignals(true);
        gainSpinBox->setValue(m_gain);
        gainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(gainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_gain = value;
        gainSlider->blockSignals(true);
        gainSlider->setValue(static_cast<int>(value * 10));
        gainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(lfeGainSlider, &QSlider::valueChanged, [this](int value) {
        m_lfeGain = value / 10.0;
        lfeGainSpinBox->blockSignals(true);
        lfeGainSpinBox->setValue(m_lfeGain);
        lfeGainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(lfeGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_lfeGain = value;
        lfeGainSlider->blockSignals(true);
        lfeGainSlider->setValue(static_cast<int>(value * 10));
        lfeGainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Options
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_type = index;
        updateFFmpegFlags();
    });

    connect(framesizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_framesize = value;
        updateFFmpegFlags();
    });

    connect(normalizeCheck, &QCheckBox::toggled, [this](bool checked) {
        m_normalize = checked;
        updateFFmpegFlags();
    });

    connect(interpolateCheck, &QCheckBox::toggled, [this](bool checked) {
        m_interpolate = checked;
        updateFFmpegFlags();
    });

    connect(minphaseCheck, &QCheckBox::toggled, [this](bool checked) {
        m_minphase = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFSofalizer::updateFFmpegFlags() {
    QStringList typeNames = {"freq", "time"};
    QString typeName = (m_type >= 0 && m_type < typeNames.size()) ? typeNames[m_type] : "freq";
    
    if (m_sofa.isEmpty()) {
        ffmpegFlags = "sofalizer";  // Will fail without file, but shows intent
        return;
    }
    
    ffmpegFlags = QString("sofalizer=sofa='%1':gain=%2:rotation=%3:elevation=%4:radius=%5:type=%6:lfegain=%7:framesize=%8:normalize=%9:interpolate=%10:minphase=%11")
                      .arg(m_sofa)
                      .arg(m_gain, 0, 'f', 1)
                      .arg(m_rotation, 0, 'f', 1)
                      .arg(m_elevation, 0, 'f', 1)
                      .arg(m_radius, 0, 'f', 2)
                      .arg(typeName)
                      .arg(m_lfeGain, 0, 'f', 1)
                      .arg(m_framesize)
                      .arg(m_normalize ? "true" : "false")
                      .arg(m_interpolate ? "true" : "false")
                      .arg(m_minphase ? "true" : "false");
}

QString FFSofalizer::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFSofalizer::toJSON(QJsonObject& json) const {
    json["type"] = "ff-sofalizer";
    json["sofa"] = m_sofa;
    json["gain"] = m_gain;
    json["rotation"] = m_rotation;
    json["elevation"] = m_elevation;
    json["radius"] = m_radius;
    json["processing_type"] = m_type;
    json["lfe_gain"] = m_lfeGain;
    json["framesize"] = m_framesize;
    json["normalize"] = m_normalize;
    json["interpolate"] = m_interpolate;
    json["minphase"] = m_minphase;
}

void FFSofalizer::fromJSON(const QJsonObject& json) {
    m_sofa = json["sofa"].toString("");
    m_gain = json["gain"].toDouble(0.0);
    m_rotation = json["rotation"].toDouble(0.0);
    m_elevation = json["elevation"].toDouble(0.0);
    m_radius = json["radius"].toDouble(1.0);
    m_type = json["processing_type"].toInt(0);
    m_lfeGain = json["lfe_gain"].toDouble(0.0);
    m_framesize = json["framesize"].toInt(1024);
    m_normalize = json["normalize"].toBool(true);
    m_interpolate = json["interpolate"].toBool(false);
    m_minphase = json["minphase"].toBool(false);
    updateFFmpegFlags();
}
