#include "ff-acrusher.h"
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
#include <cmath>

FFAcrusher::FFAcrusher() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcrusher::getParametersWidget() {
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

    // ========== TAB 1: CRUSHER ==========
    auto crusherTab = new QWidget();
    auto crusherLayout = new QVBoxLayout(crusherTab);

    // Bits parameter (1 - 64)
    auto bitsGroup = new QGroupBox("Bit Depth");
    auto bitsGroupLayout = new QVBoxLayout(bitsGroup);

    bitsSpinBox = new QDoubleSpinBox();
    bitsSpinBox->setMinimum(1.0);
    bitsSpinBox->setMaximum(64.0);
    bitsSpinBox->setValue(m_bits);
    bitsSpinBox->setDecimals(1);
    bitsSpinBox->setSingleStep(1);
    bitsSpinBox->setSuffix(" bits");

    auto bitsControlLayout = new QHBoxLayout();
    bitsControlLayout->addWidget(bitsSpinBox);
    bitsControlLayout->addStretch();
    bitsControlLayout->addWidget(new QLabel("1"));

    bitsSlider = new QSlider(Qt::Horizontal);
    bitsSlider->setMinimum(10);
    bitsSlider->setMaximum(640);
    bitsSlider->setValue(static_cast<int>(m_bits * 10));
    bitsControlLayout->addWidget(bitsSlider, 1);
    bitsControlLayout->addWidget(new QLabel("64"));

    bitsGroupLayout->addLayout(bitsControlLayout);
    crusherLayout->addWidget(bitsGroup);

    // Samples parameter (1 - 250, sample rate reduction)
    auto samplesGroup = new QGroupBox("Sample Reduction");
    auto samplesGroupLayout = new QVBoxLayout(samplesGroup);

    samplesSpinBox = new QDoubleSpinBox();
    samplesSpinBox->setMinimum(1.0);
    samplesSpinBox->setMaximum(250.0);
    samplesSpinBox->setValue(m_samples);
    samplesSpinBox->setDecimals(1);
    samplesSpinBox->setSingleStep(1);

    auto samplesControlLayout = new QHBoxLayout();
    samplesControlLayout->addWidget(samplesSpinBox);
    samplesControlLayout->addStretch();
    samplesControlLayout->addWidget(new QLabel("1"));

    samplesSlider = new QSlider(Qt::Horizontal);
    samplesSlider->setMinimum(10);
    samplesSlider->setMaximum(2500);
    samplesSlider->setValue(static_cast<int>(m_samples * 10));
    samplesControlLayout->addWidget(samplesSlider, 1);
    samplesControlLayout->addWidget(new QLabel("250"));

    samplesGroupLayout->addLayout(samplesControlLayout);
    crusherLayout->addWidget(samplesGroup);

    // Mix parameter (0 - 100%)
    auto mixGroup = new QGroupBox("Wet/Dry Mix");
    auto mixGroupLayout = new QVBoxLayout(mixGroup);

    mixSpinBox = new QDoubleSpinBox();
    mixSpinBox->setMinimum(0.0);
    mixSpinBox->setMaximum(100.0);
    mixSpinBox->setValue(m_mix * 100);
    mixSpinBox->setDecimals(0);
    mixSpinBox->setSingleStep(1);
    mixSpinBox->setSuffix("%");

    auto mixControlLayout = new QHBoxLayout();
    mixControlLayout->addWidget(mixSpinBox);
    mixControlLayout->addStretch();
    mixControlLayout->addWidget(new QLabel("0%"));

    mixSlider = new QSlider(Qt::Horizontal);
    mixSlider->setMinimum(0);
    mixSlider->setMaximum(100);
    mixSlider->setValue(static_cast<int>(m_mix * 100));
    mixControlLayout->addWidget(mixSlider, 1);
    mixControlLayout->addWidget(new QLabel("100%"));

    mixGroupLayout->addLayout(mixControlLayout);
    crusherLayout->addWidget(mixGroup);

    // Mode combo
    auto modeGroup = new QGroupBox("Mode");
    auto modeGroupLayout = new QVBoxLayout(modeGroup);
    modeCombo = new QComboBox();
    modeCombo->addItem("Linear", 0);
    modeCombo->addItem("Logarithmic", 1);
    modeCombo->setCurrentIndex(m_mode);
    modeGroupLayout->addWidget(modeCombo);
    crusherLayout->addWidget(modeGroup);

    crusherLayout->addStretch();
    tabWidget->addTab(crusherTab, "Crusher");

    // ========== TAB 2: LEVELS ==========
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

    // DC parameter (0.25 - 4)
    auto dcGroup = new QGroupBox("DC Offset");
    auto dcGroupLayout = new QVBoxLayout(dcGroup);

    dcSpinBox = new QDoubleSpinBox();
    dcSpinBox->setMinimum(0.25);
    dcSpinBox->setMaximum(4.0);
    dcSpinBox->setValue(m_dc);
    dcSpinBox->setDecimals(2);
    dcSpinBox->setSingleStep(0.1);

    auto dcControlLayout = new QHBoxLayout();
    dcControlLayout->addWidget(dcSpinBox);
    dcControlLayout->addStretch();
    dcControlLayout->addWidget(new QLabel("0.25"));

    dcSlider = new QSlider(Qt::Horizontal);
    dcSlider->setMinimum(25);
    dcSlider->setMaximum(400);
    dcSlider->setValue(static_cast<int>(m_dc * 100));
    dcControlLayout->addWidget(dcSlider, 1);
    dcControlLayout->addWidget(new QLabel("4.0"));

    dcGroupLayout->addLayout(dcControlLayout);
    levelsLayout->addWidget(dcGroup);

    // Anti-aliasing parameter (0 - 1)
    auto aaGroup = new QGroupBox("Anti-Aliasing");
    auto aaGroupLayout = new QVBoxLayout(aaGroup);

    aaSpinBox = new QDoubleSpinBox();
    aaSpinBox->setMinimum(0.0);
    aaSpinBox->setMaximum(100.0);
    aaSpinBox->setValue(m_aa * 100);
    aaSpinBox->setDecimals(0);
    aaSpinBox->setSingleStep(1);
    aaSpinBox->setSuffix("%");

    auto aaControlLayout = new QHBoxLayout();
    aaControlLayout->addWidget(aaSpinBox);
    aaControlLayout->addStretch();
    aaControlLayout->addWidget(new QLabel("0%"));

    aaSlider = new QSlider(Qt::Horizontal);
    aaSlider->setMinimum(0);
    aaSlider->setMaximum(100);
    aaSlider->setValue(static_cast<int>(m_aa * 100));
    aaControlLayout->addWidget(aaSlider, 1);
    aaControlLayout->addWidget(new QLabel("100%"));

    aaGroupLayout->addLayout(aaControlLayout);
    levelsLayout->addWidget(aaGroup);

    levelsLayout->addStretch();
    tabWidget->addTab(levelsTab, "Levels");

    // ========== TAB 3: LFO ==========
    auto lfoTab = new QWidget();
    auto lfoLayout = new QVBoxLayout(lfoTab);

    // LFO Enable checkbox
    auto lfoEnableGroup = new QGroupBox("LFO Modulation");
    auto lfoEnableLayout = new QVBoxLayout(lfoEnableGroup);
    lfoCheckBox = new QCheckBox("Enable LFO");
    lfoCheckBox->setChecked(m_lfo);
    lfoEnableLayout->addWidget(lfoCheckBox);
    lfoLayout->addWidget(lfoEnableGroup);

    // LFO Rate parameter (0.01 - 200 Hz)
    auto lforateGroup = new QGroupBox("LFO Rate (Hz)");
    auto lforateGroupLayout = new QVBoxLayout(lforateGroup);

    lforateSpinBox = new QDoubleSpinBox();
    lforateSpinBox->setMinimum(0.01);
    lforateSpinBox->setMaximum(200.0);
    lforateSpinBox->setValue(m_lforate);
    lforateSpinBox->setDecimals(2);
    lforateSpinBox->setSingleStep(0.1);
    lforateSpinBox->setSuffix(" Hz");

    auto lforateControlLayout = new QHBoxLayout();
    lforateControlLayout->addWidget(lforateSpinBox);
    lforateControlLayout->addStretch();
    lforateControlLayout->addWidget(new QLabel("0.01"));

    lforateSlider = new QSlider(Qt::Horizontal);
    lforateSlider->setMinimum(1);
    lforateSlider->setMaximum(20000);
    lforateSlider->setValue(static_cast<int>(m_lforate * 100));
    lforateControlLayout->addWidget(lforateSlider, 1);
    lforateControlLayout->addWidget(new QLabel("200"));

    lforateGroupLayout->addLayout(lforateControlLayout);
    lfoLayout->addWidget(lforateGroup);

    // LFO Range parameter (1 - 250)
    auto lforangeGroup = new QGroupBox("LFO Depth");
    auto lforangeGroupLayout = new QVBoxLayout(lforangeGroup);

    lforangeSpinBox = new QDoubleSpinBox();
    lforangeSpinBox->setMinimum(1.0);
    lforangeSpinBox->setMaximum(250.0);
    lforangeSpinBox->setValue(m_lforange);
    lforangeSpinBox->setDecimals(1);
    lforangeSpinBox->setSingleStep(1);

    auto lforangeControlLayout = new QHBoxLayout();
    lforangeControlLayout->addWidget(lforangeSpinBox);
    lforangeControlLayout->addStretch();
    lforangeControlLayout->addWidget(new QLabel("1"));

    lforangeSlider = new QSlider(Qt::Horizontal);
    lforangeSlider->setMinimum(10);
    lforangeSlider->setMaximum(2500);
    lforangeSlider->setValue(static_cast<int>(m_lforange * 10));
    lforangeControlLayout->addWidget(lforangeSlider, 1);
    lforangeControlLayout->addWidget(new QLabel("250"));

    lforangeGroupLayout->addLayout(lforangeControlLayout);
    lfoLayout->addWidget(lforangeGroup);

    lfoLayout->addStretch();
    tabWidget->addTab(lfoTab, "LFO");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Bits
    connect(bitsSlider, &QSlider::valueChanged, [this](int value) {
        m_bits = value / 10.0;
        bitsSpinBox->blockSignals(true);
        bitsSpinBox->setValue(m_bits);
        bitsSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bitsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_bits = value;
        bitsSlider->blockSignals(true);
        bitsSlider->setValue(static_cast<int>(value * 10));
        bitsSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Samples
    connect(samplesSlider, &QSlider::valueChanged, [this](int value) {
        m_samples = value / 10.0;
        samplesSpinBox->blockSignals(true);
        samplesSpinBox->setValue(m_samples);
        samplesSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(samplesSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_samples = value;
        samplesSlider->blockSignals(true);
        samplesSlider->setValue(static_cast<int>(value * 10));
        samplesSlider->blockSignals(false);
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

    // Mode
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = index;
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

    // DC
    connect(dcSlider, &QSlider::valueChanged, [this](int value) {
        m_dc = value / 100.0;
        dcSpinBox->blockSignals(true);
        dcSpinBox->setValue(m_dc);
        dcSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(dcSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_dc = value;
        dcSlider->blockSignals(true);
        dcSlider->setValue(static_cast<int>(value * 100));
        dcSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Anti-aliasing
    connect(aaSlider, &QSlider::valueChanged, [this](int value) {
        m_aa = value / 100.0;
        aaSpinBox->blockSignals(true);
        aaSpinBox->setValue(value);
        aaSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(aaSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_aa = value / 100.0;
        aaSlider->blockSignals(true);
        aaSlider->setValue(static_cast<int>(value));
        aaSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // LFO Enable
    connect(lfoCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_lfo = checked;
        updateFFmpegFlags();
    });

    // LFO Rate
    connect(lforateSlider, &QSlider::valueChanged, [this](int value) {
        m_lforate = value / 100.0;
        lforateSpinBox->blockSignals(true);
        lforateSpinBox->setValue(m_lforate);
        lforateSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(lforateSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_lforate = value;
        lforateSlider->blockSignals(true);
        lforateSlider->setValue(static_cast<int>(value * 100));
        lforateSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // LFO Range
    connect(lforangeSlider, &QSlider::valueChanged, [this](int value) {
        m_lforange = value / 10.0;
        lforangeSpinBox->blockSignals(true);
        lforangeSpinBox->setValue(m_lforange);
        lforangeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(lforangeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_lforange = value;
        lforangeSlider->blockSignals(true);
        lforangeSlider->setValue(static_cast<int>(value * 10));
        lforangeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAcrusher::updateFFmpegFlags() {
    QStringList modeNames = {"lin", "log"};
    QString modeName = (m_mode >= 0 && m_mode < modeNames.size()) ? modeNames[m_mode] : "lin";
    
    ffmpegFlags = QString("acrusher=level_in=%1:level_out=%2:bits=%3:mix=%4:mode=%5:dc=%6:aa=%7:samples=%8:lfo=%9:lforange=%10:lforate=%11")
                      .arg(m_levelIn, 0, 'f', 4)
                      .arg(m_levelOut, 0, 'f', 4)
                      .arg(m_bits, 0, 'f', 1)
                      .arg(m_mix, 0, 'f', 2)
                      .arg(modeName)
                      .arg(m_dc, 0, 'f', 2)
                      .arg(m_aa, 0, 'f', 2)
                      .arg(m_samples, 0, 'f', 1)
                      .arg(m_lfo ? "true" : "false")
                      .arg(m_lforange, 0, 'f', 1)
                      .arg(m_lforate, 0, 'f', 2);
}

QString FFAcrusher::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcrusher::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acrusher";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["bits"] = m_bits;
    json["mix"] = m_mix;
    json["mode"] = m_mode;
    json["dc"] = m_dc;
    json["aa"] = m_aa;
    json["samples"] = m_samples;
    json["lfo"] = m_lfo;
    json["lforange"] = m_lforange;
    json["lforate"] = m_lforate;
}

void FFAcrusher::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_bits = json["bits"].toDouble(8.0);
    m_mix = json["mix"].toDouble(0.5);
    m_mode = json["mode"].toInt(0);
    m_dc = json["dc"].toDouble(1.0);
    m_aa = json["aa"].toDouble(0.5);
    m_samples = json["samples"].toDouble(1.0);
    m_lfo = json["lfo"].toBool(false);
    m_lforange = json["lforange"].toDouble(20.0);
    m_lforate = json["lforate"].toDouble(0.3);
    updateFFmpegFlags();
}
