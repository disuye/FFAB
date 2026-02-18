#include "ff-sidechaincompress.h"
#include "CollapsibleHelpSection.h"
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

FFSidechaincompress::FFSidechaincompress() {
    position = Position::MIDDLE;
}

QWidget* FFSidechaincompress::getParametersWidget() {
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
    levelInSpinBox->setValue(linearToDb(levelIn));
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
    levelInSlider->setValue(static_cast<int>(linearToDb(levelIn) * 100));
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
    thresholdSpinBox->setValue(linearToDb(threshold));
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
    thresholdSlider->setValue(static_cast<int>((linearToDb(threshold) + 60.0) * 10.0));
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
    ratioSpinBox->setValue(ratio);
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
    attackSpinBox->setValue(attack);
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
    releaseSpinBox->setValue(release);
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
    kneeSpinBox->setValue(knee);
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
    makeupSpinBox->setValue(linearToDb(makeup));
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
    makeupSlider->setValue(static_cast<int>(linearToDb(makeup) * 10.0));
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

    // Link Type
    auto linkGroup = new QGroupBox("Link");
    auto linkLayout = new QVBoxLayout(linkGroup);

    linkCombo = new QComboBox();
    linkCombo->addItem("Average", "average");
    linkCombo->addItem("Maximum", "maximum");
    linkCombo->setToolTip("How to combine channels for level detection");
    
    connect(linkCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        link = linkCombo->currentData().toString();
    });

    linkLayout->addWidget(linkCombo);
    optionsLayout->addWidget(linkGroup);

    // Detection Mode
    auto detectionGroup = new QGroupBox("Detection");
    auto detectionLayout = new QVBoxLayout(detectionGroup);

    detectionCombo = new QComboBox();
    detectionCombo->addItem("RMS (smooth)", "rms");
    detectionCombo->addItem("Peak (fast)", "peak");
    detectionCombo->setToolTip("Level detection mode");
    
    connect(detectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        detection = detectionCombo->currentData().toString();
    });

    detectionLayout->addWidget(detectionCombo);
    optionsLayout->addWidget(detectionGroup);

    // Wet/Dry Mix parameter (0 - 100%)
    auto mixGroup = new QGroupBox("Wet/Dry Mix");
    auto mixLayout = new QVBoxLayout(mixGroup);

    mixSpinBox = new QDoubleSpinBox();
    mixSpinBox->setMinimum(0.0);
    mixSpinBox->setMaximum(100.0);
    mixSpinBox->setValue(mix * 100.0);
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
    mixSlider->setValue(static_cast<int>(mix * 100));
    mixControlLayout->addWidget(mixSlider, 1);

    auto mixMaxLabel = new QLabel("100%");
    mixControlLayout->addWidget(mixMaxLabel);

    mixLayout->addLayout(mixControlLayout);
    optionsLayout->addWidget(mixGroup);

    // Collapsible help section
    auto helpSection = new CollapsibleHelpSection(
        "<b>⚠️  Multi-input filter:</b> Requires an <b>Audio Input</b> filter before this filter. "
        "The sidechain signal (loaded in Audio Input) triggers compression on the main audio.<br><br>"
        
        "<b>Common uses:</b><br>"
        "• <b>Voice ducking:</b> Use voice as sidechain to duck music (threshold ~0.1, ratio 4:1)<br>"
        "• <b>Kick pumping:</b> Use kick drum to create pumping effect (fast attack, slow release)<br>"
        "• <b>Podcast production:</b> Auto-duck background when host speaks<br><br>"
        
        "<b>Quick settings:</b><br>"
        "• <b>Gentle ducking:</b> Threshold 0.125, Ratio 2:1, Attack 20ms, Release 250ms<br>"
        "• <b>Hard ducking:</b> Threshold 0.05, Ratio 8:1, Attack 5ms, Release 500ms<br>"
        "• <b>Pumping:</b> Threshold 0.1, Ratio 4:1, Attack 1ms, Release 100ms"
    );
    optionsLayout->addWidget(helpSection);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Input Level (dB conversion)
    connect(levelInSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 100.0;
        levelIn = dbToLinear(db);
        
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(db);
        levelInSpinBox->blockSignals(false);
    });

    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        levelIn = dbToLinear(db);
        
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(db * 100));
        levelInSlider->blockSignals(false);
    });

    // Threshold (dB conversion)
    connect(thresholdSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 60.0;
        threshold = dbToLinear(db);
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(db);
        thresholdSpinBox->blockSignals(false);
    });
    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        threshold = dbToLinear(db);
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>((db + 60.0) * 10.0));
        thresholdSlider->blockSignals(false);
    });

    // Ratio
    connect(ratioSlider, &QSlider::valueChanged, [this](int value) {
        ratio = value / 100.0;
        ratioSpinBox->blockSignals(true);
        ratioSpinBox->setValue(ratio);
        ratioSpinBox->blockSignals(false);
    });
    connect(ratioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        ratio = value;
        ratioSlider->blockSignals(true);
        ratioSlider->setValue(static_cast<int>(value * 100));
        ratioSlider->blockSignals(false);
    });

    // Attack
    connect(attackSlider, &QSlider::valueChanged, [this](int value) {
        attack = value / 100.0;
        attackSpinBox->blockSignals(true);
        attackSpinBox->setValue(attack);
        attackSpinBox->blockSignals(false);
    });
    connect(attackSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        attack = value;
        attackSlider->blockSignals(true);
        attackSlider->setValue(static_cast<int>(value * 100));
        attackSlider->blockSignals(false);
    });

    // Release
    connect(releaseSlider, &QSlider::valueChanged, [this](int value) {
        release = value / 100.0;
        releaseSpinBox->blockSignals(true);
        releaseSpinBox->setValue(release);
        releaseSpinBox->blockSignals(false);
    });
    connect(releaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        release = value;
        releaseSlider->blockSignals(true);
        releaseSlider->setValue(static_cast<int>(value * 100));
        releaseSlider->blockSignals(false);
    });

    // Knee
    connect(kneeSlider, &QSlider::valueChanged, [this](int value) {
        knee = value / 100.0;
        kneeSpinBox->blockSignals(true);
        kneeSpinBox->setValue(knee);
        kneeSpinBox->blockSignals(false);
    });
    connect(kneeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        knee = value;
        kneeSlider->blockSignals(true);
        kneeSlider->setValue(static_cast<int>(value * 100));
        kneeSlider->blockSignals(false);
    });

    // Makeup (dB conversion)
    connect(makeupSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        makeup = dbToLinear(db);
        makeupSpinBox->blockSignals(true);
        makeupSpinBox->setValue(db);
        makeupSpinBox->blockSignals(false);
    });
    connect(makeupSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        makeup = dbToLinear(db);
        makeupSlider->blockSignals(true);
        makeupSlider->setValue(static_cast<int>(db * 10.0));
        makeupSlider->blockSignals(false);
    });

    // Mix (0-100% display, 0-1 internal)
    connect(mixSlider, &QSlider::valueChanged, [this](int value) {
        mix = value / 100.0;
        mixSpinBox->blockSignals(true);
        mixSpinBox->setValue(value);
        mixSpinBox->blockSignals(false);
    });
    connect(mixSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        mix = value / 100.0;
        mixSlider->blockSignals(true);
        mixSlider->setValue(static_cast<int>(value));
        mixSlider->blockSignals(false);
    });

    return parametersWidget;
}

void FFSidechaincompress::updateFFmpegFlags() {
    // No-op: flags generated dynamically in buildFFmpegFlags()
}

QString FFSidechaincompress::buildFFmpegFlags() const {
    QStringList params;
    
    if (levelIn != 1.0) params << QString("level_in=%1").arg(levelIn);
    params << QString("threshold=%1").arg(threshold);
    params << QString("ratio=%1").arg(ratio);
    params << QString("attack=%1").arg(attack);
    params << QString("release=%1").arg(release);
    if (makeup != 1.0) params << QString("makeup=%1").arg(makeup);
    if (knee != 2.82843) params << QString("knee=%1").arg(knee);
    if (link != "average") params << QString("link=%1").arg(link);
    if (detection != "rms") params << QString("detection=%1").arg(detection);
    if (mix != 1.0) params << QString("mix=%1").arg(mix);
    
    return QString("[0:a][%1:a]sidechaincompress=%2")
        .arg(sidechainInputIndex)
        .arg(params.join(":"));
}

void FFSidechaincompress::toJSON(QJsonObject& json) const {
    json["type"] = "ff-sidechaincompress";
    json["levelIn"] = levelIn;
    json["threshold"] = threshold;
    json["ratio"] = ratio;
    json["attack"] = attack;
    json["release"] = release;
    json["makeup"] = makeup;
    json["knee"] = knee;
    json["link"] = link;
    json["detection"] = detection;
    json["mix"] = mix;
}

void FFSidechaincompress::fromJSON(const QJsonObject& json) {
    levelIn = json["levelIn"].toDouble(1.0);
    threshold = json["threshold"].toDouble(0.125);
    ratio = json["ratio"].toDouble(2.0);
    attack = json["attack"].toDouble(20.0);
    release = json["release"].toDouble(250.0);
    makeup = json["makeup"].toDouble(1.0);
    knee = json["knee"].toDouble(2.82843);
    link = json["link"].toString("average");
    detection = json["detection"].toString("rms");
    mix = json["mix"].toDouble(1.0);
    
    // Update UI if it exists (needs dB conversion for levelIn)
    if (levelInSpinBox) {
        double db = 20.0 * std::log10(levelIn);
        levelInSpinBox->setValue(db);
    }
    if (levelInSlider) {
        double db = 20.0 * std::log10(levelIn);
        levelInSlider->setValue(static_cast<int>(db * 100));
    }
    if (thresholdSpinBox) {
        double db = 20.0 * std::log10(std::max(threshold, 1e-10));
        thresholdSpinBox->setValue(db);
    }
    if (thresholdSlider) {
        double db = 20.0 * std::log10(std::max(threshold, 1e-10));
        thresholdSlider->setValue(static_cast<int>((db + 60.0) * 10.0));
    }
    if (ratioSpinBox) ratioSpinBox->setValue(ratio);
    if (ratioSlider) ratioSlider->setValue(static_cast<int>(ratio * 100));
    if (attackSpinBox) attackSpinBox->setValue(attack);
    if (attackSlider) attackSlider->setValue(static_cast<int>(attack * 100));
    if (releaseSpinBox) releaseSpinBox->setValue(release);
    if (releaseSlider) releaseSlider->setValue(static_cast<int>(release * 100));
    if (kneeSpinBox) kneeSpinBox->setValue(knee);
    if (kneeSlider) kneeSlider->setValue(static_cast<int>(knee * 100));
    if (makeupSpinBox) makeupSpinBox->setValue(20.0 * std::log10(std::max(makeup, 1e-10)));
    if (makeupSlider) makeupSlider->setValue(static_cast<int>(20.0 * std::log10(std::max(makeup, 1e-10)) * 10.0));
    if (mixSpinBox) mixSpinBox->setValue(mix * 100.0);
    if (mixSlider) mixSlider->setValue(static_cast<int>(mix * 100));
    if (linkCombo) {
        int idx = linkCombo->findData(link);
        if (idx >= 0) linkCombo->setCurrentIndex(idx);
    }
    if (detectionCombo) {
        int idx = detectionCombo->findData(detection);
        if (idx >= 0) detectionCombo->setCurrentIndex(idx);
    }
}