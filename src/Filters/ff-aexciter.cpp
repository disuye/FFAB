#include "ff-aexciter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <cmath>

FFAexciter::FFAexciter() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAexciter::getParametersWidget() {
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

    // ========== TAB 1: EXCITER ==========
    auto exciterTab = new QWidget();
    auto exciterLayout = new QVBoxLayout(exciterTab);

    // Amount parameter (0 - 64, displayed as dB)
    auto amountGroup = new QGroupBox("Amount (dB)");
    auto amountLayout = new QVBoxLayout(amountGroup);

    amountSpinBox = new QDoubleSpinBox();
    amountSpinBox->setMinimum(-36.0);
    amountSpinBox->setMaximum(36.0);
    amountSpinBox->setValue(linearToDb(m_amount));
    amountSpinBox->setDecimals(1);
    amountSpinBox->setSingleStep(0.5);
    amountSpinBox->setSuffix(" dB");

    auto amountControlLayout = new QHBoxLayout();
    amountControlLayout->addWidget(amountSpinBox);
    amountControlLayout->addStretch();
    amountControlLayout->addWidget(new QLabel("-36"));

    amountSlider = new QSlider(Qt::Horizontal);
    amountSlider->setMinimum(-3600);
    amountSlider->setMaximum(3600);
    amountSlider->setValue(static_cast<int>(linearToDb(m_amount) * 100));
    amountControlLayout->addWidget(amountSlider, 1);
    amountControlLayout->addWidget(new QLabel("+36"));

    amountLayout->addLayout(amountControlLayout);
    exciterLayout->addWidget(amountGroup);

    // Drive parameter (0.1 - 10)
    auto driveGroup = new QGroupBox("Drive (Harmonics)");
    auto driveLayout = new QVBoxLayout(driveGroup);

    driveSpinBox = new QDoubleSpinBox();
    driveSpinBox->setMinimum(0.1);
    driveSpinBox->setMaximum(10.0);
    driveSpinBox->setValue(m_drive);
    driveSpinBox->setDecimals(1);
    driveSpinBox->setSingleStep(0.1);

    auto driveControlLayout = new QHBoxLayout();
    driveControlLayout->addWidget(driveSpinBox);
    driveControlLayout->addStretch();
    driveControlLayout->addWidget(new QLabel("0.1"));

    driveSlider = new QSlider(Qt::Horizontal);
    driveSlider->setMinimum(10);
    driveSlider->setMaximum(1000);
    driveSlider->setValue(static_cast<int>(m_drive * 100));
    driveControlLayout->addWidget(driveSlider, 1);
    driveControlLayout->addWidget(new QLabel("10"));

    driveLayout->addLayout(driveControlLayout);
    exciterLayout->addWidget(driveGroup);

    // Blend parameter (-10 - 10)
    auto blendGroup = new QGroupBox("Harmonic Blend");
    auto blendLayout = new QVBoxLayout(blendGroup);

    blendSpinBox = new QDoubleSpinBox();
    blendSpinBox->setMinimum(-10.0);
    blendSpinBox->setMaximum(10.0);
    blendSpinBox->setValue(m_blend);
    blendSpinBox->setDecimals(1);
    blendSpinBox->setSingleStep(0.1);

    auto blendControlLayout = new QHBoxLayout();
    blendControlLayout->addWidget(blendSpinBox);
    blendControlLayout->addStretch();
    blendControlLayout->addWidget(new QLabel("-10"));

    blendSlider = new QSlider(Qt::Horizontal);
    blendSlider->setMinimum(-1000);
    blendSlider->setMaximum(1000);
    blendSlider->setValue(static_cast<int>(m_blend * 100));
    blendControlLayout->addWidget(blendSlider, 1);
    blendControlLayout->addWidget(new QLabel("+10"));

    blendLayout->addLayout(blendControlLayout);
    exciterLayout->addWidget(blendGroup);

    // Frequency parameter (2000 - 12000 Hz)
    auto freqGroup = new QGroupBox("Frequency (Hz)");
    auto freqLayout = new QVBoxLayout(freqGroup);

    freqSpinBox = new QDoubleSpinBox();
    freqSpinBox->setMinimum(2000.0);
    freqSpinBox->setMaximum(12000.0);
    freqSpinBox->setValue(m_freq);
    freqSpinBox->setDecimals(0);
    freqSpinBox->setSingleStep(100);
    freqSpinBox->setSuffix(" Hz");

    auto freqControlLayout = new QHBoxLayout();
    freqControlLayout->addWidget(freqSpinBox);
    freqControlLayout->addStretch();
    freqControlLayout->addWidget(new QLabel("2k"));

    freqSlider = new QSlider(Qt::Horizontal);
    freqSlider->setMinimum(2000);
    freqSlider->setMaximum(12000);
    freqSlider->setValue(static_cast<int>(m_freq));
    freqControlLayout->addWidget(freqSlider, 1);
    freqControlLayout->addWidget(new QLabel("12k"));

    freqLayout->addLayout(freqControlLayout);
    exciterLayout->addWidget(freqGroup);

    // Ceiling parameter (9999 - 20000 Hz)
    auto ceilGroup = new QGroupBox("Ceiling (Hz)");
    auto ceilLayout = new QVBoxLayout(ceilGroup);

    ceilSpinBox = new QDoubleSpinBox();
    ceilSpinBox->setMinimum(9999.0);
    ceilSpinBox->setMaximum(20000.0);
    ceilSpinBox->setValue(m_ceil);
    ceilSpinBox->setDecimals(0);
    ceilSpinBox->setSingleStep(100);
    ceilSpinBox->setSuffix(" Hz");

    auto ceilControlLayout = new QHBoxLayout();
    ceilControlLayout->addWidget(ceilSpinBox);
    ceilControlLayout->addStretch();
    ceilControlLayout->addWidget(new QLabel("10k"));

    ceilSlider = new QSlider(Qt::Horizontal);
    ceilSlider->setMinimum(9999);
    ceilSlider->setMaximum(20000);
    ceilSlider->setValue(static_cast<int>(m_ceil));
    ceilControlLayout->addWidget(ceilSlider, 1);
    ceilControlLayout->addWidget(new QLabel("20k"));

    ceilLayout->addLayout(ceilControlLayout);
    exciterLayout->addWidget(ceilGroup);

    exciterLayout->addStretch();
    tabWidget->addTab(exciterTab, "Exciter");

    // ========== TAB 2: LEVELS ==========
    auto levelsTab = new QWidget();
    auto levelsLayout = new QVBoxLayout(levelsTab);

    // Input Level parameter (0 - 64, displayed as dB)
    auto levelInGroup = new QGroupBox("Input Level (dB)");
    auto levelInLayout = new QVBoxLayout(levelInGroup);

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

    levelInLayout->addLayout(levelInControlLayout);
    levelsLayout->addWidget(levelInGroup);

    // Output Level parameter (0 - 64, displayed as dB)
    auto levelOutGroup = new QGroupBox("Output Level (dB)");
    auto levelOutLayout = new QVBoxLayout(levelOutGroup);

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

    levelOutLayout->addLayout(levelOutControlLayout);
    levelsLayout->addWidget(levelOutGroup);

    // Listen mode checkbox
    auto listenGroup = new QGroupBox("Monitor");
    auto listenLayout = new QVBoxLayout(listenGroup);
    listenCheckBox = new QCheckBox("Listen Mode (solo exciter signal)");
    listenCheckBox->setChecked(m_listen);
    listenLayout->addWidget(listenCheckBox);
    levelsLayout->addWidget(listenGroup);

    levelsLayout->addStretch();
    tabWidget->addTab(levelsTab, "Levels");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Amount (dB conversion)
    connect(amountSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 100.0;
        m_amount = dbToLinear(db);
        amountSpinBox->blockSignals(true);
        amountSpinBox->setValue(db);
        amountSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(amountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_amount = dbToLinear(db);
        amountSlider->blockSignals(true);
        amountSlider->setValue(static_cast<int>(db * 100));
        amountSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Drive
    connect(driveSlider, &QSlider::valueChanged, [this](int value) {
        m_drive = value / 100.0;
        driveSpinBox->blockSignals(true);
        driveSpinBox->setValue(m_drive);
        driveSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(driveSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_drive = value;
        driveSlider->blockSignals(true);
        driveSlider->setValue(static_cast<int>(value * 100));
        driveSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Blend
    connect(blendSlider, &QSlider::valueChanged, [this](int value) {
        m_blend = value / 100.0;
        blendSpinBox->blockSignals(true);
        blendSpinBox->setValue(m_blend);
        blendSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(blendSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_blend = value;
        blendSlider->blockSignals(true);
        blendSlider->setValue(static_cast<int>(value * 100));
        blendSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Frequency
    connect(freqSlider, &QSlider::valueChanged, [this](int value) {
        m_freq = value;
        freqSpinBox->blockSignals(true);
        freqSpinBox->setValue(m_freq);
        freqSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(freqSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_freq = value;
        freqSlider->blockSignals(true);
        freqSlider->setValue(static_cast<int>(value));
        freqSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Ceiling
    connect(ceilSlider, &QSlider::valueChanged, [this](int value) {
        m_ceil = value;
        ceilSpinBox->blockSignals(true);
        ceilSpinBox->setValue(m_ceil);
        ceilSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(ceilSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_ceil = value;
        ceilSlider->blockSignals(true);
        ceilSlider->setValue(static_cast<int>(value));
        ceilSlider->blockSignals(false);
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

    // Listen
    connect(listenCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_listen = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAexciter::updateFFmpegFlags() {
    ffmpegFlags = QString("aexciter=level_in=%1:level_out=%2:amount=%3:drive=%4:blend=%5:freq=%6:ceil=%7:listen=%8")
                      .arg(m_levelIn, 0, 'f', 4)
                      .arg(m_levelOut, 0, 'f', 4)
                      .arg(m_amount, 0, 'f', 4)
                      .arg(m_drive, 0, 'f', 2)
                      .arg(m_blend, 0, 'f', 2)
                      .arg(m_freq, 0, 'f', 0)
                      .arg(m_ceil, 0, 'f', 0)
                      .arg(m_listen ? "true" : "false");
}

QString FFAexciter::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAexciter::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aexciter";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["amount"] = m_amount;
    json["drive"] = m_drive;
    json["blend"] = m_blend;
    json["freq"] = m_freq;
    json["ceil"] = m_ceil;
    json["listen"] = m_listen;
}

void FFAexciter::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_amount = json["amount"].toDouble(1.0);
    m_drive = json["drive"].toDouble(8.5);
    m_blend = json["blend"].toDouble(0.0);
    m_freq = json["freq"].toDouble(7500.0);
    m_ceil = json["ceil"].toDouble(9999.0);
    m_listen = json["listen"].toBool(false);
    updateFFmpegFlags();
}
