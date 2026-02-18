#include "ff-agate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAgate::FFAgate() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAgate::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(8);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Mode
    auto modeGroup = new QGroupBox("Mode");
    auto modeLayout = new QHBoxLayout(modeGroup);
    modeCombo = new QComboBox();
    modeCombo->addItem("Downward", 0);
    modeCombo->addItem("Upward", 1);
    modeCombo->setCurrentIndex(m_mode);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = modeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });
    modeLayout->addWidget(modeCombo);
    mainLayout->addWidget(modeGroup);

    // Range (-96 to 0 dB)
    auto rangeGroup = new QGroupBox("Range (dB)");
    auto rangeLayout = new QVBoxLayout(rangeGroup);

    rangeSpinBox = new QDoubleSpinBox();
    rangeSpinBox->setMinimum(-96.0);
    rangeSpinBox->setMaximum(0.0);
    rangeSpinBox->setValue(linearToDb(m_range));
    rangeSpinBox->setDecimals(1);
    rangeSpinBox->setSingleStep(0.1);
    rangeSpinBox->setSuffix(" dB");

    auto rangeControlLayout = new QHBoxLayout();
    rangeControlLayout->addWidget(rangeSpinBox);
    rangeControlLayout->addStretch();

    auto rangeMinLabel = new QLabel("-96");
    rangeControlLayout->addWidget(rangeMinLabel);

    rangeSlider = new QSlider(Qt::Horizontal);
    rangeSlider->setMinimum(0);
    rangeSlider->setMaximum(960);
    rangeSlider->setValue(static_cast<int>((linearToDb(m_range) + 96.0) * 10.0));
    rangeControlLayout->addWidget(rangeSlider, 1);

    auto rangeMaxLabel = new QLabel("0");
    rangeControlLayout->addWidget(rangeMaxLabel);

    rangeLayout->addLayout(rangeControlLayout);
    mainLayout->addWidget(rangeGroup);

    // Threshold (-60 to 0 dB)
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
    mainLayout->addWidget(thresholdGroup);

    // Ratio (1 - 9000)
    auto ratioGroup = new QGroupBox("Ratio");
    auto ratioLayout = new QVBoxLayout(ratioGroup);

    ratioSpinBox = new QDoubleSpinBox();
    ratioSpinBox->setMinimum(1.0);
    ratioSpinBox->setMaximum(9000.0);
    ratioSpinBox->setValue(m_ratio);
    ratioSpinBox->setDecimals(2);
    ratioSpinBox->setSuffix(":1");

    auto ratioControlLayout = new QHBoxLayout();
    ratioControlLayout->addWidget(ratioSpinBox);
    ratioControlLayout->addStretch();
    
    auto ratioMinLabel = new QLabel("1:1");
    ratioControlLayout->addWidget(ratioMinLabel);

    ratioSlider = new QSlider(Qt::Horizontal);
    ratioSlider->setMinimum(100);
    ratioSlider->setMaximum(90000);
    ratioSlider->setValue(200);
    ratioControlLayout->addWidget(ratioSlider, 1);
    
    auto ratioMaxLabel = new QLabel("9000:1");
    ratioControlLayout->addWidget(ratioMaxLabel);

    ratioLayout->addLayout(ratioControlLayout);
    mainLayout->addWidget(ratioGroup);

    // Attack (0.01 - 9000 ms)
    auto attackGroup = new QGroupBox("Attack (ms)");
    auto attackLayout = new QVBoxLayout(attackGroup);

    attackSpinBox = new QDoubleSpinBox();
    attackSpinBox->setMinimum(0.01);
    attackSpinBox->setMaximum(9000.0);
    attackSpinBox->setValue(m_attack);
    attackSpinBox->setDecimals(2);
    attackSpinBox->setSuffix(" ms");

    auto attackControlLayout = new QHBoxLayout();
    attackControlLayout->addWidget(attackSpinBox);
    attackControlLayout->addStretch();
    
    auto attackMinLabel = new QLabel("0.01");
    attackControlLayout->addWidget(attackMinLabel);

    attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setMinimum(1);
    attackSlider->setMaximum(900000);
    attackSlider->setValue(2000);
    attackControlLayout->addWidget(attackSlider, 1);
    
    auto attackMaxLabel = new QLabel("9000");
    attackControlLayout->addWidget(attackMaxLabel);

    attackLayout->addLayout(attackControlLayout);
    mainLayout->addWidget(attackGroup);

    // Release (0.01 - 9000 ms)
    auto releaseGroup = new QGroupBox("Release (ms)");
    auto releaseLayout = new QVBoxLayout(releaseGroup);

    releaseSpinBox = new QDoubleSpinBox();
    releaseSpinBox->setMinimum(0.01);
    releaseSpinBox->setMaximum(9000.0);
    releaseSpinBox->setValue(m_release);
    releaseSpinBox->setDecimals(2);
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
    mainLayout->addWidget(releaseGroup);

    // Makeup Gain (1 - 64)
    auto makeupGroup = new QGroupBox("Makeup Gain");
    auto makeupLayout = new QVBoxLayout(makeupGroup);

    makeupSpinBox = new QDoubleSpinBox();
    makeupSpinBox->setMinimum(1.0);
    makeupSpinBox->setMaximum(64.0);
    makeupSpinBox->setValue(m_makeup);
    makeupSpinBox->setDecimals(2);

    auto makeupControlLayout = new QHBoxLayout();
    makeupControlLayout->addWidget(makeupSpinBox);
    makeupControlLayout->addStretch();
    
    auto makeupMinLabel = new QLabel("1.0");
    makeupControlLayout->addWidget(makeupMinLabel);

    makeupSlider = new QSlider(Qt::Horizontal);
    makeupSlider->setMinimum(1000);
    makeupSlider->setMaximum(64000);
    makeupSlider->setValue(1000);
    makeupControlLayout->addWidget(makeupSlider, 1);
    
    auto makeupMaxLabel = new QLabel("64.0");
    makeupControlLayout->addWidget(makeupMaxLabel);

    makeupLayout->addLayout(makeupControlLayout);
    mainLayout->addWidget(makeupGroup);

    // Knee (1 - 8 dB)
    auto kneeGroup = new QGroupBox("Knee (dB)");
    auto kneeLayout = new QVBoxLayout(kneeGroup);

    kneeSpinBox = new QDoubleSpinBox();
    kneeSpinBox->setMinimum(1.0);
    kneeSpinBox->setMaximum(8.0);
    kneeSpinBox->setValue(m_knee);
    kneeSpinBox->setDecimals(2);
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
    mainLayout->addWidget(kneeGroup);

    // Link and Detection
    auto optionsGroup = new QGroupBox("Options");
    auto optionsLayout = new QHBoxLayout(optionsGroup);
    
    optionsLayout->addWidget(new QLabel("Link:"));
    linkCombo = new QComboBox();
    linkCombo->addItem("Average", 0);
    linkCombo->addItem("Maximum", 1);
    linkCombo->setCurrentIndex(m_link);
    connect(linkCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_link = linkCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });
    optionsLayout->addWidget(linkCombo);
    
    optionsLayout->addWidget(new QLabel("Detection:"));
    detectionCombo = new QComboBox();
    detectionCombo->addItem("Peak", 0);
    detectionCombo->addItem("RMS", 1);
    detectionCombo->setCurrentIndex(m_detection);
    connect(detectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_detection = detectionCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });
    optionsLayout->addWidget(detectionCombo);
    mainLayout->addWidget(optionsGroup);

    // Connect sliders and spinboxes
    // Range (dB conversion)
    connect(rangeSlider, &QSlider::valueChanged, [this, dbToLinear](int v) {
        double db = (v / 10.0) - 96.0;
        m_range = dbToLinear(db);
        rangeSpinBox->blockSignals(true);
        rangeSpinBox->setValue(db);
        rangeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rangeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_range = dbToLinear(db);
        rangeSlider->blockSignals(true);
        rangeSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        rangeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Threshold (dB conversion)
    connect(thresholdSlider, &QSlider::valueChanged, [this, dbToLinear](int v) {
        double db = (v / 10.0) - 60.0;
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

    connect(ratioSlider, &QSlider::valueChanged, [this](int v) {
        m_ratio = v / 100.0;
        ratioSpinBox->blockSignals(true);
        ratioSpinBox->setValue(m_ratio);
        ratioSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(ratioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_ratio = v;
        ratioSlider->blockSignals(true);
        ratioSlider->setValue(v * 100);
        ratioSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(attackSlider, &QSlider::valueChanged, [this](int v) {
        m_attack = v / 100.0;
        attackSpinBox->blockSignals(true);
        attackSpinBox->setValue(m_attack);
        attackSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(attackSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_attack = v;
        attackSlider->blockSignals(true);
        attackSlider->setValue(v * 100);
        attackSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(releaseSlider, &QSlider::valueChanged, [this](int v) {
        m_release = v / 100.0;
        releaseSpinBox->blockSignals(true);
        releaseSpinBox->setValue(m_release);
        releaseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(releaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_release = v;
        releaseSlider->blockSignals(true);
        releaseSlider->setValue(v * 100);
        releaseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(makeupSlider, &QSlider::valueChanged, [this](int v) {
        m_makeup = v / 1000.0;
        makeupSpinBox->blockSignals(true);
        makeupSpinBox->setValue(m_makeup);
        makeupSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(makeupSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_makeup = v;
        makeupSlider->blockSignals(true);
        makeupSlider->setValue(v * 1000);
        makeupSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(kneeSlider, &QSlider::valueChanged, [this](int v) {
        m_knee = v / 100.0;
        kneeSpinBox->blockSignals(true);
        kneeSpinBox->setValue(m_knee);
        kneeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(kneeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_knee = v;
        kneeSlider->blockSignals(true);
        kneeSlider->setValue(v * 100);
        kneeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    mainLayout->addStretch();
    return parametersWidget;
}

void FFAgate::updateFFmpegFlags() {
    QStringList modeNames = {"downward", "upward"};
    QStringList linkNames = {"average", "maximum"};
    QStringList detectionNames = {"peak", "rms"};
    
    QString modeName = (m_mode < modeNames.size()) ? modeNames[m_mode] : "downward";
    QString linkName = (m_link < linkNames.size()) ? linkNames[m_link] : "average";
    QString detectionName = (m_detection < detectionNames.size()) ? detectionNames[m_detection] : "rms";
    
    ffmpegFlags = QString("agate=level_in=%1:mode=%2:range=%3:threshold=%4:ratio=%5:attack=%6:release=%7:makeup=%8:knee=%9:detection=%10:link=%11")
                      .arg(m_levelIn, 0, 'f', 6)
                      .arg(modeName)
                      .arg(m_range, 0, 'f', 4)
                      .arg(m_threshold, 0, 'f', 4)
                      .arg(m_ratio, 0, 'f', 1)
                      .arg(m_attack, 0, 'f', 2)
                      .arg(m_release, 0, 'f', 2)
                      .arg(m_makeup, 0, 'f', 2)
                      .arg(m_knee, 0, 'f', 2)
                      .arg(detectionName)
                      .arg(linkName);
}

QString FFAgate::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAgate::toJSON(QJsonObject& json) const {
    json["type"] = "ff-agate";
    json["level_in"] = m_levelIn;
    json["mode"] = m_mode;
    json["range"] = m_range;
    json["threshold"] = m_threshold;
    json["ratio"] = m_ratio;
    json["attack"] = m_attack;
    json["release"] = m_release;
    json["makeup"] = m_makeup;
    json["knee"] = m_knee;
    json["detection"] = m_detection;
    json["link"] = m_link;
    json["level_sc"] = m_levelSc;
}

void FFAgate::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_mode = json["mode"].toInt(0);
    m_range = json["range"].toDouble(0.06125);
    m_threshold = json["threshold"].toDouble(0.125);
    m_ratio = json["ratio"].toDouble(2.0);
    m_attack = json["attack"].toDouble(20.0);
    m_release = json["release"].toDouble(250.0);
    m_makeup = json["makeup"].toDouble(1.0);
    m_knee = json["knee"].toDouble(2.82843);
    m_detection = json["detection"].toInt(1);
    m_link = json["link"].toInt(0);
    m_levelSc = json["level_sc"].toDouble(1.0);
    updateFFmpegFlags();
}
