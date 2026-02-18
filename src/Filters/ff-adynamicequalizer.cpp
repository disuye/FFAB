#include "ff-adynamicequalizer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <cmath>

FFAdynamicequalizer::FFAdynamicequalizer() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdynamicequalizer::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: DETECTION ==========
    auto detectionTab = new QWidget();
    auto detectionLayout = new QVBoxLayout(detectionTab);

    // Threshold (0 - 100 dB)
    auto thresholdGroup = new QGroupBox("Threshold (dB)");
    auto thresholdLayout = new QVBoxLayout(thresholdGroup);

    thresholdSpinBox = new QDoubleSpinBox();
    thresholdSpinBox->setMinimum(0.0);
    thresholdSpinBox->setMaximum(100.0);
    thresholdSpinBox->setValue(m_threshold);
    thresholdSpinBox->setDecimals(1);
    thresholdSpinBox->setSingleStep(1.0);
    thresholdSpinBox->setSuffix(" dB");

    auto thresholdControlLayout = new QHBoxLayout();
    thresholdControlLayout->addWidget(thresholdSpinBox);
    thresholdControlLayout->addStretch();
    thresholdControlLayout->addWidget(new QLabel("0"));
    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setMinimum(0);
    thresholdSlider->setMaximum(1000);
    thresholdSlider->setValue(static_cast<int>(m_threshold * 10));
    thresholdControlLayout->addWidget(thresholdSlider, 1);
    thresholdControlLayout->addWidget(new QLabel("100"));
    thresholdLayout->addLayout(thresholdControlLayout);
    detectionLayout->addWidget(thresholdGroup);

    // Detection Frequency (2 - 20000 Hz for practical use)
    auto dfrequencyGroup = new QGroupBox("Detection Frequency (Hz)");
    auto dfrequencyLayout = new QVBoxLayout(dfrequencyGroup);

    dfrequencySpinBox = new QDoubleSpinBox();
    dfrequencySpinBox->setMinimum(20.0);
    dfrequencySpinBox->setMaximum(20000.0);
    dfrequencySpinBox->setValue(m_dfrequency);
    dfrequencySpinBox->setDecimals(1);
    dfrequencySpinBox->setSingleStep(10.0);
    dfrequencySpinBox->setSuffix(" Hz");

    auto dfrequencyControlLayout = new QHBoxLayout();
    dfrequencyControlLayout->addWidget(dfrequencySpinBox);
    dfrequencyControlLayout->addStretch();
    dfrequencyControlLayout->addWidget(new QLabel("20"));
    dfrequencySlider = new QSlider(Qt::Horizontal);
    dfrequencySlider->setMinimum(20);
    dfrequencySlider->setMaximum(20000);
    dfrequencySlider->setValue(static_cast<int>(m_dfrequency));
    dfrequencyControlLayout->addWidget(dfrequencySlider, 1);
    dfrequencyControlLayout->addWidget(new QLabel("20k"));
    dfrequencyLayout->addLayout(dfrequencyControlLayout);
    detectionLayout->addWidget(dfrequencyGroup);

    // Detection Q Factor (0.1 - 100)
    auto dqfactorGroup = new QGroupBox("Detection Q Factor");
    auto dqfactorLayout = new QVBoxLayout(dqfactorGroup);

    dqfactorSpinBox = new QDoubleSpinBox();
    dqfactorSpinBox->setMinimum(0.1);
    dqfactorSpinBox->setMaximum(100.0);
    dqfactorSpinBox->setValue(m_dqfactor);
    dqfactorSpinBox->setDecimals(2);
    dqfactorSpinBox->setSingleStep(0.1);

    auto dqfactorControlLayout = new QHBoxLayout();
    dqfactorControlLayout->addWidget(dqfactorSpinBox);
    dqfactorControlLayout->addStretch();
    dqfactorControlLayout->addWidget(new QLabel("0.1"));
    dqfactorSlider = new QSlider(Qt::Horizontal);
    dqfactorSlider->setMinimum(10);
    dqfactorSlider->setMaximum(10000);
    dqfactorSlider->setValue(static_cast<int>(m_dqfactor * 100));
    dqfactorControlLayout->addWidget(dqfactorSlider, 1);
    dqfactorControlLayout->addWidget(new QLabel("100"));
    dqfactorLayout->addLayout(dqfactorControlLayout);
    detectionLayout->addWidget(dqfactorGroup);

    // Detection Filter Type
    auto dftypeGroup = new QGroupBox("Detection Filter Type");
    auto dftypeLayout = new QVBoxLayout(dftypeGroup);
    dftypeCombo = new QComboBox();
    dftypeCombo->addItem("Bandpass", 0);
    dftypeCombo->addItem("Lowpass", 1);
    dftypeCombo->addItem("Highpass", 2);
    dftypeCombo->addItem("Peak", 3);
    dftypeCombo->setCurrentIndex(m_dftype);
    dftypeLayout->addWidget(dftypeCombo);
    detectionLayout->addWidget(dftypeGroup);

    detectionLayout->addStretch();
    tabWidget->addTab(detectionTab, "Detection");

    // ========== TAB 2: TARGET ==========
    auto targetTab = new QWidget();
    auto targetLayout = new QVBoxLayout(targetTab);

    // Target Frequency (2 - 20000 Hz)
    auto tfrequencyGroup = new QGroupBox("Target Frequency (Hz)");
    auto tfrequencyLayout = new QVBoxLayout(tfrequencyGroup);

    tfrequencySpinBox = new QDoubleSpinBox();
    tfrequencySpinBox->setMinimum(20.0);
    tfrequencySpinBox->setMaximum(20000.0);
    tfrequencySpinBox->setValue(m_tfrequency);
    tfrequencySpinBox->setDecimals(1);
    tfrequencySpinBox->setSingleStep(10.0);
    tfrequencySpinBox->setSuffix(" Hz");

    auto tfrequencyControlLayout = new QHBoxLayout();
    tfrequencyControlLayout->addWidget(tfrequencySpinBox);
    tfrequencyControlLayout->addStretch();
    tfrequencyControlLayout->addWidget(new QLabel("20"));
    tfrequencySlider = new QSlider(Qt::Horizontal);
    tfrequencySlider->setMinimum(20);
    tfrequencySlider->setMaximum(20000);
    tfrequencySlider->setValue(static_cast<int>(m_tfrequency));
    tfrequencyControlLayout->addWidget(tfrequencySlider, 1);
    tfrequencyControlLayout->addWidget(new QLabel("20k"));
    tfrequencyLayout->addLayout(tfrequencyControlLayout);
    targetLayout->addWidget(tfrequencyGroup);

    // Target Q Factor (0.1 - 100)
    auto tqfactorGroup = new QGroupBox("Target Q Factor");
    auto tqfactorLayout = new QVBoxLayout(tqfactorGroup);

    tqfactorSpinBox = new QDoubleSpinBox();
    tqfactorSpinBox->setMinimum(0.1);
    tqfactorSpinBox->setMaximum(100.0);
    tqfactorSpinBox->setValue(m_tqfactor);
    tqfactorSpinBox->setDecimals(2);
    tqfactorSpinBox->setSingleStep(0.1);

    auto tqfactorControlLayout = new QHBoxLayout();
    tqfactorControlLayout->addWidget(tqfactorSpinBox);
    tqfactorControlLayout->addStretch();
    tqfactorControlLayout->addWidget(new QLabel("0.1"));
    tqfactorSlider = new QSlider(Qt::Horizontal);
    tqfactorSlider->setMinimum(10);
    tqfactorSlider->setMaximum(10000);
    tqfactorSlider->setValue(static_cast<int>(m_tqfactor * 100));
    tqfactorControlLayout->addWidget(tqfactorSlider, 1);
    tqfactorControlLayout->addWidget(new QLabel("100"));
    tqfactorLayout->addLayout(tqfactorControlLayout);
    targetLayout->addWidget(tqfactorGroup);

    // Target Filter Type
    auto tftypeGroup = new QGroupBox("Target Filter Type");
    auto tftypeLayout = new QVBoxLayout(tftypeGroup);
    tftypeCombo = new QComboBox();
    tftypeCombo->addItem("Bell", 0);
    tftypeCombo->addItem("Low Shelf", 1);
    tftypeCombo->addItem("High Shelf", 2);
    tftypeCombo->setCurrentIndex(m_tftype);
    tftypeLayout->addWidget(tftypeCombo);
    targetLayout->addWidget(tftypeGroup);

    targetLayout->addStretch();
    tabWidget->addTab(targetTab, "Target");

    // ========== TAB 3: DYNAMICS ==========
    auto dynamicsTab = new QWidget();
    auto dynamicsLayout = new QVBoxLayout(dynamicsTab);

    // Attack (0.01 - 2000 ms)
    auto attackGroup = new QGroupBox("Attack (ms)");
    auto attackLayout = new QVBoxLayout(attackGroup);

    attackSpinBox = new QDoubleSpinBox();
    attackSpinBox->setMinimum(0.01);
    attackSpinBox->setMaximum(2000.0);
    attackSpinBox->setValue(m_attack);
    attackSpinBox->setDecimals(2);
    attackSpinBox->setSingleStep(1.0);
    attackSpinBox->setSuffix(" ms");

    auto attackControlLayout = new QHBoxLayout();
    attackControlLayout->addWidget(attackSpinBox);
    attackControlLayout->addStretch();
    attackControlLayout->addWidget(new QLabel("0.01"));
    attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setMinimum(1);
    attackSlider->setMaximum(200000);
    attackSlider->setValue(static_cast<int>(m_attack * 100));
    attackControlLayout->addWidget(attackSlider, 1);
    attackControlLayout->addWidget(new QLabel("2000"));
    attackLayout->addLayout(attackControlLayout);
    dynamicsLayout->addWidget(attackGroup);

    // Release (0.01 - 2000 ms)
    auto releaseGroup = new QGroupBox("Release (ms)");
    auto releaseLayout = new QVBoxLayout(releaseGroup);

    releaseSpinBox = new QDoubleSpinBox();
    releaseSpinBox->setMinimum(0.01);
    releaseSpinBox->setMaximum(2000.0);
    releaseSpinBox->setValue(m_release);
    releaseSpinBox->setDecimals(2);
    releaseSpinBox->setSingleStep(1.0);
    releaseSpinBox->setSuffix(" ms");

    auto releaseControlLayout = new QHBoxLayout();
    releaseControlLayout->addWidget(releaseSpinBox);
    releaseControlLayout->addStretch();
    releaseControlLayout->addWidget(new QLabel("0.01"));
    releaseSlider = new QSlider(Qt::Horizontal);
    releaseSlider->setMinimum(1);
    releaseSlider->setMaximum(200000);
    releaseSlider->setValue(static_cast<int>(m_release * 100));
    releaseControlLayout->addWidget(releaseSlider, 1);
    releaseControlLayout->addWidget(new QLabel("2000"));
    releaseLayout->addLayout(releaseControlLayout);
    dynamicsLayout->addWidget(releaseGroup);

    // Ratio (0 - 30)
    auto ratioGroup = new QGroupBox("Ratio");
    auto ratioLayout = new QVBoxLayout(ratioGroup);

    ratioSpinBox = new QDoubleSpinBox();
    ratioSpinBox->setMinimum(0.0);
    ratioSpinBox->setMaximum(30.0);
    ratioSpinBox->setValue(m_ratio);
    ratioSpinBox->setDecimals(2);
    ratioSpinBox->setSingleStep(0.1);
    ratioSpinBox->setSuffix(":1");

    auto ratioControlLayout = new QHBoxLayout();
    ratioControlLayout->addWidget(ratioSpinBox);
    ratioControlLayout->addStretch();
    ratioControlLayout->addWidget(new QLabel("0"));
    ratioSlider = new QSlider(Qt::Horizontal);
    ratioSlider->setMinimum(0);
    ratioSlider->setMaximum(3000);
    ratioSlider->setValue(static_cast<int>(m_ratio * 100));
    ratioControlLayout->addWidget(ratioSlider, 1);
    ratioControlLayout->addWidget(new QLabel("30"));
    ratioLayout->addLayout(ratioControlLayout);
    dynamicsLayout->addWidget(ratioGroup);

    dynamicsLayout->addStretch();
    tabWidget->addTab(dynamicsTab, "Dynamics");

    // ========== TAB 4: OUTPUT ==========
    auto outputTab = new QWidget();
    auto outputLayout = new QVBoxLayout(outputTab);

    // Makeup Gain (0 - 100 dB practical range)
    auto makeupGroup = new QGroupBox("Makeup Gain (dB)");
    auto makeupLayout = new QVBoxLayout(makeupGroup);

    makeupSpinBox = new QDoubleSpinBox();
    makeupSpinBox->setMinimum(0.0);
    makeupSpinBox->setMaximum(100.0);
    makeupSpinBox->setValue(m_makeup);
    makeupSpinBox->setDecimals(1);
    makeupSpinBox->setSingleStep(0.5);
    makeupSpinBox->setSuffix(" dB");

    auto makeupControlLayout = new QHBoxLayout();
    makeupControlLayout->addWidget(makeupSpinBox);
    makeupControlLayout->addStretch();
    makeupControlLayout->addWidget(new QLabel("0"));
    makeupSlider = new QSlider(Qt::Horizontal);
    makeupSlider->setMinimum(0);
    makeupSlider->setMaximum(1000);
    makeupSlider->setValue(static_cast<int>(m_makeup * 10));
    makeupControlLayout->addWidget(makeupSlider, 1);
    makeupControlLayout->addWidget(new QLabel("100"));
    makeupLayout->addLayout(makeupControlLayout);
    outputLayout->addWidget(makeupGroup);

    // Range (1 - 100 dB practical range)
    auto rangeGroup = new QGroupBox("Max Gain (dB)");
    auto rangeLayout = new QVBoxLayout(rangeGroup);

    rangeSpinBox = new QDoubleSpinBox();
    rangeSpinBox->setMinimum(1.0);
    rangeSpinBox->setMaximum(100.0);
    rangeSpinBox->setValue(m_range);
    rangeSpinBox->setDecimals(1);
    rangeSpinBox->setSingleStep(1.0);
    rangeSpinBox->setSuffix(" dB");

    auto rangeControlLayout = new QHBoxLayout();
    rangeControlLayout->addWidget(rangeSpinBox);
    rangeControlLayout->addStretch();
    rangeControlLayout->addWidget(new QLabel("1"));
    rangeSlider = new QSlider(Qt::Horizontal);
    rangeSlider->setMinimum(10);
    rangeSlider->setMaximum(1000);
    rangeSlider->setValue(static_cast<int>(m_range * 10));
    rangeControlLayout->addWidget(rangeSlider, 1);
    rangeControlLayout->addWidget(new QLabel("100"));
    rangeLayout->addLayout(rangeControlLayout);
    outputLayout->addWidget(rangeGroup);

    // Mode
    auto modeGroup = new QGroupBox("Mode");
    auto modeLayout = new QVBoxLayout(modeGroup);
    modeCombo = new QComboBox();
    modeCombo->addItem("Listen (detection only)", -1);
    modeCombo->addItem("Cut Below Threshold", 0);
    modeCombo->addItem("Cut Above Threshold", 1);
    modeCombo->addItem("Boost Below Threshold", 2);
    modeCombo->addItem("Boost Above Threshold", 3);
    modeCombo->setCurrentIndex(m_mode + 1);  // Offset by 1 since -1 is first
    modeLayout->addWidget(modeCombo);
    outputLayout->addWidget(modeGroup);

    // Auto Threshold
    auto autoGroup = new QGroupBox("Auto Threshold");
    auto autoLayout = new QVBoxLayout(autoGroup);
    autoCombo = new QComboBox();
    autoCombo->addItem("Off", 1);
    autoCombo->addItem("On", 2);
    autoCombo->addItem("Adaptive", 3);
    autoCombo->addItem("Disabled", 4);
    autoCombo->setCurrentIndex(m_autoThreshold - 1);
    autoLayout->addWidget(autoCombo);
    outputLayout->addWidget(autoGroup);

    outputLayout->addStretch();
    tabWidget->addTab(outputTab, "Output");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Threshold
    connect(thresholdSlider, &QSlider::valueChanged, [this](int value) {
        m_threshold = value / 10.0;
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(m_threshold);
        thresholdSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_threshold = value;
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>(value * 10));
        thresholdSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Detection Frequency
    connect(dfrequencySlider, &QSlider::valueChanged, [this](int value) {
        m_dfrequency = value;
        dfrequencySpinBox->blockSignals(true);
        dfrequencySpinBox->setValue(m_dfrequency);
        dfrequencySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(dfrequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_dfrequency = value;
        dfrequencySlider->blockSignals(true);
        dfrequencySlider->setValue(static_cast<int>(value));
        dfrequencySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Detection Q Factor
    connect(dqfactorSlider, &QSlider::valueChanged, [this](int value) {
        m_dqfactor = value / 100.0;
        dqfactorSpinBox->blockSignals(true);
        dqfactorSpinBox->setValue(m_dqfactor);
        dqfactorSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(dqfactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_dqfactor = value;
        dqfactorSlider->blockSignals(true);
        dqfactorSlider->setValue(static_cast<int>(value * 100));
        dqfactorSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Detection Filter Type
    connect(dftypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_dftype = index;
        updateFFmpegFlags();
    });

    // Target Frequency
    connect(tfrequencySlider, &QSlider::valueChanged, [this](int value) {
        m_tfrequency = value;
        tfrequencySpinBox->blockSignals(true);
        tfrequencySpinBox->setValue(m_tfrequency);
        tfrequencySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(tfrequencySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_tfrequency = value;
        tfrequencySlider->blockSignals(true);
        tfrequencySlider->setValue(static_cast<int>(value));
        tfrequencySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Target Q Factor
    connect(tqfactorSlider, &QSlider::valueChanged, [this](int value) {
        m_tqfactor = value / 100.0;
        tqfactorSpinBox->blockSignals(true);
        tqfactorSpinBox->setValue(m_tqfactor);
        tqfactorSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(tqfactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_tqfactor = value;
        tqfactorSlider->blockSignals(true);
        tqfactorSlider->setValue(static_cast<int>(value * 100));
        tqfactorSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Target Filter Type
    connect(tftypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_tftype = index;
        updateFFmpegFlags();
    });

    // Attack
    connect(attackSlider, &QSlider::valueChanged, [this](int value) {
        m_attack = value / 100.0;
        attackSpinBox->blockSignals(true);
        attackSpinBox->setValue(m_attack);
        attackSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(attackSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_attack = value;
        attackSlider->blockSignals(true);
        attackSlider->setValue(static_cast<int>(value * 100));
        attackSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Release
    connect(releaseSlider, &QSlider::valueChanged, [this](int value) {
        m_release = value / 100.0;
        releaseSpinBox->blockSignals(true);
        releaseSpinBox->setValue(m_release);
        releaseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(releaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_release = value;
        releaseSlider->blockSignals(true);
        releaseSlider->setValue(static_cast<int>(value * 100));
        releaseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Ratio
    connect(ratioSlider, &QSlider::valueChanged, [this](int value) {
        m_ratio = value / 100.0;
        ratioSpinBox->blockSignals(true);
        ratioSpinBox->setValue(m_ratio);
        ratioSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(ratioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_ratio = value;
        ratioSlider->blockSignals(true);
        ratioSlider->setValue(static_cast<int>(value * 100));
        ratioSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Makeup
    connect(makeupSlider, &QSlider::valueChanged, [this](int value) {
        m_makeup = value / 10.0;
        makeupSpinBox->blockSignals(true);
        makeupSpinBox->setValue(m_makeup);
        makeupSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(makeupSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_makeup = value;
        makeupSlider->blockSignals(true);
        makeupSlider->setValue(static_cast<int>(value * 10));
        makeupSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Range
    connect(rangeSlider, &QSlider::valueChanged, [this](int value) {
        m_range = value / 10.0;
        rangeSpinBox->blockSignals(true);
        rangeSpinBox->setValue(m_range);
        rangeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rangeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_range = value;
        rangeSlider->blockSignals(true);
        rangeSlider->setValue(static_cast<int>(value * 10));
        rangeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Mode
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = index - 1;  // Offset back since -1 is first
        updateFFmpegFlags();
    });

    // Auto
    connect(autoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_autoThreshold = index + 1;  // Offset back since 1 is first
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdynamicequalizer::updateFFmpegFlags() {
    QStringList modeNames = {"listen", "cutbelow", "cutabove", "boostbelow", "boostabove"};
    QStringList dftypeNames = {"bandpass", "lowpass", "highpass", "peak"};
    QStringList tftypeNames = {"bell", "lowshelf", "highshelf"};
    QStringList autoNames = {"off", "on", "adaptive", "disabled"};
    
    QString modeName = (m_mode >= -1 && m_mode <= 3) ? modeNames[m_mode + 1] : "cutbelow";
    QString dftypeName = (m_dftype >= 0 && m_dftype < dftypeNames.size()) ? dftypeNames[m_dftype] : "bandpass";
    QString tftypeName = (m_tftype >= 0 && m_tftype < tftypeNames.size()) ? tftypeNames[m_tftype] : "bell";
    QString autoName = (m_autoThreshold >= 1 && m_autoThreshold <= 4) ? autoNames[m_autoThreshold - 1] : "off";
    
    ffmpegFlags = QString("adynamicequalizer=threshold=%1:dfrequency=%2:dqfactor=%3:tfrequency=%4:tqfactor=%5:attack=%6:release=%7:ratio=%8:makeup=%9:range=%10:mode=%11:dftype=%12:tftype=%13:auto=%14")
                      .arg(m_threshold, 0, 'f', 1)
                      .arg(m_dfrequency, 0, 'f', 1)
                      .arg(m_dqfactor, 0, 'f', 3)
                      .arg(m_tfrequency, 0, 'f', 1)
                      .arg(m_tqfactor, 0, 'f', 3)
                      .arg(m_attack, 0, 'f', 2)
                      .arg(m_release, 0, 'f', 2)
                      .arg(m_ratio, 0, 'f', 2)
                      .arg(m_makeup, 0, 'f', 1)
                      .arg(m_range, 0, 'f', 1)
                      .arg(modeName)
                      .arg(dftypeName)
                      .arg(tftypeName)
                      .arg(autoName);
}

QString FFAdynamicequalizer::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdynamicequalizer::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adynamicequalizer";
    json["threshold"] = m_threshold;
    json["dfrequency"] = m_dfrequency;
    json["dqfactor"] = m_dqfactor;
    json["tfrequency"] = m_tfrequency;
    json["tqfactor"] = m_tqfactor;
    json["attack"] = m_attack;
    json["release"] = m_release;
    json["ratio"] = m_ratio;
    json["makeup"] = m_makeup;
    json["range"] = m_range;
    json["mode"] = m_mode;
    json["dftype"] = m_dftype;
    json["tftype"] = m_tftype;
    json["autoThreshold"] = m_autoThreshold;
}

void FFAdynamicequalizer::fromJSON(const QJsonObject& json) {
    m_threshold = json["threshold"].toDouble(0.0);
    m_dfrequency = json["dfrequency"].toDouble(1000.0);
    m_dqfactor = json["dqfactor"].toDouble(1.0);
    m_tfrequency = json["tfrequency"].toDouble(1000.0);
    m_tqfactor = json["tqfactor"].toDouble(1.0);
    m_attack = json["attack"].toDouble(20.0);
    m_release = json["release"].toDouble(200.0);
    m_ratio = json["ratio"].toDouble(1.0);
    m_makeup = json["makeup"].toDouble(0.0);
    m_range = json["range"].toDouble(50.0);
    m_mode = json["mode"].toInt(0);
    m_dftype = json["dftype"].toInt(0);
    m_tftype = json["tftype"].toInt(0);
    m_autoThreshold = json["autoThreshold"].toInt(1);
    updateFFmpegFlags();
}
