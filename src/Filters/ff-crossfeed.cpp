#include "ff-crossfeed.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFCrossfeed::FFCrossfeed() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFCrossfeed::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Strength parameter (crossfeed amount)
    auto strengthGroup = new QGroupBox("Crossfeed Strength");
    auto strengthLayout = new QVBoxLayout(strengthGroup);

    strengthSpinBox = new QDoubleSpinBox();
    strengthSpinBox->setMinimum(0.0);
    strengthSpinBox->setMaximum(100.0);
    strengthSpinBox->setValue(m_strength * 100.0);
    strengthSpinBox->setDecimals(2);
    strengthSpinBox->setSingleStep(1.0);
    strengthSpinBox->setSuffix(" %");

    auto strengthControlLayout = new QHBoxLayout();
    strengthControlLayout->addWidget(strengthSpinBox);
    strengthControlLayout->addStretch();
    
    auto strengthMinLabel = new QLabel("0%");
    strengthControlLayout->addWidget(strengthMinLabel);

    strengthSlider = new QSlider(Qt::Horizontal);
    strengthSlider->setMinimum(0);
    strengthSlider->setMaximum(100);
    strengthSlider->setValue(static_cast<int>(m_strength * 100));
    strengthControlLayout->addWidget(strengthSlider, 1);

    auto strengthMaxLabel = new QLabel("100%");
    strengthControlLayout->addWidget(strengthMaxLabel);

    strengthLayout->addLayout(strengthControlLayout);
    mainLayout->addWidget(strengthGroup);

    // Range parameter (soundstage wideness)
    auto rangeGroup = new QGroupBox("Soundstage Width");
    auto rangeLayout = new QVBoxLayout(rangeGroup);

    rangeSpinBox = new QDoubleSpinBox();
    rangeSpinBox->setMinimum(0.0);
    rangeSpinBox->setMaximum(100.0);
    rangeSpinBox->setValue(m_range * 100.0);
    rangeSpinBox->setDecimals(2);
    rangeSpinBox->setSingleStep(1.0);
    rangeSpinBox->setSuffix(" %");

    auto rangeControlLayout = new QHBoxLayout();
    rangeControlLayout->addWidget(rangeSpinBox);
    rangeControlLayout->addStretch();
    
    auto rangeMinLabel = new QLabel("0%");
    rangeControlLayout->addWidget(rangeMinLabel);

    rangeSlider = new QSlider(Qt::Horizontal);
    rangeSlider->setMinimum(0);
    rangeSlider->setMaximum(100);
    rangeSlider->setValue(static_cast<int>(m_range * 100));
    rangeControlLayout->addWidget(rangeSlider, 1);

    auto rangeMaxLabel = new QLabel("100%");
    rangeControlLayout->addWidget(rangeMaxLabel);

    rangeLayout->addLayout(rangeControlLayout);
    mainLayout->addWidget(rangeGroup);

    // Slope parameter (curve slope)
    auto slopeGroup = new QGroupBox("Curve Slope");
    auto slopeLayout = new QVBoxLayout(slopeGroup);

    slopeSpinBox = new QDoubleSpinBox();
    slopeSpinBox->setMinimum(1.0);
    slopeSpinBox->setMaximum(100.0);
    slopeSpinBox->setValue(m_slope * 100.0);
    slopeSpinBox->setDecimals(2);
    slopeSpinBox->setSingleStep(1.0);
    slopeSpinBox->setSuffix(" %");

    auto slopeControlLayout = new QHBoxLayout();
    slopeControlLayout->addWidget(slopeSpinBox);
    slopeControlLayout->addStretch();
    
    auto slopeMinLabel = new QLabel("1%");
    slopeControlLayout->addWidget(slopeMinLabel);

    slopeSlider = new QSlider(Qt::Horizontal);
    slopeSlider->setMinimum(1);
    slopeSlider->setMaximum(100);
    slopeSlider->setValue(static_cast<int>(m_slope * 100));
    slopeControlLayout->addWidget(slopeSlider, 1);

    auto slopeMaxLabel = new QLabel("100%");
    slopeControlLayout->addWidget(slopeMaxLabel);

    slopeLayout->addLayout(slopeControlLayout);
    mainLayout->addWidget(slopeGroup);

    // Level In parameter
    auto levelInGroup = new QGroupBox("Input Level");
    auto levelInLayout = new QVBoxLayout(levelInGroup);

    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(0.0);
    levelInSpinBox->setMaximum(100.0);
    levelInSpinBox->setValue(m_levelIn * 100.0);
    levelInSpinBox->setDecimals(2);
    levelInSpinBox->setSingleStep(1.0);
    levelInSpinBox->setSuffix(" %");

    auto levelInControlLayout = new QHBoxLayout();
    levelInControlLayout->addWidget(levelInSpinBox);
    levelInControlLayout->addStretch();
    
    auto levelInMinLabel = new QLabel("0%");
    levelInControlLayout->addWidget(levelInMinLabel);

    levelInSlider = new QSlider(Qt::Horizontal);
    levelInSlider->setMinimum(0);
    levelInSlider->setMaximum(100);
    levelInSlider->setValue(static_cast<int>(m_levelIn * 100));
    levelInControlLayout->addWidget(levelInSlider, 1);

    auto levelInMaxLabel = new QLabel("100%");
    levelInControlLayout->addWidget(levelInMaxLabel);

    levelInLayout->addLayout(levelInControlLayout);
    mainLayout->addWidget(levelInGroup);

    // Level Out parameter
    auto levelOutGroup = new QGroupBox("Output Level");
    auto levelOutLayout = new QVBoxLayout(levelOutGroup);

    levelOutSpinBox = new QDoubleSpinBox();
    levelOutSpinBox->setMinimum(0.0);
    levelOutSpinBox->setMaximum(100.0);
    levelOutSpinBox->setValue(m_levelOut * 100.0);
    levelOutSpinBox->setDecimals(2);
    levelOutSpinBox->setSingleStep(1.0);
    levelOutSpinBox->setSuffix(" %");

    auto levelOutControlLayout = new QHBoxLayout();
    levelOutControlLayout->addWidget(levelOutSpinBox);
    levelOutControlLayout->addStretch();
    
    auto levelOutMinLabel = new QLabel("0%");
    levelOutControlLayout->addWidget(levelOutMinLabel);

    levelOutSlider = new QSlider(Qt::Horizontal);
    levelOutSlider->setMinimum(0);
    levelOutSlider->setMaximum(100);
    levelOutSlider->setValue(static_cast<int>(m_levelOut * 100));
    levelOutControlLayout->addWidget(levelOutSlider, 1);

    auto levelOutMaxLabel = new QLabel("100%");
    levelOutControlLayout->addWidget(levelOutMaxLabel);

    levelOutLayout->addLayout(levelOutControlLayout);
    mainLayout->addWidget(levelOutGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Strength
    connect(strengthSlider, &QSlider::valueChanged, [this](int value) {
        m_strength = value / 100.0;
        strengthSpinBox->blockSignals(true);
        strengthSpinBox->setValue(value);
        strengthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(strengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_strength = value / 100.0;
        strengthSlider->blockSignals(true);
        strengthSlider->setValue(static_cast<int>(value));
        strengthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Range
    connect(rangeSlider, &QSlider::valueChanged, [this](int value) {
        m_range = value / 100.0;
        rangeSpinBox->blockSignals(true);
        rangeSpinBox->setValue(value);
        rangeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(rangeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_range = value / 100.0;
        rangeSlider->blockSignals(true);
        rangeSlider->setValue(static_cast<int>(value));
        rangeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Slope
    connect(slopeSlider, &QSlider::valueChanged, [this](int value) {
        m_slope = value / 100.0;
        slopeSpinBox->blockSignals(true);
        slopeSpinBox->setValue(value);
        slopeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(slopeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_slope = value / 100.0;
        slopeSlider->blockSignals(true);
        slopeSlider->setValue(static_cast<int>(std::clamp(value, 1.0, 100.0)));
        slopeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Level In
    connect(levelInSlider, &QSlider::valueChanged, [this](int value) {
        m_levelIn = value / 100.0;
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(value);
        levelInSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelIn = value / 100.0;
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(value));
        levelInSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Level Out
    connect(levelOutSlider, &QSlider::valueChanged, [this](int value) {
        m_levelOut = value / 100.0;
        levelOutSpinBox->blockSignals(true);
        levelOutSpinBox->setValue(value);
        levelOutSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelOut = value / 100.0;
        levelOutSlider->blockSignals(true);
        levelOutSlider->setValue(static_cast<int>(value));
        levelOutSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFCrossfeed::updateFFmpegFlags() {
    ffmpegFlags = QString("crossfeed=strength=%1:range=%2:slope=%3:level_in=%4:level_out=%5")
        .arg(m_strength, 0, 'f', 2)
        .arg(m_range, 0, 'f', 2)
        .arg(m_slope, 0, 'f', 2)
        .arg(m_levelIn, 0, 'f', 2)
        .arg(m_levelOut, 0, 'f', 2);
}

QString FFCrossfeed::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFCrossfeed::toJSON(QJsonObject& json) const {
    json["type"] = "ff-crossfeed";
    json["strength"] = m_strength;
    json["range"] = m_range;
    json["slope"] = m_slope;
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
}

void FFCrossfeed::fromJSON(const QJsonObject& json) {
    m_strength = json["strength"].toDouble(0.2);
    m_range = json["range"].toDouble(0.5);
    m_slope = json["slope"].toDouble(0.5);
    m_levelIn = json["level_in"].toDouble(0.9);
    m_levelOut = json["level_out"].toDouble(1.0);
    updateFFmpegFlags();
}
