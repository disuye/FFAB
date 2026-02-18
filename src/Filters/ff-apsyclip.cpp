#include "ff-apsyclip.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFApsyclip::FFApsyclip() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFApsyclip::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // dB conversion lambdas
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -120.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Level In parameter (-36 to +36 dB)
    auto levelInGroup = new QGroupBox("Input Level");
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
    levelInSlider->setMinimum(-360);
    levelInSlider->setMaximum(360);
    levelInSlider->setValue(static_cast<int>(linearToDb(m_levelIn) * 10));
    levelInControlLayout->addWidget(levelInSlider, 1);
    levelInControlLayout->addWidget(new QLabel("+36 dB"));

    levelInLayout->addLayout(levelInControlLayout);
    mainLayout->addWidget(levelInGroup);

    // Level Out parameter (-36 to +36 dB)
    auto levelOutGroup = new QGroupBox("Output Level");
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
    levelOutSlider->setMinimum(-360);
    levelOutSlider->setMaximum(360);
    levelOutSlider->setValue(static_cast<int>(linearToDb(m_levelOut) * 10));
    levelOutControlLayout->addWidget(levelOutSlider, 1);
    levelOutControlLayout->addWidget(new QLabel("+36 dB"));

    levelOutLayout->addLayout(levelOutControlLayout);
    mainLayout->addWidget(levelOutGroup);

    // Clip parameter (-36 to 0 dB)
    auto clipGroup = new QGroupBox("Clip Level");
    auto clipLayout = new QVBoxLayout(clipGroup);

    clipSpinBox = new QDoubleSpinBox();
    clipSpinBox->setMinimum(-36.0);
    clipSpinBox->setMaximum(0.0);
    clipSpinBox->setValue(linearToDb(m_clip));
    clipSpinBox->setDecimals(1);
    clipSpinBox->setSingleStep(0.5);
    clipSpinBox->setSuffix(" dB");

    auto clipControlLayout = new QHBoxLayout();
    clipControlLayout->addWidget(clipSpinBox);
    clipControlLayout->addStretch();
    clipControlLayout->addWidget(new QLabel("-36"));

    clipSlider = new QSlider(Qt::Horizontal);
    clipSlider->setMinimum(-360);
    clipSlider->setMaximum(0);
    clipSlider->setValue(static_cast<int>(linearToDb(m_clip) * 10));
    clipControlLayout->addWidget(clipSlider, 1);
    clipControlLayout->addWidget(new QLabel("0 dB"));

    auto clipHint = new QLabel("Clipping threshold (0 dB = full scale)");
    clipHint->setStyleSheet("color: gray; font-size: 11px;");
    clipLayout->addLayout(clipControlLayout);
    clipLayout->addWidget(clipHint);
    mainLayout->addWidget(clipGroup);

    // Adaptive parameter (0 - 100%)
    auto adaptiveGroup = new QGroupBox("Adaptive Distortion");
    auto adaptiveLayout = new QVBoxLayout(adaptiveGroup);

    adaptiveSpinBox = new QDoubleSpinBox();
    adaptiveSpinBox->setMinimum(0.0);
    adaptiveSpinBox->setMaximum(100.0);
    adaptiveSpinBox->setValue(m_adaptive * 100.0);
    adaptiveSpinBox->setDecimals(1);
    adaptiveSpinBox->setSingleStep(1.0);
    adaptiveSpinBox->setSuffix(" %");

    auto adaptiveControlLayout = new QHBoxLayout();
    adaptiveControlLayout->addWidget(adaptiveSpinBox);
    adaptiveControlLayout->addStretch();
    adaptiveControlLayout->addWidget(new QLabel("0%"));

    adaptiveSlider = new QSlider(Qt::Horizontal);
    adaptiveSlider->setMinimum(0);
    adaptiveSlider->setMaximum(100);
    adaptiveSlider->setValue(static_cast<int>(m_adaptive * 100));
    adaptiveControlLayout->addWidget(adaptiveSlider, 1);
    adaptiveControlLayout->addWidget(new QLabel("100%"));

    adaptiveLayout->addLayout(adaptiveControlLayout);
    mainLayout->addWidget(adaptiveGroup);

    // Iterations parameter (1 - 20)
    auto iterationsGroup = new QGroupBox("Iterations");
    auto iterationsLayout = new QVBoxLayout(iterationsGroup);

    iterationsSpinBox = new QDoubleSpinBox();
    iterationsSpinBox->setMinimum(1);
    iterationsSpinBox->setMaximum(20);
    iterationsSpinBox->setValue(m_iterations);
    iterationsSpinBox->setDecimals(0);

    auto iterationsControlLayout = new QHBoxLayout();
    iterationsControlLayout->addWidget(iterationsSpinBox);
    iterationsControlLayout->addStretch();
    iterationsControlLayout->addWidget(new QLabel("1"));

    iterationsSlider = new QSlider(Qt::Horizontal);
    iterationsSlider->setMinimum(1);
    iterationsSlider->setMaximum(20);
    iterationsSlider->setValue(m_iterations);
    iterationsControlLayout->addWidget(iterationsSlider, 1);
    iterationsControlLayout->addWidget(new QLabel("20"));

    iterationsLayout->addLayout(iterationsControlLayout);
    mainLayout->addWidget(iterationsGroup);

    // Boolean options
    auto optionsGroup = new QGroupBox("Options");
    auto optionsLayout = new QVBoxLayout(optionsGroup);

    diffCheckBox = new QCheckBox("Output difference only (hear distortion)");
    diffCheckBox->setChecked(m_diff);
    optionsLayout->addWidget(diffCheckBox);

    levelCheckBox = new QCheckBox("Auto level output to 0 dBFS");
    levelCheckBox->setChecked(m_level);
    optionsLayout->addWidget(levelCheckBox);

    mainLayout->addWidget(optionsGroup);

    // Connect level in slider and spinbox (dB)
    connect(levelInSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_levelIn = dbToLinear(db);
        
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(db);
        levelInSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_levelIn = dbToLinear(db);
        
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(db * 10));
        levelInSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect level out slider and spinbox (dB)
    connect(levelOutSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_levelOut = dbToLinear(db);
        
        levelOutSpinBox->blockSignals(true);
        levelOutSpinBox->setValue(db);
        levelOutSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_levelOut = dbToLinear(db);
        
        levelOutSlider->blockSignals(true);
        levelOutSlider->setValue(static_cast<int>(db * 10));
        levelOutSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect clip slider and spinbox (dB)
    connect(clipSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_clip = dbToLinear(db);
        
        clipSpinBox->blockSignals(true);
        clipSpinBox->setValue(db);
        clipSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(clipSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_clip = dbToLinear(db);
        
        clipSlider->blockSignals(true);
        clipSlider->setValue(static_cast<int>(db * 10));
        clipSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect adaptive slider and spinbox (percentage)
    connect(adaptiveSlider, &QSlider::valueChanged, [this](int value) {
        m_adaptive = value / 100.0;
        
        adaptiveSpinBox->blockSignals(true);
        adaptiveSpinBox->setValue(value);
        adaptiveSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(adaptiveSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_adaptive = value / 100.0;
        
        adaptiveSlider->blockSignals(true);
        adaptiveSlider->setValue(static_cast<int>(value));
        adaptiveSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect iterations slider and spinbox
    connect(iterationsSlider, &QSlider::valueChanged, [this](int value) {
        m_iterations = value;
        
        iterationsSpinBox->blockSignals(true);
        iterationsSpinBox->setValue(m_iterations);
        iterationsSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(iterationsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_iterations = static_cast<int>(value);
        
        iterationsSlider->blockSignals(true);
        iterationsSlider->setValue(m_iterations);
        iterationsSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect checkboxes
    connect(diffCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_diff = checked;
        updateFFmpegFlags();
    });

    connect(levelCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_level = checked;
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFApsyclip::updateFFmpegFlags() {
    ffmpegFlags = QString("apsyclip=level_in=%1:level_out=%2:clip=%3:diff=%4:adaptive=%5:iterations=%6:level=%7")
                      .arg(m_levelIn, 0, 'f', 6)
                      .arg(m_levelOut, 0, 'f', 6)
                      .arg(m_clip, 0, 'f', 6)
                      .arg(m_diff ? 1 : 0)
                      .arg(m_adaptive, 0, 'f', 2)
                      .arg(m_iterations)
                      .arg(m_level ? 1 : 0);
}

QString FFApsyclip::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFApsyclip::toJSON(QJsonObject& json) const {
    json["type"] = "ff-apsyclip";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["clip"] = m_clip;
    json["diff"] = m_diff;
    json["adaptive"] = m_adaptive;
    json["iterations"] = m_iterations;
    json["level"] = m_level;
}

void FFApsyclip::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_clip = json["clip"].toDouble(1.0);
    m_diff = json["diff"].toBool(false);
    m_adaptive = json["adaptive"].toDouble(0.5);
    m_iterations = json["iterations"].toInt(10);
    m_level = json["level"].toBool(false);
    updateFFmpegFlags();
}