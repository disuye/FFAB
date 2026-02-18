#include "ff-haas.h"
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

FFHaas::FFHaas() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

double FFHaas::dbToLinear(double db) const {
    return std::pow(10.0, db / 20.0);
}

double FFHaas::linearToDb(double linear) const {
    if (linear <= 0.0) return -36.0;
    return 20.0 * std::log10(linear);
}

QWidget* FFHaas::getParametersWidget() {
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
    levelInSpinBox->setSingleStep(0.5);
    levelInSpinBox->setSuffix(" dB");

    auto levelInControlLayout = new QHBoxLayout();
    levelInControlLayout->addWidget(levelInSpinBox);
    levelInControlLayout->addStretch();
    auto levelInMinLabel = new QLabel("-36");
    levelInControlLayout->addWidget(levelInMinLabel);
    levelInSlider = new QSlider(Qt::Horizontal);
    levelInSlider->setMinimum(-3600);
    levelInSlider->setMaximum(3600);
    levelInSlider->setValue(static_cast<int>(linearToDb(m_levelIn) * 100));
    levelInControlLayout->addWidget(levelInSlider, 1);
    auto levelInMaxLabel = new QLabel("+36");
    levelInControlLayout->addWidget(levelInMaxLabel);
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
    levelOutSpinBox->setSingleStep(0.5);
    levelOutSpinBox->setSuffix(" dB");

    auto levelOutControlLayout = new QHBoxLayout();
    levelOutControlLayout->addWidget(levelOutSpinBox);
    levelOutControlLayout->addStretch();
    auto levelOutMinLabel = new QLabel("-36");
    levelOutControlLayout->addWidget(levelOutMinLabel);
    levelOutSlider = new QSlider(Qt::Horizontal);
    levelOutSlider->setMinimum(-3600);
    levelOutSlider->setMaximum(3600);
    levelOutSlider->setValue(static_cast<int>(linearToDb(m_levelOut) * 100));
    levelOutControlLayout->addWidget(levelOutSlider, 1);
    auto levelOutMaxLabel = new QLabel("+36");
    levelOutControlLayout->addWidget(levelOutMaxLabel);
    levelOutLayout->addLayout(levelOutControlLayout);
    levelsLayout->addWidget(levelOutGroup);

    // Side Gain (dB)
    auto sideGainGroup = new QGroupBox("Side Gain (dB)");
    auto sideGainLayout = new QVBoxLayout(sideGainGroup);

    sideGainSpinBox = new QDoubleSpinBox();
    sideGainSpinBox->setMinimum(-36.0);
    sideGainSpinBox->setMaximum(36.0);
    sideGainSpinBox->setValue(linearToDb(m_sideGain));
    sideGainSpinBox->setDecimals(2);
    sideGainSpinBox->setSingleStep(0.5);
    sideGainSpinBox->setSuffix(" dB");

    auto sideGainControlLayout = new QHBoxLayout();
    sideGainControlLayout->addWidget(sideGainSpinBox);
    sideGainControlLayout->addStretch();
    auto sideGainMinLabel = new QLabel("-36");
    sideGainControlLayout->addWidget(sideGainMinLabel);
    sideGainSlider = new QSlider(Qt::Horizontal);
    sideGainSlider->setMinimum(-3600);
    sideGainSlider->setMaximum(3600);
    sideGainSlider->setValue(static_cast<int>(linearToDb(m_sideGain) * 100));
    sideGainControlLayout->addWidget(sideGainSlider, 1);
    auto sideGainMaxLabel = new QLabel("+36");
    sideGainControlLayout->addWidget(sideGainMaxLabel);
    sideGainLayout->addLayout(sideGainControlLayout);
    levelsLayout->addWidget(sideGainGroup);

    // Middle Source
    auto middleSourceGroup = new QGroupBox("Middle Source");
    auto middleSourceLayout = new QVBoxLayout(middleSourceGroup);

    middleSourceCombo = new QComboBox();
    middleSourceCombo->addItem("Left Channel", 0);
    middleSourceCombo->addItem("Right Channel", 1);
    middleSourceCombo->addItem("Mid (L+R)", 2);
    middleSourceCombo->addItem("Side (L-R)", 3);
    middleSourceCombo->setCurrentIndex(m_middleSource);
    middleSourceLayout->addWidget(middleSourceCombo);
    levelsLayout->addWidget(middleSourceGroup);

    // Middle Phase
    middlePhaseCheck = new QCheckBox("Invert Middle Phase");
    middlePhaseCheck->setChecked(m_middlePhase);
    levelsLayout->addWidget(middlePhaseCheck);

    levelsLayout->addStretch();
    tabWidget->addTab(levelsTab, "Levels");

    // ========== TAB 2: LEFT/RIGHT CHANNELS ==========
    auto channelsTab = new QWidget();
    auto channelsLayout = new QVBoxLayout(channelsTab);

    // Left Channel Group
    auto leftGroup = new QGroupBox("Left Channel");
    auto leftLayout = new QVBoxLayout(leftGroup);

    // Left Delay
    auto leftDelayLayout = new QHBoxLayout();
    leftDelayLayout->addWidget(new QLabel("Delay:"));
    leftDelaySpinBox = new QDoubleSpinBox();
    leftDelaySpinBox->setMinimum(0.0);
    leftDelaySpinBox->setMaximum(40.0);
    leftDelaySpinBox->setValue(m_leftDelay);
    leftDelaySpinBox->setDecimals(2);
    leftDelaySpinBox->setSuffix(" ms");
    leftDelayLayout->addWidget(leftDelaySpinBox);
    leftDelaySlider = new QSlider(Qt::Horizontal);
    leftDelaySlider->setMinimum(0);
    leftDelaySlider->setMaximum(4000);
    leftDelaySlider->setValue(static_cast<int>(m_leftDelay * 100));
    leftDelayLayout->addWidget(leftDelaySlider, 1);
    leftLayout->addLayout(leftDelayLayout);

    // Left Balance
    auto leftBalanceLayout = new QHBoxLayout();
    leftBalanceLayout->addWidget(new QLabel("Balance:"));
    leftBalanceSpinBox = new QDoubleSpinBox();
    leftBalanceSpinBox->setMinimum(-100.0);
    leftBalanceSpinBox->setMaximum(100.0);
    leftBalanceSpinBox->setValue(m_leftBalance * 100.0);
    leftBalanceSpinBox->setDecimals(2);
    leftBalanceSpinBox->setSuffix(" %");
    leftBalanceLayout->addWidget(leftBalanceSpinBox);
    leftBalanceSlider = new QSlider(Qt::Horizontal);
    leftBalanceSlider->setMinimum(-100);
    leftBalanceSlider->setMaximum(100);
    leftBalanceSlider->setValue(static_cast<int>(m_leftBalance * 100));
    leftBalanceLayout->addWidget(leftBalanceSlider, 1);
    leftLayout->addLayout(leftBalanceLayout);

    // Left Gain
    auto leftGainLayout = new QHBoxLayout();
    leftGainLayout->addWidget(new QLabel("Gain:"));
    leftGainSpinBox = new QDoubleSpinBox();
    leftGainSpinBox->setMinimum(-36.0);
    leftGainSpinBox->setMaximum(36.0);
    leftGainSpinBox->setValue(linearToDb(m_leftGain));
    leftGainSpinBox->setDecimals(2);
    leftGainSpinBox->setSuffix(" dB");
    leftGainLayout->addWidget(leftGainSpinBox);
    leftGainSlider = new QSlider(Qt::Horizontal);
    leftGainSlider->setMinimum(-3600);
    leftGainSlider->setMaximum(3600);
    leftGainSlider->setValue(static_cast<int>(linearToDb(m_leftGain) * 100));
    leftGainLayout->addWidget(leftGainSlider, 1);
    leftLayout->addLayout(leftGainLayout);

    // Left Phase
    leftPhaseCheck = new QCheckBox("Invert Phase");
    leftPhaseCheck->setChecked(m_leftPhase);
    leftLayout->addWidget(leftPhaseCheck);

    channelsLayout->addWidget(leftGroup);

    // Right Channel Group
    auto rightGroup = new QGroupBox("Right Channel");
    auto rightLayout = new QVBoxLayout(rightGroup);

    // Right Delay
    auto rightDelayLayout = new QHBoxLayout();
    rightDelayLayout->addWidget(new QLabel("Delay:"));
    rightDelaySpinBox = new QDoubleSpinBox();
    rightDelaySpinBox->setMinimum(0.0);
    rightDelaySpinBox->setMaximum(40.0);
    rightDelaySpinBox->setValue(m_rightDelay);
    rightDelaySpinBox->setDecimals(2);
    rightDelaySpinBox->setSuffix(" ms");
    rightDelayLayout->addWidget(rightDelaySpinBox);
    rightDelaySlider = new QSlider(Qt::Horizontal);
    rightDelaySlider->setMinimum(0);
    rightDelaySlider->setMaximum(4000);
    rightDelaySlider->setValue(static_cast<int>(m_rightDelay * 100));
    rightDelayLayout->addWidget(rightDelaySlider, 1);
    rightLayout->addLayout(rightDelayLayout);

    // Right Balance
    auto rightBalanceLayout = new QHBoxLayout();
    rightBalanceLayout->addWidget(new QLabel("Balance:"));
    rightBalanceSpinBox = new QDoubleSpinBox();
    rightBalanceSpinBox->setMinimum(-100.0);
    rightBalanceSpinBox->setMaximum(100.0);
    rightBalanceSpinBox->setValue(m_rightBalance * 100.0);
    rightBalanceSpinBox->setDecimals(2);
    rightBalanceSpinBox->setSuffix(" %");
    rightBalanceLayout->addWidget(rightBalanceSpinBox);
    rightBalanceSlider = new QSlider(Qt::Horizontal);
    rightBalanceSlider->setMinimum(-100);
    rightBalanceSlider->setMaximum(100);
    rightBalanceSlider->setValue(static_cast<int>(m_rightBalance * 100));
    rightBalanceLayout->addWidget(rightBalanceSlider, 1);
    rightLayout->addLayout(rightBalanceLayout);

    // Right Gain
    auto rightGainLayout = new QHBoxLayout();
    rightGainLayout->addWidget(new QLabel("Gain:"));
    rightGainSpinBox = new QDoubleSpinBox();
    rightGainSpinBox->setMinimum(-36.0);
    rightGainSpinBox->setMaximum(36.0);
    rightGainSpinBox->setValue(linearToDb(m_rightGain));
    rightGainSpinBox->setDecimals(2);
    rightGainSpinBox->setSuffix(" dB");
    rightGainLayout->addWidget(rightGainSpinBox);
    rightGainSlider = new QSlider(Qt::Horizontal);
    rightGainSlider->setMinimum(-3600);
    rightGainSlider->setMaximum(3600);
    rightGainSlider->setValue(static_cast<int>(linearToDb(m_rightGain) * 100));
    rightGainLayout->addWidget(rightGainSlider, 1);
    rightLayout->addLayout(rightGainLayout);

    // Right Phase
    rightPhaseCheck = new QCheckBox("Invert Phase");
    rightPhaseCheck->setChecked(m_rightPhase);
    rightLayout->addWidget(rightPhaseCheck);

    channelsLayout->addWidget(rightGroup);
    channelsLayout->addStretch();
    tabWidget->addTab(channelsTab, "Channels");

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

    // Side Gain
    connect(sideGainSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_sideGain = dbToLinear(db);
        sideGainSpinBox->blockSignals(true);
        sideGainSpinBox->setValue(db);
        sideGainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(sideGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_sideGain = dbToLinear(value);
        sideGainSlider->blockSignals(true);
        sideGainSlider->setValue(static_cast<int>(value * 100));
        sideGainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Middle Source
    connect(middleSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_middleSource = middleSourceCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Middle Phase
    connect(middlePhaseCheck, &QCheckBox::toggled, [this](bool checked) {
        m_middlePhase = checked;
        updateFFmpegFlags();
    });

    // Left Delay
    connect(leftDelaySlider, &QSlider::valueChanged, [this](int value) {
        m_leftDelay = value / 100.0;
        leftDelaySpinBox->blockSignals(true);
        leftDelaySpinBox->setValue(m_leftDelay);
        leftDelaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(leftDelaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_leftDelay = value;
        leftDelaySlider->blockSignals(true);
        leftDelaySlider->setValue(static_cast<int>(value * 100));
        leftDelaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Left Balance
    connect(leftBalanceSlider, &QSlider::valueChanged, [this](int value) {
        m_leftBalance = value / 100.0;
        leftBalanceSpinBox->blockSignals(true);
        leftBalanceSpinBox->setValue(value);
        leftBalanceSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(leftBalanceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_leftBalance = value / 100.0;
        leftBalanceSlider->blockSignals(true);
        leftBalanceSlider->setValue(static_cast<int>(value));
        leftBalanceSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Left Gain
    connect(leftGainSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_leftGain = dbToLinear(db);
        leftGainSpinBox->blockSignals(true);
        leftGainSpinBox->setValue(db);
        leftGainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(leftGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_leftGain = dbToLinear(value);
        leftGainSlider->blockSignals(true);
        leftGainSlider->setValue(static_cast<int>(value * 100));
        leftGainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Left Phase
    connect(leftPhaseCheck, &QCheckBox::toggled, [this](bool checked) {
        m_leftPhase = checked;
        updateFFmpegFlags();
    });

    // Right Delay
    connect(rightDelaySlider, &QSlider::valueChanged, [this](int value) {
        m_rightDelay = value / 100.0;
        rightDelaySpinBox->blockSignals(true);
        rightDelaySpinBox->setValue(m_rightDelay);
        rightDelaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rightDelaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_rightDelay = value;
        rightDelaySlider->blockSignals(true);
        rightDelaySlider->setValue(static_cast<int>(value * 100));
        rightDelaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Right Balance
    connect(rightBalanceSlider, &QSlider::valueChanged, [this](int value) {
        m_rightBalance = value / 100.0;
        rightBalanceSpinBox->blockSignals(true);
        rightBalanceSpinBox->setValue(value);
        rightBalanceSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rightBalanceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_rightBalance = value / 100.0;
        rightBalanceSlider->blockSignals(true);
        rightBalanceSlider->setValue(static_cast<int>(value));
        rightBalanceSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Right Gain
    connect(rightGainSlider, &QSlider::valueChanged, [this](int value) {
        double db = value / 100.0;
        m_rightGain = dbToLinear(db);
        rightGainSpinBox->blockSignals(true);
        rightGainSpinBox->setValue(db);
        rightGainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rightGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_rightGain = dbToLinear(value);
        rightGainSlider->blockSignals(true);
        rightGainSlider->setValue(static_cast<int>(value * 100));
        rightGainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Right Phase
    connect(rightPhaseCheck, &QCheckBox::toggled, [this](bool checked) {
        m_rightPhase = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFHaas::updateFFmpegFlags() {
    QStringList middleSourceNames = {"left", "right", "mid", "side"};
    QString middleSourceName = (m_middleSource >= 0 && m_middleSource < middleSourceNames.size())
        ? middleSourceNames[m_middleSource] : "mid";
    
    ffmpegFlags = QString("haas=level_in=%1:level_out=%2:side_gain=%3:middle_source=%4:middle_phase=%5:"
                          "left_delay=%6:left_balance=%7:left_gain=%8:left_phase=%9:"
                          "right_delay=%10:right_balance=%11:right_gain=%12:right_phase=%13")
        .arg(m_levelIn, 0, 'f', 6)
        .arg(m_levelOut, 0, 'f', 6)
        .arg(m_sideGain, 0, 'f', 6)
        .arg(middleSourceName)
        .arg(m_middlePhase ? "true" : "false")
        .arg(m_leftDelay, 0, 'f', 2)
        .arg(m_leftBalance, 0, 'f', 2)
        .arg(m_leftGain, 0, 'f', 6)
        .arg(m_leftPhase ? "true" : "false")
        .arg(m_rightDelay, 0, 'f', 2)
        .arg(m_rightBalance, 0, 'f', 2)
        .arg(m_rightGain, 0, 'f', 6)
        .arg(m_rightPhase ? "true" : "false");
}

QString FFHaas::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFHaas::toJSON(QJsonObject& json) const {
    json["type"] = "ff-haas";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["side_gain"] = m_sideGain;
    json["middle_source"] = m_middleSource;
    json["middle_phase"] = m_middlePhase;
    json["left_delay"] = m_leftDelay;
    json["left_balance"] = m_leftBalance;
    json["left_gain"] = m_leftGain;
    json["left_phase"] = m_leftPhase;
    json["right_delay"] = m_rightDelay;
    json["right_balance"] = m_rightBalance;
    json["right_gain"] = m_rightGain;
    json["right_phase"] = m_rightPhase;
}

void FFHaas::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_sideGain = json["side_gain"].toDouble(1.0);
    m_middleSource = json["middle_source"].toInt(2);
    m_middlePhase = json["middle_phase"].toBool(false);
    m_leftDelay = json["left_delay"].toDouble(2.05);
    m_leftBalance = json["left_balance"].toDouble(-1.0);
    m_leftGain = json["left_gain"].toDouble(1.0);
    m_leftPhase = json["left_phase"].toBool(false);
    m_rightDelay = json["right_delay"].toDouble(2.12);
    m_rightBalance = json["right_balance"].toDouble(1.0);
    m_rightGain = json["right_gain"].toDouble(1.0);
    m_rightPhase = json["right_phase"].toBool(true);
    updateFFmpegFlags();
}
