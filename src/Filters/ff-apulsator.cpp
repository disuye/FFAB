#include "ff-apulsator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <cmath>

FFApulsator::FFApulsator() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFApulsator::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // dB conversion helpers
    auto linearToDb = [](double linear) -> double {
        return (linear <= 0.0) ? -36.0 : 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: MODULATION ==========
    auto modTab = new QWidget();
    auto modLayout = new QVBoxLayout(modTab);

    // Mode combo
    auto modeGroup = new QGroupBox("Waveform");
    auto modeGroupLayout = new QVBoxLayout(modeGroup);
    modeCombo = new QComboBox();
    modeCombo->addItem("Sine", 0);
    modeCombo->addItem("Triangle", 1);
    modeCombo->addItem("Square", 2);
    modeCombo->addItem("Saw Up", 3);
    modeCombo->addItem("Saw Down", 4);
    modeCombo->setCurrentIndex(m_mode);
    modeGroupLayout->addWidget(modeCombo);
    modLayout->addWidget(modeGroup);

    // Amount parameter (0 - 100%)
    auto amountGroup = new QGroupBox("Modulation Depth");
    auto amountGroupLayout = new QVBoxLayout(amountGroup);

    amountSpinBox = new QDoubleSpinBox();
    amountSpinBox->setMinimum(0.0);
    amountSpinBox->setMaximum(100.0);
    amountSpinBox->setValue(m_amount * 100);
    amountSpinBox->setDecimals(0);
    amountSpinBox->setSingleStep(1);
    amountSpinBox->setSuffix("%");

    auto amountControlLayout = new QHBoxLayout();
    amountControlLayout->addWidget(amountSpinBox);
    amountControlLayout->addStretch();
    amountControlLayout->addWidget(new QLabel("0%"));

    amountSlider = new QSlider(Qt::Horizontal);
    amountSlider->setMinimum(0);
    amountSlider->setMaximum(100);
    amountSlider->setValue(static_cast<int>(m_amount * 100));
    amountControlLayout->addWidget(amountSlider, 1);
    amountControlLayout->addWidget(new QLabel("100%"));

    amountGroupLayout->addLayout(amountControlLayout);
    modLayout->addWidget(amountGroup);

    // Width parameter (0 - 2)
    auto widthGroup = new QGroupBox("Pulse Width");
    auto widthGroupLayout = new QVBoxLayout(widthGroup);

    widthSpinBox = new QDoubleSpinBox();
    widthSpinBox->setMinimum(0.0);
    widthSpinBox->setMaximum(2.0);
    widthSpinBox->setValue(m_width);
    widthSpinBox->setDecimals(2);
    widthSpinBox->setSingleStep(0.1);

    auto widthControlLayout = new QHBoxLayout();
    widthControlLayout->addWidget(widthSpinBox);
    widthControlLayout->addStretch();
    widthControlLayout->addWidget(new QLabel("0"));

    widthSlider = new QSlider(Qt::Horizontal);
    widthSlider->setMinimum(0);
    widthSlider->setMaximum(200);
    widthSlider->setValue(static_cast<int>(m_width * 100));
    widthControlLayout->addWidget(widthSlider, 1);
    widthControlLayout->addWidget(new QLabel("2"));

    widthGroupLayout->addLayout(widthControlLayout);
    modLayout->addWidget(widthGroup);

    modLayout->addStretch();
    tabWidget->addTab(modTab, "Modulation");

    // ========== TAB 2: TIMING ==========
    auto timingTab = new QWidget();
    auto timingLayout = new QVBoxLayout(timingTab);

    // Timing mode combo
    auto timingModeGroup = new QGroupBox("Timing Mode");
    auto timingModeLayout = new QVBoxLayout(timingModeGroup);
    timingCombo = new QComboBox();
    timingCombo->addItem("Hz (Frequency)", 0);
    timingCombo->addItem("BPM (Tempo)", 1);
    timingCombo->addItem("ms (Period)", 2);
    timingCombo->setCurrentIndex(m_timing);
    timingModeLayout->addWidget(timingCombo);
    timingLayout->addWidget(timingModeGroup);

    // Hz parameter (0.01 - 100)
    auto hzGroup = new QGroupBox("Frequency (Hz)");
    auto hzGroupLayout = new QVBoxLayout(hzGroup);

    hzSpinBox = new QDoubleSpinBox();
    hzSpinBox->setMinimum(0.01);
    hzSpinBox->setMaximum(100.0);
    hzSpinBox->setValue(m_hz);
    hzSpinBox->setDecimals(2);
    hzSpinBox->setSingleStep(0.1);
    hzSpinBox->setSuffix(" Hz");

    auto hzControlLayout = new QHBoxLayout();
    hzControlLayout->addWidget(hzSpinBox);
    hzControlLayout->addStretch();
    hzControlLayout->addWidget(new QLabel("0.01"));

    hzSlider = new QSlider(Qt::Horizontal);
    hzSlider->setMinimum(1);
    hzSlider->setMaximum(10000);
    hzSlider->setValue(static_cast<int>(m_hz * 100));
    hzControlLayout->addWidget(hzSlider, 1);
    hzControlLayout->addWidget(new QLabel("100"));

    hzGroupLayout->addLayout(hzControlLayout);
    timingLayout->addWidget(hzGroup);

    // BPM parameter (30 - 300)
    auto bpmGroup = new QGroupBox("BPM");
    auto bpmGroupLayout = new QVBoxLayout(bpmGroup);

    bpmSpinBox = new QDoubleSpinBox();
    bpmSpinBox->setMinimum(30.0);
    bpmSpinBox->setMaximum(300.0);
    bpmSpinBox->setValue(m_bpm);
    bpmSpinBox->setDecimals(1);
    bpmSpinBox->setSingleStep(1);
    bpmSpinBox->setSuffix(" BPM");

    auto bpmControlLayout = new QHBoxLayout();
    bpmControlLayout->addWidget(bpmSpinBox);
    bpmControlLayout->addStretch();
    bpmControlLayout->addWidget(new QLabel("30"));

    bpmSlider = new QSlider(Qt::Horizontal);
    bpmSlider->setMinimum(300);
    bpmSlider->setMaximum(3000);
    bpmSlider->setValue(static_cast<int>(m_bpm * 10));
    bpmControlLayout->addWidget(bpmSlider, 1);
    bpmControlLayout->addWidget(new QLabel("300"));

    bpmGroupLayout->addLayout(bpmControlLayout);
    timingLayout->addWidget(bpmGroup);

    // MS parameter (10 - 2000)
    auto msGroup = new QGroupBox("Period (ms)");
    auto msGroupLayout = new QVBoxLayout(msGroup);

    msSpinBox = new QSpinBox();
    msSpinBox->setMinimum(10);
    msSpinBox->setMaximum(2000);
    msSpinBox->setValue(m_ms);
    msSpinBox->setSingleStep(10);
    msSpinBox->setSuffix(" ms");

    auto msControlLayout = new QHBoxLayout();
    msControlLayout->addWidget(msSpinBox);
    msControlLayout->addStretch();
    msControlLayout->addWidget(new QLabel("10"));

    msSlider = new QSlider(Qt::Horizontal);
    msSlider->setMinimum(10);
    msSlider->setMaximum(2000);
    msSlider->setValue(m_ms);
    msControlLayout->addWidget(msSlider, 1);
    msControlLayout->addWidget(new QLabel("2000"));

    msGroupLayout->addLayout(msControlLayout);
    timingLayout->addWidget(msGroup);

    timingLayout->addStretch();
    tabWidget->addTab(timingTab, "Timing");

    // ========== TAB 3: STEREO ==========
    auto stereoTab = new QWidget();
    auto stereoLayout = new QVBoxLayout(stereoTab);

    // Left Offset parameter (0 - 1)
    auto offsetLGroup = new QGroupBox("Left Channel Phase Offset");
    auto offsetLGroupLayout = new QVBoxLayout(offsetLGroup);

    offsetLSpinBox = new QDoubleSpinBox();
    offsetLSpinBox->setMinimum(0.0);
    offsetLSpinBox->setMaximum(100.0);
    offsetLSpinBox->setValue(m_offsetL * 100);
    offsetLSpinBox->setDecimals(0);
    offsetLSpinBox->setSingleStep(1);
    offsetLSpinBox->setSuffix("%");

    auto offsetLControlLayout = new QHBoxLayout();
    offsetLControlLayout->addWidget(offsetLSpinBox);
    offsetLControlLayout->addStretch();
    offsetLControlLayout->addWidget(new QLabel("0%"));

    offsetLSlider = new QSlider(Qt::Horizontal);
    offsetLSlider->setMinimum(0);
    offsetLSlider->setMaximum(100);
    offsetLSlider->setValue(static_cast<int>(m_offsetL * 100));
    offsetLControlLayout->addWidget(offsetLSlider, 1);
    offsetLControlLayout->addWidget(new QLabel("100%"));

    offsetLGroupLayout->addLayout(offsetLControlLayout);
    stereoLayout->addWidget(offsetLGroup);

    // Right Offset parameter (0 - 1)
    auto offsetRGroup = new QGroupBox("Right Channel Phase Offset");
    auto offsetRGroupLayout = new QVBoxLayout(offsetRGroup);

    offsetRSpinBox = new QDoubleSpinBox();
    offsetRSpinBox->setMinimum(0.0);
    offsetRSpinBox->setMaximum(100.0);
    offsetRSpinBox->setValue(m_offsetR * 100);
    offsetRSpinBox->setDecimals(0);
    offsetRSpinBox->setSingleStep(1);
    offsetRSpinBox->setSuffix("%");

    auto offsetRControlLayout = new QHBoxLayout();
    offsetRControlLayout->addWidget(offsetRSpinBox);
    offsetRControlLayout->addStretch();
    offsetRControlLayout->addWidget(new QLabel("0%"));

    offsetRSlider = new QSlider(Qt::Horizontal);
    offsetRSlider->setMinimum(0);
    offsetRSlider->setMaximum(100);
    offsetRSlider->setValue(static_cast<int>(m_offsetR * 100));
    offsetRControlLayout->addWidget(offsetRSlider, 1);
    offsetRControlLayout->addWidget(new QLabel("100%"));

    offsetRGroupLayout->addLayout(offsetRControlLayout);
    stereoLayout->addWidget(offsetRGroup);

    auto stereoHintLabel = new QLabel("50% offset between L/R creates stereo panning effect");
    stereoHintLabel->setStyleSheet("color: gray; font-size: 11px;");
    stereoLayout->addWidget(stereoHintLabel);

    stereoLayout->addStretch();
    tabWidget->addTab(stereoTab, "Stereo");

    // ========== TAB 4: LEVELS ==========
    auto levelsTab = new QWidget();
    auto levelsLayout = new QVBoxLayout(levelsTab);

    // Input Level (dB)
    auto levelInGroup = new QGroupBox("Input Level (dB)");
    auto levelInGroupLayout = new QVBoxLayout(levelInGroup);

    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(-36.0);
    levelInSpinBox->setMaximum(36.0);
    levelInSpinBox->setValue(linearToDb(m_levelIn));
    levelInSpinBox->setDecimals(1);
    levelInSpinBox->setSingleStep(0.5);
    levelInSpinBox->setSuffix(" dB");

    auto levelInControlLayout = new QHBoxLayout();
    levelInControlLayout->addWidget(levelInSpinBox);
    levelInControlLayout->addStretch();
    levelInControlLayout->addWidget(new QLabel("-36"));

    levelInSlider = new QSlider(Qt::Horizontal);
    levelInSlider->setMinimum(-3600);
    levelInSlider->setMaximum(3600);
    levelInSlider->setValue(static_cast<int>(linearToDb(m_levelIn) * 100));
    levelInControlLayout->addWidget(levelInSlider, 1);
    levelInControlLayout->addWidget(new QLabel("+36"));

    levelInGroupLayout->addLayout(levelInControlLayout);
    levelsLayout->addWidget(levelInGroup);

    // Output Level (dB)
    auto levelOutGroup = new QGroupBox("Output Level (dB)");
    auto levelOutGroupLayout = new QVBoxLayout(levelOutGroup);

    levelOutSpinBox = new QDoubleSpinBox();
    levelOutSpinBox->setMinimum(-36.0);
    levelOutSpinBox->setMaximum(36.0);
    levelOutSpinBox->setValue(linearToDb(m_levelOut));
    levelOutSpinBox->setDecimals(1);
    levelOutSpinBox->setSingleStep(0.5);
    levelOutSpinBox->setSuffix(" dB");

    auto levelOutControlLayout = new QHBoxLayout();
    levelOutControlLayout->addWidget(levelOutSpinBox);
    levelOutControlLayout->addStretch();
    levelOutControlLayout->addWidget(new QLabel("-36"));

    levelOutSlider = new QSlider(Qt::Horizontal);
    levelOutSlider->setMinimum(-3600);
    levelOutSlider->setMaximum(3600);
    levelOutSlider->setValue(static_cast<int>(linearToDb(m_levelOut) * 100));
    levelOutControlLayout->addWidget(levelOutSlider, 1);
    levelOutControlLayout->addWidget(new QLabel("+36"));

    levelOutGroupLayout->addLayout(levelOutControlLayout);
    levelsLayout->addWidget(levelOutGroup);

    levelsLayout->addStretch();
    tabWidget->addTab(levelsTab, "Levels");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Mode
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = index;
        updateFFmpegFlags();
    });

    // Amount
    connect(amountSlider, &QSlider::valueChanged, [this](int value) {
        m_amount = value / 100.0;
        amountSpinBox->blockSignals(true);
        amountSpinBox->setValue(value);
        amountSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(amountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_amount = value / 100.0;
        amountSlider->blockSignals(true);
        amountSlider->setValue(static_cast<int>(value));
        amountSlider->blockSignals(false);
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
        widthSlider->setValue(static_cast<int>(value * 100));
        widthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Timing mode
    connect(timingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_timing = index;
        updateFFmpegFlags();
    });

    // Hz
    connect(hzSlider, &QSlider::valueChanged, [this](int value) {
        m_hz = value / 100.0;
        hzSpinBox->blockSignals(true);
        hzSpinBox->setValue(m_hz);
        hzSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(hzSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_hz = value;
        hzSlider->blockSignals(true);
        hzSlider->setValue(static_cast<int>(value * 100));
        hzSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // BPM
    connect(bpmSlider, &QSlider::valueChanged, [this](int value) {
        m_bpm = value / 10.0;
        bpmSpinBox->blockSignals(true);
        bpmSpinBox->setValue(m_bpm);
        bpmSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bpmSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_bpm = value;
        bpmSlider->blockSignals(true);
        bpmSlider->setValue(static_cast<int>(value * 10));
        bpmSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // MS
    connect(msSlider, &QSlider::valueChanged, [this](int value) {
        m_ms = value;
        msSpinBox->blockSignals(true);
        msSpinBox->setValue(m_ms);
        msSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(msSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_ms = value;
        msSlider->blockSignals(true);
        msSlider->setValue(value);
        msSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Offset L
    connect(offsetLSlider, &QSlider::valueChanged, [this](int value) {
        m_offsetL = value / 100.0;
        offsetLSpinBox->blockSignals(true);
        offsetLSpinBox->setValue(value);
        offsetLSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(offsetLSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_offsetL = value / 100.0;
        offsetLSlider->blockSignals(true);
        offsetLSlider->setValue(static_cast<int>(value));
        offsetLSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Offset R
    connect(offsetRSlider, &QSlider::valueChanged, [this](int value) {
        m_offsetR = value / 100.0;
        offsetRSpinBox->blockSignals(true);
        offsetRSpinBox->setValue(value);
        offsetRSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(offsetRSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_offsetR = value / 100.0;
        offsetRSlider->blockSignals(true);
        offsetRSlider->setValue(static_cast<int>(value));
        offsetRSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Input Level (dB conversion)
    connect(levelInSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 100.0;
        m_levelIn = dbToLinear(db);
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(db);
        levelInSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_levelIn = dbToLinear(db);
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(db * 100));
        levelInSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Output Level (dB conversion)
    connect(levelOutSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 100.0;
        m_levelOut = dbToLinear(db);
        levelOutSpinBox->blockSignals(true);
        levelOutSpinBox->setValue(db);
        levelOutSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_levelOut = dbToLinear(db);
        levelOutSlider->blockSignals(true);
        levelOutSlider->setValue(static_cast<int>(db * 100));
        levelOutSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFApulsator::updateFFmpegFlags() {
    QStringList modeNames = {"sine", "triangle", "square", "sawup", "sawdown"};
    QStringList timingNames = {"hz", "bpm", "ms"};
    
    QString modeName = (m_mode >= 0 && m_mode < modeNames.size()) ? modeNames[m_mode] : "sine";
    QString timingName = (m_timing >= 0 && m_timing < timingNames.size()) ? timingNames[m_timing] : "hz";
    
    ffmpegFlags = QString("apulsator=level_in=%1:level_out=%2:mode=%3:amount=%4:offset_l=%5:offset_r=%6:width=%7:timing=%8:bpm=%9:ms=%10:hz=%11")
                      .arg(m_levelIn, 0, 'f', 4)
                      .arg(m_levelOut, 0, 'f', 4)
                      .arg(modeName)
                      .arg(m_amount, 0, 'f', 2)
                      .arg(m_offsetL, 0, 'f', 2)
                      .arg(m_offsetR, 0, 'f', 2)
                      .arg(m_width, 0, 'f', 2)
                      .arg(timingName)
                      .arg(m_bpm, 0, 'f', 1)
                      .arg(m_ms)
                      .arg(m_hz, 0, 'f', 2);
}

QString FFApulsator::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFApulsator::toJSON(QJsonObject& json) const {
    json["type"] = "ff-apulsator";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["mode"] = m_mode;
    json["amount"] = m_amount;
    json["offset_l"] = m_offsetL;
    json["offset_r"] = m_offsetR;
    json["width"] = m_width;
    json["timing"] = m_timing;
    json["bpm"] = m_bpm;
    json["ms"] = m_ms;
    json["hz"] = m_hz;
}

void FFApulsator::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_mode = json["mode"].toInt(0);
    m_amount = json["amount"].toDouble(1.0);
    m_offsetL = json["offset_l"].toDouble(0.0);
    m_offsetR = json["offset_r"].toDouble(0.5);
    m_width = json["width"].toDouble(1.0);
    m_timing = json["timing"].toInt(0);
    m_bpm = json["bpm"].toDouble(120.0);
    m_ms = json["ms"].toInt(500);
    m_hz = json["hz"].toDouble(2.0);
    updateFFmpegFlags();
}
