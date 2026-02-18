#include "ff-acompressor.h"
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

FFAcompressor::FFAcompressor() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcompressor::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: DYNAMICS ==========
    auto dynamicsTab = new QWidget();
    auto dynamicsLayout = new QVBoxLayout(dynamicsTab);

    // Input Level parameter (-36 to +36 dB)
    auto levelInGroup = new QGroupBox("Input Level (dB)");
    auto levelInLayout = new QVBoxLayout(levelInGroup);

    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(-36.0);
    levelInSpinBox->setMaximum(36.0);
    levelInSpinBox->setValue(linearToDb(m_levelIn));
    levelInSpinBox->setDecimals(2);
    levelInSpinBox->setSingleStep(0.1);
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
    dynamicsLayout->addWidget(levelInGroup);

    // Threshold parameter (-60 to 0 dB)
    auto thresholdGroup = new QGroupBox("Threshold (dB)");
    auto thresholdLayout = new QVBoxLayout(thresholdGroup);

    thresholdSpinBox = new QDoubleSpinBox();
    thresholdSpinBox->setMinimum(-60.0);
    thresholdSpinBox->setMaximum(0.0);
    thresholdSpinBox->setValue(linearToDb(m_threshold));
    thresholdSpinBox->setDecimals(1);
    thresholdSpinBox->setSingleStep(0.1);
    thresholdSpinBox->setSuffix(" dB");

    auto thresholdControlLayout = new QHBoxLayout();
    thresholdControlLayout->addWidget(thresholdSpinBox);
    thresholdControlLayout->addStretch();

    auto thresholdMinLabel = new QLabel("-60");
    thresholdControlLayout->addWidget(thresholdMinLabel);

    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setMinimum(0);
    thresholdSlider->setMaximum(600);
    thresholdSlider->setValue(static_cast<int>((linearToDb(m_threshold) + 60.0) * 10.0));
    thresholdControlLayout->addWidget(thresholdSlider, 1);

    auto thresholdMaxLabel = new QLabel("0");
    thresholdControlLayout->addWidget(thresholdMaxLabel);

    thresholdLayout->addLayout(thresholdControlLayout);
    dynamicsLayout->addWidget(thresholdGroup);

    // Ratio parameter (1 - 20)
    auto ratioGroup = new QGroupBox("Ratio");
    auto ratioLayout = new QVBoxLayout(ratioGroup);

    ratioSpinBox = new QDoubleSpinBox();
    ratioSpinBox->setMinimum(1.0);
    ratioSpinBox->setMaximum(20.0);
    ratioSpinBox->setValue(m_ratio);
    ratioSpinBox->setDecimals(2);
    ratioSpinBox->setSingleStep(0.1);
    ratioSpinBox->setSuffix(":1");

    auto ratioControlLayout = new QHBoxLayout();
    ratioControlLayout->addWidget(ratioSpinBox);
    ratioControlLayout->addStretch();
    
    auto ratioMinLabel = new QLabel("1:1");
    ratioControlLayout->addWidget(ratioMinLabel);

    ratioSlider = new QSlider(Qt::Horizontal);
    ratioSlider->setMinimum(100);
    ratioSlider->setMaximum(2000);
    ratioSlider->setValue(200);
    ratioControlLayout->addWidget(ratioSlider, 1);

    auto ratioMaxLabel = new QLabel("20:1");
    ratioControlLayout->addWidget(ratioMaxLabel);

    ratioLayout->addLayout(ratioControlLayout);
    dynamicsLayout->addWidget(ratioGroup);

    // Attack parameter (0.01 - 2000 ms)
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
    
    auto attackMinLabel = new QLabel("0.01");
    attackControlLayout->addWidget(attackMinLabel);

    attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setMinimum(1);
    attackSlider->setMaximum(200000);
    attackSlider->setValue(2000);
    attackControlLayout->addWidget(attackSlider, 1);

    auto attackMaxLabel = new QLabel("2000");
    attackControlLayout->addWidget(attackMaxLabel);

    attackLayout->addLayout(attackControlLayout);
    dynamicsLayout->addWidget(attackGroup);

    // Release parameter (0.01 - 9000 ms)
    auto releaseGroup = new QGroupBox("Release (ms)");
    auto releaseLayout = new QVBoxLayout(releaseGroup);

    releaseSpinBox = new QDoubleSpinBox();
    releaseSpinBox->setMinimum(0.01);
    releaseSpinBox->setMaximum(9000.0);
    releaseSpinBox->setValue(m_release);
    releaseSpinBox->setDecimals(2);
    releaseSpinBox->setSingleStep(1.0);
    releaseSpinBox->setSuffix(" ms");

    auto releaseControlLayout = new QHBoxLayout();
    releaseControlLayout->addWidget(releaseSpinBox);
    releaseControlLayout->addStretch();
    
    auto releaseMinLabel = new QLabel("0.01");
    releaseControlLayout->addWidget(releaseMinLabel);

    releaseSlider = new QSlider(Qt::Horizontal);
    releaseSlider->setMinimum(1);
    releaseSlider->setMaximum(900000);
    releaseSlider->setValue(25000);
    releaseControlLayout->addWidget(releaseSlider, 1);

    auto releaseMaxLabel = new QLabel("9000");
    releaseControlLayout->addWidget(releaseMaxLabel);

    releaseLayout->addLayout(releaseControlLayout);
    dynamicsLayout->addWidget(releaseGroup);

    // Knee parameter (1 - 8 dB)
    auto kneeGroup = new QGroupBox("Knee (dB)");
    auto kneeLayout = new QVBoxLayout(kneeGroup);

    kneeSpinBox = new QDoubleSpinBox();
    kneeSpinBox->setMinimum(1.0);
    kneeSpinBox->setMaximum(8.0);
    kneeSpinBox->setValue(m_knee);
    kneeSpinBox->setDecimals(2);
    kneeSpinBox->setSingleStep(0.1);
    kneeSpinBox->setSuffix(" dB");

    auto kneeControlLayout = new QHBoxLayout();
    kneeControlLayout->addWidget(kneeSpinBox);
    kneeControlLayout->addStretch();
    
    auto kneeMinLabel = new QLabel("1.0");
    kneeControlLayout->addWidget(kneeMinLabel);

    kneeSlider = new QSlider(Qt::Horizontal);
    kneeSlider->setMinimum(100);
    kneeSlider->setMaximum(800);
    kneeSlider->setValue(283);
    kneeControlLayout->addWidget(kneeSlider, 1);

    auto kneeMaxLabel = new QLabel("8.0");
    kneeControlLayout->addWidget(kneeMaxLabel);

    kneeLayout->addLayout(kneeControlLayout);
    dynamicsLayout->addWidget(kneeGroup);

    // Makeup Gain parameter (0 to +36 dB)
    auto makeupGroup = new QGroupBox("Makeup Gain (dB)");
    auto makeupLayout = new QVBoxLayout(makeupGroup);

    makeupSpinBox = new QDoubleSpinBox();
    makeupSpinBox->setMinimum(0.0);
    makeupSpinBox->setMaximum(36.0);
    makeupSpinBox->setValue(linearToDb(m_makeup));
    makeupSpinBox->setDecimals(1);
    makeupSpinBox->setSingleStep(0.1);
    makeupSpinBox->setSuffix(" dB");

    auto makeupControlLayout = new QHBoxLayout();
    makeupControlLayout->addWidget(makeupSpinBox);
    makeupControlLayout->addStretch();

    auto makeupMinLabel = new QLabel("0");
    makeupControlLayout->addWidget(makeupMinLabel);

    makeupSlider = new QSlider(Qt::Horizontal);
    makeupSlider->setMinimum(0);
    makeupSlider->setMaximum(360);
    makeupSlider->setValue(static_cast<int>(linearToDb(m_makeup) * 10.0));
    makeupControlLayout->addWidget(makeupSlider, 1);

    auto makeupMaxLabel = new QLabel("+36");
    makeupControlLayout->addWidget(makeupMaxLabel);

    makeupLayout->addLayout(makeupControlLayout);
    dynamicsLayout->addWidget(makeupGroup);

    dynamicsLayout->addStretch();
    tabWidget->addTab(dynamicsTab, "Dynamics");

    // ========== TAB 2: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Mode selection
    auto modeGroup = new QGroupBox("Mode");
    auto modeLayout = new QVBoxLayout(modeGroup);

    modeCombo = new QComboBox();
    modeCombo->addItem("Downward", 0);
    modeCombo->addItem("Upward", 1);
    modeCombo->setCurrentIndex(m_mode);
    
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = modeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    modeLayout->addWidget(modeCombo);
    optionsLayout->addWidget(modeGroup);

    // Link Type
    auto linkGroup = new QGroupBox("Link");
    auto linkLayout = new QVBoxLayout(linkGroup);

    linkCombo = new QComboBox();
    linkCombo->addItem("Average", 0);
    linkCombo->addItem("Maximum", 1);
    linkCombo->setCurrentIndex(m_link);
    
    connect(linkCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_link = linkCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    linkLayout->addWidget(linkCombo);
    optionsLayout->addWidget(linkGroup);

    // Detection Mode
    auto detectionGroup = new QGroupBox("Detection");
    auto detectionLayout = new QVBoxLayout(detectionGroup);

    detectionCombo = new QComboBox();
    detectionCombo->addItem("Peak", 0);
    detectionCombo->addItem("RMS", 1);
    detectionCombo->setCurrentIndex(m_detection);
    
    connect(detectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_detection = detectionCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    detectionLayout->addWidget(detectionCombo);
    optionsLayout->addWidget(detectionGroup);

    // // Sidechain Level parameter (-36 to +36 dB)
    // auto levelScGroup = new QGroupBox("Sidechain Level (dB)");
    // auto levelScLayout = new QVBoxLayout(levelScGroup);

    // levelScSpinBox = new QDoubleSpinBox();
    // levelScSpinBox->setMinimum(-36.0);
    // levelScSpinBox->setMaximum(36.0);
    // levelScSpinBox->setValue(linearToDb(m_levelSc));
    // levelScSpinBox->setDecimals(2);
    // levelScSpinBox->setSingleStep(0.1);
    // levelScSpinBox->setSuffix(" dB");

    // auto levelScControlLayout = new QHBoxLayout();
    // levelScControlLayout->addWidget(levelScSpinBox);
    // levelScControlLayout->addStretch();
    
    // auto levelScMinLabel = new QLabel("-36");
    // levelScControlLayout->addWidget(levelScMinLabel);

    // levelScSlider = new QSlider(Qt::Horizontal);
    // levelScSlider->setMinimum(-3600);
    // levelScSlider->setMaximum(3600);
    // levelScSlider->setValue(static_cast<int>(linearToDb(m_levelSc) * 100));
    // levelScControlLayout->addWidget(levelScSlider, 1);

    // auto levelScMaxLabel = new QLabel("+36");
    // levelScControlLayout->addWidget(levelScMaxLabel);

    // levelScLayout->addLayout(levelScControlLayout);
    // optionsLayout->addWidget(levelScGroup);

    // Wet/Dry Mix parameter (0 - 100%)
    auto mixGroup = new QGroupBox("Wet/Dry Mix");
    auto mixLayout = new QVBoxLayout(mixGroup);

    mixSpinBox = new QDoubleSpinBox();
    mixSpinBox->setMinimum(0.0);
    mixSpinBox->setMaximum(100.0);
    mixSpinBox->setValue(m_mix * 100.0);  // Convert 0-1 to 0-100
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

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

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

    // Threshold (dB conversion)
    connect(thresholdSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 60.0;
        m_threshold = dbToLinear(db);
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(db);
        thresholdSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_threshold = dbToLinear(db);
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>((db + 60.0) * 10.0));
        thresholdSlider->blockSignals(false);
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

    // Knee
    connect(kneeSlider, &QSlider::valueChanged, [this](int value) {
        m_knee = value / 100.0;
        kneeSpinBox->blockSignals(true);
        kneeSpinBox->setValue(m_knee);
        kneeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(kneeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_knee = value;
        kneeSlider->blockSignals(true);
        kneeSlider->setValue(static_cast<int>(value * 100));
        kneeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Makeup (dB conversion)
    connect(makeupSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_makeup = dbToLinear(db);
        makeupSpinBox->blockSignals(true);
        makeupSpinBox->setValue(db);
        makeupSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(makeupSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_makeup = dbToLinear(db);
        makeupSlider->blockSignals(true);
        makeupSlider->setValue(static_cast<int>(db * 10.0));
        makeupSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // // Sidechain Level (dB conversion)
    // connect(levelScSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
    //     double db = value / 100.0;
    //     m_levelSc = dbToLinear(db);
        
    //     levelScSpinBox->blockSignals(true);
    //     levelScSpinBox->setValue(db);
    //     levelScSpinBox->blockSignals(false);
    //     updateFFmpegFlags();
    // });

    // connect(levelScSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
    //     m_levelSc = dbToLinear(db);
        
    //     levelScSlider->blockSignals(true);
    //     levelScSlider->setValue(static_cast<int>(db * 100));
    //     levelScSlider->blockSignals(false);
    //     updateFFmpegFlags();
    // });

    // Mix (0-100% display, 0-1 internal)
    connect(mixSlider, &QSlider::valueChanged, [this](int value) {
        m_mix = value / 100.0;  // Convert to 0-1
        mixSpinBox->blockSignals(true);
        mixSpinBox->setValue(value);  // Display as 0-100
        mixSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(mixSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_mix = value / 100.0;  // Convert to 0-1
        mixSlider->blockSignals(true);
        mixSlider->setValue(static_cast<int>(value));  // Slider is 0-100
        mixSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAcompressor::updateFFmpegFlags() {
    QStringList modeNames = {"downward", "upward"};
    QStringList linkNames = {"average", "maximum"};
    QStringList detectionNames = {"peak", "rms"};
    
    QString modeName = (m_mode >= 0 && m_mode < modeNames.size()) ? modeNames[m_mode] : "downward";
    QString linkName = (m_link >= 0 && m_link < linkNames.size()) ? linkNames[m_link] : "average";
    QString detectionName = (m_detection >= 0 && m_detection < detectionNames.size()) ? detectionNames[m_detection] : "rms";
    
    ffmpegFlags = QString("acompressor=level_in=%1:mode=%2:threshold=%3:ratio=%4:attack=%5:release=%6:makeup=%7:knee=%8:link=%9:detection=%10:mix=%11")
                      .arg(m_levelIn, 0, 'f', 6)
                      .arg(modeName)
                      .arg(m_threshold, 0, 'f', 6)
                      .arg(m_ratio, 0, 'f', 1)
                      .arg(m_attack, 0, 'f', 2)
                      .arg(m_release, 0, 'f', 2)
                      .arg(m_makeup, 0, 'f', 2)
                      .arg(m_knee, 0, 'f', 2)
                      .arg(linkName)
                      .arg(detectionName)
                      .arg(m_mix, 0, 'f', 2);
}

QString FFAcompressor::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcompressor::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acompressor";
    json["level_in"] = m_levelIn;
    json["mode"] = m_mode;
    json["threshold"] = m_threshold;
    json["ratio"] = m_ratio;
    json["attack"] = m_attack;
    json["release"] = m_release;
    json["makeup"] = m_makeup;
    json["knee"] = m_knee;
    json["link"] = m_link;
    json["detection"] = m_detection;
    json["level_sc"] = m_levelSc;
    json["mix"] = m_mix;
}

void FFAcompressor::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_mode = json["mode"].toInt(0);
    m_threshold = json["threshold"].toDouble(0.125);
    m_ratio = json["ratio"].toDouble(2.0);
    m_attack = json["attack"].toDouble(20.0);
    m_release = json["release"].toDouble(250.0);
    m_makeup = json["makeup"].toDouble(1.0);
    m_knee = json["knee"].toDouble(2.82843);
    m_link = json["link"].toInt(0);
    m_detection = json["detection"].toInt(1);
    m_levelSc = json["level_sc"].toDouble(1.0);
    m_mix = json["mix"].toDouble(1.0);
    updateFFmpegFlags();
}