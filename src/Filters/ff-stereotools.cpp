#include "ff-stereotools.h"
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

FFStereotools::FFStereotools() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

double FFStereotools::dbToLinear(double db) const {
    return std::pow(10.0, db / 20.0);
}

double FFStereotools::linearToDb(double linear) const {
    if (linear <= 0.0) return -36.0;
    return 20.0 * std::log10(linear);
}

QWidget* FFStereotools::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    auto tabWidget = new QTabWidget();

    // ========== TAB 1: LEVELS ==========
    auto levelsTab = new QWidget();
    auto levelsLayout = new QVBoxLayout(levelsTab);

    // Level In (dB)
    auto levelInGroup = new QGroupBox("Input Level (dB)");
    auto levelInLayout = new QVBoxLayout(levelInGroup);
    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(-36.0);
    levelInSpinBox->setMaximum(36.0);
    levelInSpinBox->setValue(linearToDb(m_levelIn));
    levelInSpinBox->setDecimals(2);
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
    levelInLayout->addLayout(levelInControlLayout);
    levelsLayout->addWidget(levelInGroup);

    // Level Out (dB)
    auto levelOutGroup = new QGroupBox("Output Level (dB)");
    auto levelOutLayout = new QVBoxLayout(levelOutGroup);
    levelOutSpinBox = new QDoubleSpinBox();
    levelOutSpinBox->setMinimum(-36.0);
    levelOutSpinBox->setMaximum(36.0);
    levelOutSpinBox->setValue(linearToDb(m_levelOut));
    levelOutSpinBox->setDecimals(2);
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
    levelOutLayout->addLayout(levelOutControlLayout);
    levelsLayout->addWidget(levelOutGroup);

    // Balance In
    auto balanceInGroup = new QGroupBox("Input Balance");
    auto balanceInLayout = new QVBoxLayout(balanceInGroup);
    balanceInSpinBox = new QDoubleSpinBox();
    balanceInSpinBox->setMinimum(-100.0);
    balanceInSpinBox->setMaximum(100.0);
    balanceInSpinBox->setValue(m_balanceIn * 100.0);
    balanceInSpinBox->setDecimals(2);
    balanceInSpinBox->setSuffix(" %");
    auto balanceInControlLayout = new QHBoxLayout();
    balanceInControlLayout->addWidget(balanceInSpinBox);
    balanceInControlLayout->addStretch();
    balanceInControlLayout->addWidget(new QLabel("L"));
    balanceInSlider = new QSlider(Qt::Horizontal);
    balanceInSlider->setMinimum(-100);
    balanceInSlider->setMaximum(100);
    balanceInSlider->setValue(static_cast<int>(m_balanceIn * 100));
    balanceInControlLayout->addWidget(balanceInSlider, 1);
    balanceInControlLayout->addWidget(new QLabel("R"));
    balanceInLayout->addLayout(balanceInControlLayout);
    bmodeInCombo = new QComboBox();
    bmodeInCombo->addItem("Balance", 0);
    bmodeInCombo->addItem("Amplitude", 1);
    bmodeInCombo->addItem("Power", 2);
    bmodeInCombo->setCurrentIndex(m_bmodeIn);
    balanceInLayout->addWidget(bmodeInCombo);
    levelsLayout->addWidget(balanceInGroup);

    // Balance Out
    auto balanceOutGroup = new QGroupBox("Output Balance");
    auto balanceOutLayout = new QVBoxLayout(balanceOutGroup);
    balanceOutSpinBox = new QDoubleSpinBox();
    balanceOutSpinBox->setMinimum(-100.0);
    balanceOutSpinBox->setMaximum(100.0);
    balanceOutSpinBox->setValue(m_balanceOut * 100.0);
    balanceOutSpinBox->setDecimals(2);
    balanceOutSpinBox->setSuffix(" %");
    auto balanceOutControlLayout = new QHBoxLayout();
    balanceOutControlLayout->addWidget(balanceOutSpinBox);
    balanceOutControlLayout->addStretch();
    balanceOutControlLayout->addWidget(new QLabel("L"));
    balanceOutSlider = new QSlider(Qt::Horizontal);
    balanceOutSlider->setMinimum(-100);
    balanceOutSlider->setMaximum(100);
    balanceOutSlider->setValue(static_cast<int>(m_balanceOut * 100));
    balanceOutControlLayout->addWidget(balanceOutSlider, 1);
    balanceOutControlLayout->addWidget(new QLabel("R"));
    balanceOutLayout->addLayout(balanceOutControlLayout);
    bmodeOutCombo = new QComboBox();
    bmodeOutCombo->addItem("Balance", 0);
    bmodeOutCombo->addItem("Amplitude", 1);
    bmodeOutCombo->addItem("Power", 2);
    bmodeOutCombo->setCurrentIndex(m_bmodeOut);
    balanceOutLayout->addWidget(bmodeOutCombo);
    levelsLayout->addWidget(balanceOutGroup);

    levelsLayout->addStretch();
    tabWidget->addTab(levelsTab, "Levels");

    // ========== TAB 2: M/S PROCESSING ==========
    auto msTab = new QWidget();
    auto msLayout = new QVBoxLayout(msTab);

    // Stereo Mode
    auto modeGroup = new QGroupBox("Stereo Mode");
    auto modeLayout = new QVBoxLayout(modeGroup);
    modeCombo = new QComboBox();
    modeCombo->addItem("L/R → L/R (passthrough)", 0);
    modeCombo->addItem("L/R → M/S (encode)", 1);
    modeCombo->addItem("M/S → L/R (decode)", 2);
    modeCombo->addItem("L/R → L/L", 3);
    modeCombo->addItem("L/R → R/R", 4);
    modeCombo->addItem("L/R → L+R/L+R (mono)", 5);
    modeCombo->addItem("L/R → R/L (swap)", 6);
    modeCombo->addItem("Side only", 7);
    modeCombo->addItem("Middle only", 8);
    modeCombo->addItem("M/S → L/L", 9);
    modeCombo->addItem("M/S → R/R", 10);
    modeCombo->setCurrentIndex(m_mode);
    modeLayout->addWidget(modeCombo);
    msLayout->addWidget(modeGroup);

    // Side Level (dB)
    auto slevGroup = new QGroupBox("Side Level (dB)");
    auto slevLayout = new QVBoxLayout(slevGroup);
    slevSpinBox = new QDoubleSpinBox();
    slevSpinBox->setMinimum(-36.0);
    slevSpinBox->setMaximum(36.0);
    slevSpinBox->setValue(linearToDb(m_slev));
    slevSpinBox->setDecimals(2);
    slevSpinBox->setSuffix(" dB");
    auto slevControlLayout = new QHBoxLayout();
    slevControlLayout->addWidget(slevSpinBox);
    slevControlLayout->addStretch();
    slevControlLayout->addWidget(new QLabel("-36"));
    slevSlider = new QSlider(Qt::Horizontal);
    slevSlider->setMinimum(-3600);
    slevSlider->setMaximum(3600);
    slevSlider->setValue(static_cast<int>(linearToDb(m_slev) * 100));
    slevControlLayout->addWidget(slevSlider, 1);
    slevControlLayout->addWidget(new QLabel("+36"));
    slevLayout->addLayout(slevControlLayout);
    msLayout->addWidget(slevGroup);

    // Middle Level (dB)
    auto mlevGroup = new QGroupBox("Middle Level (dB)");
    auto mlevLayout = new QVBoxLayout(mlevGroup);
    mlevSpinBox = new QDoubleSpinBox();
    mlevSpinBox->setMinimum(-36.0);
    mlevSpinBox->setMaximum(36.0);
    mlevSpinBox->setValue(linearToDb(m_mlev));
    mlevSpinBox->setDecimals(2);
    mlevSpinBox->setSuffix(" dB");
    auto mlevControlLayout = new QHBoxLayout();
    mlevControlLayout->addWidget(mlevSpinBox);
    mlevControlLayout->addStretch();
    mlevControlLayout->addWidget(new QLabel("-36"));
    mlevSlider = new QSlider(Qt::Horizontal);
    mlevSlider->setMinimum(-3600);
    mlevSlider->setMaximum(3600);
    mlevSlider->setValue(static_cast<int>(linearToDb(m_mlev) * 100));
    mlevControlLayout->addWidget(mlevSlider, 1);
    mlevControlLayout->addWidget(new QLabel("+36"));
    mlevLayout->addLayout(mlevControlLayout);
    msLayout->addWidget(mlevGroup);

    // Stereo Base
    auto baseGroup = new QGroupBox("Stereo Base (Width)");
    auto baseLayout = new QVBoxLayout(baseGroup);
    baseSpinBox = new QDoubleSpinBox();
    baseSpinBox->setMinimum(-100.0);
    baseSpinBox->setMaximum(100.0);
    baseSpinBox->setValue(m_base * 100.0);
    baseSpinBox->setDecimals(2);
    baseSpinBox->setSuffix(" %");
    auto baseControlLayout = new QHBoxLayout();
    baseControlLayout->addWidget(baseSpinBox);
    baseControlLayout->addStretch();
    baseControlLayout->addWidget(new QLabel("Narrow"));
    baseSlider = new QSlider(Qt::Horizontal);
    baseSlider->setMinimum(-100);
    baseSlider->setMaximum(100);
    baseSlider->setValue(static_cast<int>(m_base * 100));
    baseControlLayout->addWidget(baseSlider, 1);
    baseControlLayout->addWidget(new QLabel("Wide"));
    baseLayout->addLayout(baseControlLayout);
    msLayout->addWidget(baseGroup);

    // Delay (ms)
    auto delayGroup = new QGroupBox("Stereo Delay (ms)");
    auto delayLayout = new QVBoxLayout(delayGroup);
    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(-20.0);
    delaySpinBox->setMaximum(20.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(2);
    delaySpinBox->setSuffix(" ms");
    auto delayControlLayout = new QHBoxLayout();
    delayControlLayout->addWidget(delaySpinBox);
    delayControlLayout->addStretch();
    delayControlLayout->addWidget(new QLabel("L early"));
    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(-2000);
    delaySlider->setMaximum(2000);
    delaySlider->setValue(static_cast<int>(m_delay * 100));
    delayControlLayout->addWidget(delaySlider, 1);
    delayControlLayout->addWidget(new QLabel("R early"));
    delayLayout->addLayout(delayControlLayout);
    msLayout->addWidget(delayGroup);

    // Phase (degrees)
    auto phaseGroup = new QGroupBox("Stereo Phase (°)");
    auto phaseLayout = new QVBoxLayout(phaseGroup);
    phaseSpinBox = new QDoubleSpinBox();
    phaseSpinBox->setMinimum(0.0);
    phaseSpinBox->setMaximum(360.0);
    phaseSpinBox->setValue(m_phase);
    phaseSpinBox->setDecimals(2);
    phaseSpinBox->setSuffix("°");
    auto phaseControlLayout = new QHBoxLayout();
    phaseControlLayout->addWidget(phaseSpinBox);
    phaseControlLayout->addStretch();
    phaseControlLayout->addWidget(new QLabel("0°"));
    phaseSlider = new QSlider(Qt::Horizontal);
    phaseSlider->setMinimum(0);
    phaseSlider->setMaximum(36000);
    phaseSlider->setValue(static_cast<int>(m_phase * 100));
    phaseControlLayout->addWidget(phaseSlider, 1);
    phaseControlLayout->addWidget(new QLabel("360°"));
    phaseLayout->addLayout(phaseControlLayout);
    msLayout->addWidget(phaseGroup);

    msLayout->addStretch();
    tabWidget->addTab(msTab, "M/S");

    // ========== TAB 3: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Mute/Phase options
    auto channelGroup = new QGroupBox("Channel Options");
    auto channelLayout = new QVBoxLayout(channelGroup);
    mutelCheck = new QCheckBox("Mute Left Channel");
    mutelCheck->setChecked(m_mutel);
    channelLayout->addWidget(mutelCheck);
    muterCheck = new QCheckBox("Mute Right Channel");
    muterCheck->setChecked(m_muter);
    channelLayout->addWidget(muterCheck);
    phaselCheck = new QCheckBox("Invert Left Phase");
    phaselCheck->setChecked(m_phasel);
    channelLayout->addWidget(phaselCheck);
    phaserCheck = new QCheckBox("Invert Right Phase");
    phaserCheck->setChecked(m_phaser);
    channelLayout->addWidget(phaserCheck);
    optionsLayout->addWidget(channelGroup);

    // Softclip
    auto clipGroup = new QGroupBox("Output Protection");
    auto clipLayout = new QVBoxLayout(clipGroup);
    softclipCheck = new QCheckBox("Enable Soft Clipping");
    softclipCheck->setChecked(m_softclip);
    clipLayout->addWidget(softclipCheck);
    optionsLayout->addWidget(clipGroup);

    // Side/Center Level
    auto scGroup = new QGroupBox("Side/Center Level");
    auto scLayout = new QVBoxLayout(scGroup);
    sclevelSpinBox = new QDoubleSpinBox();
    sclevelSpinBox->setMinimum(1.0);
    sclevelSpinBox->setMaximum(100.0);
    sclevelSpinBox->setValue(m_sclevel);
    sclevelSpinBox->setDecimals(2);
    auto scControlLayout = new QHBoxLayout();
    scControlLayout->addWidget(sclevelSpinBox);
    scControlLayout->addStretch();
    scControlLayout->addWidget(new QLabel("1"));
    sclevelSlider = new QSlider(Qt::Horizontal);
    sclevelSlider->setMinimum(100);
    sclevelSlider->setMaximum(10000);
    sclevelSlider->setValue(static_cast<int>(m_sclevel * 100));
    scControlLayout->addWidget(sclevelSlider, 1);
    scControlLayout->addWidget(new QLabel("100"));
    scLayout->addLayout(scControlLayout);
    optionsLayout->addWidget(scGroup);

    // Side Balance and Middle Pan (additional controls)
    auto panGroup = new QGroupBox("Pan Controls");
    auto panLayout = new QVBoxLayout(panGroup);
    
    auto sbalLayout = new QHBoxLayout();
    sbalLayout->addWidget(new QLabel("Side:"));
    sbalSpinBox = new QDoubleSpinBox();
    sbalSpinBox->setMinimum(-100.0);
    sbalSpinBox->setMaximum(100.0);
    sbalSpinBox->setValue(m_sbal * 100.0);
    sbalSpinBox->setDecimals(2);
    sbalSpinBox->setSuffix(" %");
    sbalLayout->addWidget(sbalSpinBox);
    sbalSlider = new QSlider(Qt::Horizontal);
    sbalSlider->setMinimum(-100);
    sbalSlider->setMaximum(100);
    sbalSlider->setValue(static_cast<int>(m_sbal * 100));
    sbalLayout->addWidget(sbalSlider, 1);
    panLayout->addLayout(sbalLayout);
    
    auto mpanLayout = new QHBoxLayout();
    mpanLayout->addWidget(new QLabel("Mid:"));
    mpanSpinBox = new QDoubleSpinBox();
    mpanSpinBox->setMinimum(-100.0);
    mpanSpinBox->setMaximum(100.0);
    mpanSpinBox->setValue(m_mpan * 100.0);
    mpanSpinBox->setDecimals(2);
    mpanSpinBox->setSuffix(" %");
    mpanLayout->addWidget(mpanSpinBox);
    mpanSlider = new QSlider(Qt::Horizontal);
    mpanSlider->setMinimum(-100);
    mpanSlider->setMaximum(100);
    mpanSlider->setValue(static_cast<int>(m_mpan * 100));
    mpanLayout->addWidget(mpanSlider, 1);
    panLayout->addLayout(mpanLayout);
    
    optionsLayout->addWidget(panGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========
    
    // Level In
    connect(levelInSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_levelIn = dbToLinear(db);
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(db);
        levelInSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelIn = dbToLinear(value);
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(value * 100));
        levelInSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Level Out
    connect(levelOutSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_levelOut = dbToLinear(db);
        levelOutSpinBox->blockSignals(true);
        levelOutSpinBox->setValue(db);
        levelOutSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelOut = dbToLinear(value);
        levelOutSlider->blockSignals(true);
        levelOutSlider->setValue(static_cast<int>(value * 100));
        levelOutSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Balance In
    connect(balanceInSlider, &QSlider::valueChanged, [this](int value) {
        m_balanceIn = value / 100.0;
        balanceInSpinBox->blockSignals(true);
        balanceInSpinBox->setValue(value);
        balanceInSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(balanceInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_balanceIn = value / 100.0;
        balanceInSlider->blockSignals(true);
        balanceInSlider->setValue(static_cast<int>(value));
        balanceInSlider->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bmodeInCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_bmodeIn = bmodeInCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Balance Out
    connect(balanceOutSlider, &QSlider::valueChanged, [this](int value) {
        m_balanceOut = value / 100.0;
        balanceOutSpinBox->blockSignals(true);
        balanceOutSpinBox->setValue(value);
        balanceOutSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(balanceOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_balanceOut = value / 100.0;
        balanceOutSlider->blockSignals(true);
        balanceOutSlider->setValue(static_cast<int>(value));
        balanceOutSlider->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bmodeOutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_bmodeOut = bmodeOutCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Mode
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = modeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Side Level
    connect(slevSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_slev = dbToLinear(db);
        slevSpinBox->blockSignals(true);
        slevSpinBox->setValue(db);
        slevSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(slevSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_slev = dbToLinear(value);
        slevSlider->blockSignals(true);
        slevSlider->setValue(static_cast<int>(value * 100));
        slevSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Side Balance
    connect(sbalSlider, &QSlider::valueChanged, [this](int value) {
        m_sbal = value / 100.0;
        sbalSpinBox->blockSignals(true);
        sbalSpinBox->setValue(value);
        sbalSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(sbalSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_sbal = value / 100.0;
        sbalSlider->blockSignals(true);
        sbalSlider->setValue(static_cast<int>(value));
        sbalSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Middle Level
    connect(mlevSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_mlev = dbToLinear(db);
        mlevSpinBox->blockSignals(true);
        mlevSpinBox->setValue(db);
        mlevSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(mlevSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_mlev = dbToLinear(value);
        mlevSlider->blockSignals(true);
        mlevSlider->setValue(static_cast<int>(value * 100));
        mlevSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Middle Pan
    connect(mpanSlider, &QSlider::valueChanged, [this](int value) {
        m_mpan = value / 100.0;
        mpanSpinBox->blockSignals(true);
        mpanSpinBox->setValue(value);
        mpanSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(mpanSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_mpan = value / 100.0;
        mpanSlider->blockSignals(true);
        mpanSlider->setValue(static_cast<int>(value));
        mpanSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Stereo Base
    connect(baseSlider, &QSlider::valueChanged, [this](int value) {
        m_base = value / 100.0;
        baseSpinBox->blockSignals(true);
        baseSpinBox->setValue(value);
        baseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(baseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_base = value / 100.0;
        baseSlider->blockSignals(true);
        baseSlider->setValue(static_cast<int>(value));
        baseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Delay
    connect(delaySlider, &QSlider::valueChanged, [this](int value) {
        m_delay = value / 100.0;
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_delay = value;
        delaySlider->blockSignals(true);
        delaySlider->setValue(static_cast<int>(value * 100));
        delaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Phase
    connect(phaseSlider, &QSlider::valueChanged, [this](int value) {
        m_phase = value / 100.0;
        phaseSpinBox->blockSignals(true);
        phaseSpinBox->setValue(m_phase);
        phaseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(phaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_phase = value;
        phaseSlider->blockSignals(true);
        phaseSlider->setValue(static_cast<int>(value * 100));
        phaseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // S/C Level
    connect(sclevelSlider, &QSlider::valueChanged, [this](int value) {
        m_sclevel = value / 100.0;
        sclevelSpinBox->blockSignals(true);
        sclevelSpinBox->setValue(m_sclevel);
        sclevelSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(sclevelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_sclevel = value;
        sclevelSlider->blockSignals(true);
        sclevelSlider->setValue(static_cast<int>(value * 100));
        sclevelSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Checkboxes
    connect(softclipCheck, &QCheckBox::toggled, [this](bool checked) {
        m_softclip = checked;
        updateFFmpegFlags();
    });
    connect(mutelCheck, &QCheckBox::toggled, [this](bool checked) {
        m_mutel = checked;
        updateFFmpegFlags();
    });
    connect(muterCheck, &QCheckBox::toggled, [this](bool checked) {
        m_muter = checked;
        updateFFmpegFlags();
    });
    connect(phaselCheck, &QCheckBox::toggled, [this](bool checked) {
        m_phasel = checked;
        updateFFmpegFlags();
    });
    connect(phaserCheck, &QCheckBox::toggled, [this](bool checked) {
        m_phaser = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFStereotools::updateFFmpegFlags() {
    QStringList modeNames = {"lr>lr", "lr>ms", "ms>lr", "lr>ll", "lr>rr", 
                             "lr>l+r", "lr>rl", "sidech", "midch", "ms>ll", "ms>rr"};
    QStringList bmodeNames = {"balance", "amplitude", "power"};
    
    QString modeName = (m_mode >= 0 && m_mode < modeNames.size()) 
        ? modeNames[m_mode] : "lr>lr";
    QString bmodeInName = (m_bmodeIn >= 0 && m_bmodeIn < bmodeNames.size())
        ? bmodeNames[m_bmodeIn] : "balance";
    QString bmodeOutName = (m_bmodeOut >= 0 && m_bmodeOut < bmodeNames.size())
        ? bmodeNames[m_bmodeOut] : "balance";
    
    ffmpegFlags = QString("stereotools=level_in=%1:level_out=%2:balance_in=%3:balance_out=%4:"
                          "softclip=%5:mutel=%6:muter=%7:phasel=%8:phaser=%9:"
                          "mode=%10:slev=%11:sbal=%12:mlev=%13:mpan=%14:base=%15:"
                          "delay=%16:sclevel=%17:phase=%18:bmode_in=%19:bmode_out=%20")
        .arg(m_levelIn, 0, 'f', 6)
        .arg(m_levelOut, 0, 'f', 6)
        .arg(m_balanceIn, 0, 'f', 2)
        .arg(m_balanceOut, 0, 'f', 2)
        .arg(m_softclip ? "true" : "false")
        .arg(m_mutel ? "true" : "false")
        .arg(m_muter ? "true" : "false")
        .arg(m_phasel ? "true" : "false")
        .arg(m_phaser ? "true" : "false")
        .arg(modeName)
        .arg(m_slev, 0, 'f', 6)
        .arg(m_sbal, 0, 'f', 2)
        .arg(m_mlev, 0, 'f', 6)
        .arg(m_mpan, 0, 'f', 2)
        .arg(m_base, 0, 'f', 2)
        .arg(m_delay, 0, 'f', 2)
        .arg(m_sclevel, 0, 'f', 2)
        .arg(m_phase, 0, 'f', 2)
        .arg(bmodeInName)
        .arg(bmodeOutName);
}

QString FFStereotools::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFStereotools::toJSON(QJsonObject& json) const {
    json["type"] = "ff-stereotools";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["balance_in"] = m_balanceIn;
    json["balance_out"] = m_balanceOut;
    json["bmode_in"] = m_bmodeIn;
    json["bmode_out"] = m_bmodeOut;
    json["mode"] = m_mode;
    json["slev"] = m_slev;
    json["sbal"] = m_sbal;
    json["mlev"] = m_mlev;
    json["mpan"] = m_mpan;
    json["base"] = m_base;
    json["delay"] = m_delay;
    json["phase"] = m_phase;
    json["sclevel"] = m_sclevel;
    json["softclip"] = m_softclip;
    json["mutel"] = m_mutel;
    json["muter"] = m_muter;
    json["phasel"] = m_phasel;
    json["phaser"] = m_phaser;
}

void FFStereotools::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_balanceIn = json["balance_in"].toDouble(0.0);
    m_balanceOut = json["balance_out"].toDouble(0.0);
    m_bmodeIn = json["bmode_in"].toInt(0);
    m_bmodeOut = json["bmode_out"].toInt(0);
    m_mode = json["mode"].toInt(0);
    m_slev = json["slev"].toDouble(1.0);
    m_sbal = json["sbal"].toDouble(0.0);
    m_mlev = json["mlev"].toDouble(1.0);
    m_mpan = json["mpan"].toDouble(0.0);
    m_base = json["base"].toDouble(0.0);
    m_delay = json["delay"].toDouble(0.0);
    m_phase = json["phase"].toDouble(0.0);
    m_sclevel = json["sclevel"].toDouble(1.0);
    m_softclip = json["softclip"].toBool(false);
    m_mutel = json["mutel"].toBool(false);
    m_muter = json["muter"].toBool(false);
    m_phasel = json["phasel"].toBool(false);
    m_phaser = json["phaser"].toBool(false);
    updateFFmpegFlags();
}
