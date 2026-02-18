#include "ff-alimiter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAlimiter::FFAlimiter() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAlimiter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Level In parameter (0.015625 - 64)
    auto levelInGroup = new QGroupBox("Input Level (dB)");
    auto levelInLayout = new QVBoxLayout(levelInGroup);

    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(0.015625);
    levelInSpinBox->setMaximum(64.0);
    levelInSpinBox->setValue(m_levelIn);
    levelInSpinBox->setDecimals(2);
    levelInSpinBox->setSingleStep(0.1);

    auto levelInControlLayout = new QHBoxLayout();
    levelInControlLayout->addWidget(levelInSpinBox);
    levelInControlLayout->addStretch();
    
    auto levelInMinLabel = new QLabel("-36");
    levelInControlLayout->addWidget(levelInMinLabel);

    levelInSlider = new QSlider(Qt::Horizontal);
    levelInSlider->setMinimum(16);    // 0.015625 * 1000
    levelInSlider->setMaximum(64000); // 64 * 1000
    levelInSlider->setValue(1000);    // default 1.0
    levelInControlLayout->addWidget(levelInSlider, 1);

    auto levelInMaxLabel = new QLabel("+36");
    levelInControlLayout->addWidget(levelInMaxLabel);

    levelInLayout->addLayout(levelInControlLayout);
    mainLayout->addWidget(levelInGroup);

    // Level Out parameter (0.015625 - 64)
    auto levelOutGroup = new QGroupBox("Output Level (dB)");
    auto levelOutLayout = new QVBoxLayout(levelOutGroup);

    levelOutSpinBox = new QDoubleSpinBox();
    levelOutSpinBox->setMinimum(0.015625);
    levelOutSpinBox->setMaximum(64.0);
    levelOutSpinBox->setValue(m_levelOut);
    levelOutSpinBox->setDecimals(2);
    levelOutSpinBox->setSingleStep(0.1);

    auto levelOutControlLayout = new QHBoxLayout();
    levelOutControlLayout->addWidget(levelOutSpinBox);
    levelOutControlLayout->addStretch();
    
    auto levelOutMinLabel = new QLabel("-36");
    levelOutControlLayout->addWidget(levelOutMinLabel);

    levelOutSlider = new QSlider(Qt::Horizontal);
    levelOutSlider->setMinimum(16);
    levelOutSlider->setMaximum(64000);
    levelOutSlider->setValue(1000);
    levelOutControlLayout->addWidget(levelOutSlider, 1);

    auto levelOutMaxLabel = new QLabel("+36");
    levelOutControlLayout->addWidget(levelOutMaxLabel);

    levelOutLayout->addLayout(levelOutControlLayout);
    mainLayout->addWidget(levelOutGroup);

    // Limit parameter (0.0625 - 1)
    auto limitGroup = new QGroupBox("Limit");
    auto limitLayout = new QVBoxLayout(limitGroup);

    limitSpinBox = new QDoubleSpinBox();
    limitSpinBox->setMinimum(0.0625);
    limitSpinBox->setMaximum(1.0);
    limitSpinBox->setValue(m_limit);
    limitSpinBox->setDecimals(2);
    limitSpinBox->setSingleStep(0.01);

    auto limitControlLayout = new QHBoxLayout();
    limitControlLayout->addWidget(limitSpinBox);
    limitControlLayout->addStretch();
    
    auto limitMinLabel = new QLabel("0.0625");
    limitControlLayout->addWidget(limitMinLabel);

    limitSlider = new QSlider(Qt::Horizontal);
    limitSlider->setMinimum(63);   // 0.0625 * 1000
    limitSlider->setMaximum(1000); // 1.0 * 1000
    limitSlider->setValue(1000);   // default 1.0
    limitControlLayout->addWidget(limitSlider, 1);

    auto limitMaxLabel = new QLabel("1.0");
    limitControlLayout->addWidget(limitMaxLabel);

    limitLayout->addLayout(limitControlLayout);
    mainLayout->addWidget(limitGroup);

    // Attack parameter (0.1 - 80 ms)
    auto attackGroup = new QGroupBox("Attack (ms)");
    auto attackLayout = new QVBoxLayout(attackGroup);

    attackSpinBox = new QDoubleSpinBox();
    attackSpinBox->setMinimum(0.1);
    attackSpinBox->setMaximum(80.0);
    attackSpinBox->setValue(m_attack);
    attackSpinBox->setDecimals(2);
    attackSpinBox->setSingleStep(0.1);
    attackSpinBox->setSuffix(" ms");

    auto attackControlLayout = new QHBoxLayout();
    attackControlLayout->addWidget(attackSpinBox);
    attackControlLayout->addStretch();
    
    auto attackMinLabel = new QLabel("0.1");
    attackControlLayout->addWidget(attackMinLabel);

    attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setMinimum(10);   // 0.1 * 100
    attackSlider->setMaximum(8000); // 80 * 100
    attackSlider->setValue(500);    // default 5.0
    attackControlLayout->addWidget(attackSlider, 1);

    auto attackMaxLabel = new QLabel("80");
    attackControlLayout->addWidget(attackMaxLabel);

    attackLayout->addLayout(attackControlLayout);
    mainLayout->addWidget(attackGroup);

    // Release parameter (1 - 8000 ms)
    auto releaseGroup = new QGroupBox("Release (ms)");
    auto releaseLayout = new QVBoxLayout(releaseGroup);

    releaseSpinBox = new QDoubleSpinBox();
    releaseSpinBox->setMinimum(1.0);
    releaseSpinBox->setMaximum(8000.0);
    releaseSpinBox->setValue(m_release);
    releaseSpinBox->setDecimals(2);
    releaseSpinBox->setSingleStep(1.0);
    releaseSpinBox->setSuffix(" ms");

    auto releaseControlLayout = new QHBoxLayout();
    releaseControlLayout->addWidget(releaseSpinBox);
    releaseControlLayout->addStretch();
    
    auto releaseMinLabel = new QLabel("1");
    releaseControlLayout->addWidget(releaseMinLabel);

    releaseSlider = new QSlider(Qt::Horizontal);
    releaseSlider->setMinimum(1);
    releaseSlider->setMaximum(8000);
    releaseSlider->setValue(50); // default 50
    releaseControlLayout->addWidget(releaseSlider, 1);

    auto releaseMaxLabel = new QLabel("8000");
    releaseControlLayout->addWidget(releaseMaxLabel);

    releaseLayout->addLayout(releaseControlLayout);
    mainLayout->addWidget(releaseGroup);

    // ASC Level parameter (0 - 1) - only enabled when ASC is checked
    auto ascLevelGroup = new QGroupBox("ASC Level");
    auto ascLevelLayout = new QVBoxLayout(ascLevelGroup);

    ascLevelSpinBox = new QDoubleSpinBox();
    ascLevelSpinBox->setMinimum(0.0);
    ascLevelSpinBox->setMaximum(1.0);
    ascLevelSpinBox->setValue(m_ascLevel);
    ascLevelSpinBox->setDecimals(2);
    ascLevelSpinBox->setSingleStep(0.01);

    auto ascLevelControlLayout = new QHBoxLayout();
    ascLevelControlLayout->addWidget(ascLevelSpinBox);
    ascLevelControlLayout->addStretch();
    
    auto ascLevelMinLabel = new QLabel("0.0");
    ascLevelControlLayout->addWidget(ascLevelMinLabel);

    ascLevelSlider = new QSlider(Qt::Horizontal);
    ascLevelSlider->setMinimum(0);
    ascLevelSlider->setMaximum(100);
    ascLevelSlider->setValue(50); // default 0.5
    ascLevelControlLayout->addWidget(ascLevelSlider, 1);

    auto ascLevelMaxLabel = new QLabel("1.0");
    ascLevelControlLayout->addWidget(ascLevelMaxLabel);

    ascLevelLayout->addLayout(ascLevelControlLayout);
    mainLayout->addWidget(ascLevelGroup);

    // Boolean options
    auto optionsGroup = new QGroupBox("Options");
    auto optionsLayout = new QVBoxLayout(optionsGroup);

    ascCheckBox = new QCheckBox("Enable ASC (Auto Sidechain)");
    ascCheckBox->setChecked(m_asc);
    optionsLayout->addWidget(ascCheckBox);

    levelCheckBox = new QCheckBox("Auto Level");
    levelCheckBox->setChecked(m_level);
    optionsLayout->addWidget(levelCheckBox);

    latencyCheckBox = new QCheckBox("Compensate Latency");
    latencyCheckBox->setChecked(m_latency);
    optionsLayout->addWidget(latencyCheckBox);

    mainLayout->addWidget(optionsGroup);

    // Connect level in slider and spinbox
    connect(levelInSlider, &QSlider::valueChanged, [this](int value) {
        m_levelIn = value / 1000.0;
        
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(m_levelIn);
        levelInSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelIn = value;
        
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(value * 1000));
        levelInSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect level out slider and spinbox
    connect(levelOutSlider, &QSlider::valueChanged, [this](int value) {
        m_levelOut = value / 1000.0;
        
        levelOutSpinBox->blockSignals(true);
        levelOutSpinBox->setValue(m_levelOut);
        levelOutSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelOut = value;
        
        levelOutSlider->blockSignals(true);
        levelOutSlider->setValue(static_cast<int>(value * 1000));
        levelOutSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect limit slider and spinbox
    connect(limitSlider, &QSlider::valueChanged, [this](int value) {
        m_limit = value / 1000.0;
        
        limitSpinBox->blockSignals(true);
        limitSpinBox->setValue(m_limit);
        limitSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(limitSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_limit = value;
        
        limitSlider->blockSignals(true);
        limitSlider->setValue(static_cast<int>(value * 1000));
        limitSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect attack slider and spinbox
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

    // Connect release slider and spinbox
    connect(releaseSlider, &QSlider::valueChanged, [this](int value) {
        m_release = static_cast<double>(value);
        
        releaseSpinBox->blockSignals(true);
        releaseSpinBox->setValue(m_release);
        releaseSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(releaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_release = value;
        
        releaseSlider->blockSignals(true);
        releaseSlider->setValue(static_cast<int>(value));
        releaseSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect asc level slider and spinbox
    connect(ascLevelSlider, &QSlider::valueChanged, [this](int value) {
        m_ascLevel = value / 100.0;
        
        ascLevelSpinBox->blockSignals(true);
        ascLevelSpinBox->setValue(m_ascLevel);
        ascLevelSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(ascLevelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_ascLevel = value;
        
        ascLevelSlider->blockSignals(true);
        ascLevelSlider->setValue(static_cast<int>(value * 100));
        ascLevelSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect checkboxes
    connect(ascCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_asc = checked;
        updateFFmpegFlags();
    });

    connect(levelCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_level = checked;
        updateFFmpegFlags();
    });

    connect(latencyCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_latency = checked;
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAlimiter::updateFFmpegFlags() {
    ffmpegFlags = QString("alimiter=level_in=%1:level_out=%2:limit=%3:attack=%4:release=%5:asc=%6:asc_level=%7:level=%8:latency=%9")
                      .arg(m_levelIn, 0, 'f', 6)
                      .arg(m_levelOut, 0, 'f', 6)
                      .arg(m_limit, 0, 'f', 4)
                      .arg(m_attack, 0, 'f', 1)
                      .arg(m_release, 0, 'f', 1)
                      .arg(m_asc ? 1 : 0)
                      .arg(m_ascLevel, 0, 'f', 2)
                      .arg(m_level ? 1 : 0)
                      .arg(m_latency ? 1 : 0);
}

QString FFAlimiter::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAlimiter::toJSON(QJsonObject& json) const {
    json["type"] = "ff-alimiter";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["limit"] = m_limit;
    json["attack"] = m_attack;
    json["release"] = m_release;
    json["asc"] = m_asc;
    json["asc_level"] = m_ascLevel;
    json["level"] = m_level;
    json["latency"] = m_latency;
}

void FFAlimiter::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_limit = json["limit"].toDouble(1.0);
    m_attack = json["attack"].toDouble(5.0);
    m_release = json["release"].toDouble(50.0);
    m_asc = json["asc"].toBool(false);
    m_ascLevel = json["asc_level"].toDouble(0.5);
    m_level = json["level"].toBool(true);
    m_latency = json["latency"].toBool(false);
    updateFFmpegFlags();
}
