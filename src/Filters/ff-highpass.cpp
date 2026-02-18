#include "ff-highpass.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <cmath>

FFHighpass::FFHighpass() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFHighpass::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: FILTER ==========
    auto filterTab = new QWidget();
    auto filterLayout = new QVBoxLayout(filterTab);

    // Frequency parameter (Hz) - using log-like slider mapping
    auto frequencyGroup = new QGroupBox("Cutoff Frequency (Hz)");
    auto frequencyLayout = new QVBoxLayout(frequencyGroup);

    frequencySpinBox = new QDoubleSpinBox();
    frequencySpinBox->setMinimum(1.0);
    frequencySpinBox->setMaximum(999999.0);
    frequencySpinBox->setValue(m_frequency);
    frequencySpinBox->setDecimals(2);
    frequencySpinBox->setSingleStep(10.0);
    frequencySpinBox->setSuffix(" Hz");

    auto frequencyControlLayout = new QHBoxLayout();
    frequencyControlLayout->addWidget(frequencySpinBox);
    frequencyControlLayout->addStretch();
    
    auto frequencyMinLabel = new QLabel("20");
    frequencyControlLayout->addWidget(frequencyMinLabel);

    frequencySlider = new QSlider(Qt::Horizontal);
    frequencySlider->setMinimum(0);
    frequencySlider->setMaximum(1000);
    frequencySlider->setValue(freqToSlider(m_frequency, 20.0, 20000.0));
    frequencyControlLayout->addWidget(frequencySlider, 1);

    auto frequencyMaxLabel = new QLabel("20k");
    frequencyControlLayout->addWidget(frequencyMaxLabel);

    frequencyLayout->addLayout(frequencyControlLayout);
    filterLayout->addWidget(frequencyGroup);

    // Width Type parameter
    auto widthTypeGroup = new QGroupBox("Width Type");
    auto widthTypeLayout = new QVBoxLayout(widthTypeGroup);

    widthTypeCombo = new QComboBox();
    widthTypeCombo->addItem("Hz (h)", 1);
    widthTypeCombo->addItem("Q-Factor (q)", 2);
    widthTypeCombo->addItem("Octave (o)", 3);
    widthTypeCombo->addItem("Slope (s)", 4);
    widthTypeCombo->addItem("kHz (k)", 5);
    widthTypeCombo->setCurrentIndex(m_widthType - 1);  // Convert 1-based to 0-based

    widthTypeLayout->addWidget(widthTypeCombo);
    filterLayout->addWidget(widthTypeGroup);

    // Width parameter
    auto widthGroup = new QGroupBox("Width");
    auto widthLayout = new QVBoxLayout(widthGroup);

    widthSpinBox = new QDoubleSpinBox();
    widthSpinBox->setMinimum(0.01);
    widthSpinBox->setMaximum(99999.0);
    widthSpinBox->setValue(m_width);
    widthSpinBox->setDecimals(2);
    widthSpinBox->setSingleStep(0.1);

    auto widthControlLayout = new QHBoxLayout();
    widthControlLayout->addWidget(widthSpinBox);
    widthControlLayout->addStretch();
    
    auto widthMinLabel = new QLabel("0.01");
    widthControlLayout->addWidget(widthMinLabel);

    widthSlider = new QSlider(Qt::Horizontal);
    widthSlider->setMinimum(1);
    widthSlider->setMaximum(1000);
    widthSlider->setValue(static_cast<int>(m_width * 100));
    widthControlLayout->addWidget(widthSlider, 1);

    auto widthMaxLabel = new QLabel("10.0");
    widthControlLayout->addWidget(widthMaxLabel);

    widthLayout->addLayout(widthControlLayout);
    filterLayout->addWidget(widthGroup);

    // Poles parameter
    auto polesGroup = new QGroupBox("Filter Order (Poles)");
    auto polesLayout = new QVBoxLayout(polesGroup);

    polesCombo = new QComboBox();
    polesCombo->addItem("1-pole (6 dB/oct)", 1);
    polesCombo->addItem("2-pole (12 dB/oct)", 2);
    polesCombo->setCurrentIndex(m_poles - 1);

    polesLayout->addWidget(polesCombo);
    filterLayout->addWidget(polesGroup);

    filterLayout->addStretch();
    tabWidget->addTab(filterTab, "Filter");

    // ========== TAB 2: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Wet/Dry Mix parameter (0 - 100%)
    auto mixGroup = new QGroupBox("Wet/Dry Mix");
    auto mixLayout = new QVBoxLayout(mixGroup);

    mixSpinBox = new QDoubleSpinBox();
    mixSpinBox->setMinimum(0.0);
    mixSpinBox->setMaximum(100.0);
    mixSpinBox->setValue(m_mix * 100.0);
    mixSpinBox->setDecimals(2);
    mixSpinBox->setSingleStep(1.0);
    mixSpinBox->setSuffix(" %");

    auto mixControlLayout = new QHBoxLayout();
    mixControlLayout->addWidget(mixSpinBox);
    mixControlLayout->addStretch();
    
    auto mixMinLabel = new QLabel("0%");
    mixControlLayout->addWidget(mixMinLabel);

    mixSlider = new QSlider(Qt::Horizontal);
    mixSlider->setMinimum(0);
    mixSlider->setMaximum(100);
    mixSlider->setValue(static_cast<int>(m_mix * 100));
    mixControlLayout->addWidget(mixSlider, 1);

    auto mixMaxLabel = new QLabel("100%");
    mixControlLayout->addWidget(mixMaxLabel);

    mixLayout->addLayout(mixControlLayout);
    optionsLayout->addWidget(mixGroup);

    // Normalize checkbox
    auto normalizeGroup = new QGroupBox("Normalize Coefficients");
    auto normalizeLayout = new QVBoxLayout(normalizeGroup);

    normalizeCheck = new QCheckBox("Enable normalization");
    normalizeCheck->setChecked(m_normalize);

    normalizeLayout->addWidget(normalizeCheck);
    optionsLayout->addWidget(normalizeGroup);

    // Transform type
    auto transformGroup = new QGroupBox("Transform Type");
    auto transformLayout = new QVBoxLayout(transformGroup);

    transformCombo = new QComboBox();
    transformCombo->addItem("Direct Form I (di)", 0);
    transformCombo->addItem("Direct Form II (dii)", 1);
    transformCombo->addItem("Transposed Direct Form I (tdi)", 2);
    transformCombo->addItem("Transposed Direct Form II (tdii)", 3);
    transformCombo->addItem("Lattice (latt)", 4);
    transformCombo->addItem("State Variable (svf)", 5);
    transformCombo->addItem("Zero Delay Feedback (zdf)", 6);
    transformCombo->setCurrentIndex(m_transform);

    transformLayout->addWidget(transformCombo);
    optionsLayout->addWidget(transformGroup);

    // Precision
    auto precisionGroup = new QGroupBox("Processing Precision");
    auto precisionLayout = new QVBoxLayout(precisionGroup);

    precisionCombo = new QComboBox();
    precisionCombo->addItem("Auto", -1);
    precisionCombo->addItem("16-bit Integer (s16)", 0);
    precisionCombo->addItem("32-bit Integer (s32)", 1);
    precisionCombo->addItem("32-bit Float (f32)", 2);
    precisionCombo->addItem("64-bit Float (f64)", 3);
    precisionCombo->setCurrentIndex(m_precision + 1);  // -1 becomes 0

    precisionLayout->addWidget(precisionCombo);
    optionsLayout->addWidget(precisionGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Frequency
    connect(frequencySlider, &QSlider::valueChanged, [this](int value) {
        m_frequency = sliderToFreq(value, 20.0, 20000.0);
        frequencySpinBox->blockSignals(true);
        frequencySpinBox->setValue(m_frequency);
        frequencySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(frequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_frequency = value;
        frequencySlider->blockSignals(true);
        frequencySlider->setValue(freqToSlider(value, 20.0, 20000.0));
        frequencySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Width Type
    connect(widthTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_widthType = widthTypeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Width
    connect(widthSlider, &QSlider::valueChanged, [this](int value) {
        m_width = value / 100.0;
        widthSpinBox->blockSignals(true);
        widthSpinBox->setValue(m_width);
        widthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_width = value;
        widthSlider->blockSignals(true);
        widthSlider->setValue(static_cast<int>(std::clamp(value * 100.0, 1.0, 1000.0)));
        widthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Poles
    connect(polesCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_poles = polesCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Mix
    connect(mixSlider, &QSlider::valueChanged, [this](int value) {
        m_mix = value / 100.0;
        mixSpinBox->blockSignals(true);
        mixSpinBox->setValue(value);
        mixSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(mixSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_mix = value / 100.0;
        mixSlider->blockSignals(true);
        mixSlider->setValue(static_cast<int>(value));
        mixSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Normalize
    connect(normalizeCheck, &QCheckBox::toggled, [this](bool checked) {
        m_normalize = checked;
        updateFFmpegFlags();
    });

    // Transform
    connect(transformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_transform = transformCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Precision
    connect(precisionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_precision = precisionCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFHighpass::updateFFmpegFlags() {
    QStringList widthTypeNames = {"h", "q", "o", "s", "k"};
    QStringList transformNames = {"di", "dii", "tdi", "tdii", "latt", "svf", "zdf"};
    QStringList precisionNames = {"auto", "s16", "s32", "f32", "f64"};
    
    QString widthTypeName = (m_widthType >= 1 && m_widthType <= 5) 
        ? widthTypeNames[m_widthType - 1] : "q";
    QString transformName = (m_transform >= 0 && m_transform < transformNames.size()) 
        ? transformNames[m_transform] : "di";
    QString precisionName = (m_precision >= -1 && m_precision <= 3) 
        ? precisionNames[m_precision + 1] : "auto";
    
    ffmpegFlags = QString("highpass=frequency=%1:width_type=%2:width=%3:poles=%4:mix=%5:normalize=%6:transform=%7:precision=%8")
        .arg(m_frequency, 0, 'f', 2)
        .arg(widthTypeName)
        .arg(m_width, 0, 'f', 4)
        .arg(m_poles)
        .arg(m_mix, 0, 'f', 2)
        .arg(m_normalize ? "true" : "false")
        .arg(transformName)
        .arg(precisionName);
}

QString FFHighpass::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFHighpass::toJSON(QJsonObject& json) const {
    json["type"] = "ff-highpass";
    json["frequency"] = m_frequency;
    json["width_type"] = m_widthType;
    json["width"] = m_width;
    json["poles"] = m_poles;
    json["mix"] = m_mix;
    json["normalize"] = m_normalize;
    json["transform"] = m_transform;
    json["precision"] = m_precision;
}

void FFHighpass::fromJSON(const QJsonObject& json) {
    m_frequency = json["frequency"].toDouble(3000.0);
    m_widthType = json["width_type"].toInt(2);  // default q
    m_width = json["width"].toDouble(0.707);
    m_poles = json["poles"].toInt(2);
    m_mix = json["mix"].toDouble(1.0);
    m_normalize = json["normalize"].toBool(false);
    m_transform = json["transform"].toInt(0);
    m_precision = json["precision"].toInt(-1);
    updateFFmpegFlags();
}
