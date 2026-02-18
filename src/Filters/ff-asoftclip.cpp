#include "ff-asoftclip.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QComboBox>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAsoftclip::FFAsoftclip() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsoftclip::getParametersWidget() {
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

    // Type selection
    auto typeGroup = new QGroupBox("Soft Clip Type");
    auto typeLayout = new QVBoxLayout(typeGroup);

    typeCombo = new QComboBox();
    typeCombo->addItem("Tanh", 0);
    typeCombo->addItem("Atan", 1);
    typeCombo->addItem("Cubic", 2);
    typeCombo->addItem("Exp", 3);
    typeCombo->addItem("Alg", 4);
    typeCombo->addItem("Quintic", 5);
    typeCombo->addItem("Sin", 6);
    typeCombo->addItem("Erf", 7);
    typeCombo->setCurrentIndex(m_type);
    
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_type = typeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    typeLayout->addWidget(typeCombo);
    mainLayout->addWidget(typeGroup);

    // Threshold parameter (-120 to 0 dB)
    auto thresholdGroup = new QGroupBox("Threshold");
    auto thresholdLayout = new QVBoxLayout(thresholdGroup);

    thresholdSpinBox = new QDoubleSpinBox();
    thresholdSpinBox->setMinimum(-120.0);
    thresholdSpinBox->setMaximum(0.0);
    thresholdSpinBox->setValue(linearToDb(m_threshold));
    thresholdSpinBox->setDecimals(1);
    thresholdSpinBox->setSingleStep(1.0);
    thresholdSpinBox->setSuffix(" dB");

    auto thresholdControlLayout = new QHBoxLayout();
    thresholdControlLayout->addWidget(thresholdSpinBox);
    thresholdControlLayout->addStretch();
    thresholdControlLayout->addWidget(new QLabel("-120"));

    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setMinimum(-1200);
    thresholdSlider->setMaximum(0);
    thresholdSlider->setValue(static_cast<int>(linearToDb(m_threshold) * 10));
    thresholdControlLayout->addWidget(thresholdSlider, 1);
    thresholdControlLayout->addWidget(new QLabel("0 dB"));

    auto thresholdHint = new QLabel("Level where soft clipping begins (0 dB = full scale)");
    thresholdHint->setStyleSheet("color: gray; font-size: 11px;");
    thresholdLayout->addLayout(thresholdControlLayout);
    thresholdLayout->addWidget(thresholdHint);
    mainLayout->addWidget(thresholdGroup);

    // Output gain parameter (-120 to +24 dB)
    auto outputGroup = new QGroupBox("Output Gain");
    auto outputLayout = new QVBoxLayout(outputGroup);

    outputSpinBox = new QDoubleSpinBox();
    outputSpinBox->setMinimum(-120.0);
    outputSpinBox->setMaximum(24.0);
    outputSpinBox->setValue(linearToDb(m_output));
    outputSpinBox->setDecimals(1);
    outputSpinBox->setSingleStep(0.5);
    outputSpinBox->setSuffix(" dB");

    auto outputControlLayout = new QHBoxLayout();
    outputControlLayout->addWidget(outputSpinBox);
    outputControlLayout->addStretch();
    outputControlLayout->addWidget(new QLabel("-120"));

    outputSlider = new QSlider(Qt::Horizontal);
    outputSlider->setMinimum(-1200);
    outputSlider->setMaximum(240);
    outputSlider->setValue(static_cast<int>(linearToDb(m_output) * 10));
    outputControlLayout->addWidget(outputSlider, 1);
    outputControlLayout->addWidget(new QLabel("+24 dB"));

    outputLayout->addLayout(outputControlLayout);
    mainLayout->addWidget(outputGroup);

    // Param parameter (0.01 - 3)
    auto paramGroup = new QGroupBox("Curve Parameter");
    auto paramLayout = new QVBoxLayout(paramGroup);

    paramSpinBox = new QDoubleSpinBox();
    paramSpinBox->setMinimum(0.01);
    paramSpinBox->setMaximum(3.0);
    paramSpinBox->setValue(m_param);
    paramSpinBox->setDecimals(2);
    paramSpinBox->setSingleStep(0.05);

    auto paramControlLayout = new QHBoxLayout();
    paramControlLayout->addWidget(paramSpinBox);
    paramControlLayout->addStretch();
    paramControlLayout->addWidget(new QLabel("0.01"));

    paramSlider = new QSlider(Qt::Horizontal);
    paramSlider->setMinimum(1);
    paramSlider->setMaximum(300);
    paramSlider->setValue(static_cast<int>(m_param * 100));
    paramControlLayout->addWidget(paramSlider, 1);
    paramControlLayout->addWidget(new QLabel("3.0"));

    auto paramHint = new QLabel("Controls sigmoid curve shape");
    paramHint->setStyleSheet("color: gray; font-size: 11px;");
    paramLayout->addLayout(paramControlLayout);
    paramLayout->addWidget(paramHint);
    mainLayout->addWidget(paramGroup);

    // Oversample parameter (1 - 64)
    auto oversampleGroup = new QGroupBox("Oversample Factor");
    auto oversampleLayout = new QVBoxLayout(oversampleGroup);

    oversampleSpinBox = new QDoubleSpinBox();
    oversampleSpinBox->setMinimum(1);
    oversampleSpinBox->setMaximum(64);
    oversampleSpinBox->setValue(m_oversample);
    oversampleSpinBox->setDecimals(0);
    oversampleSpinBox->setSuffix("x");

    auto oversampleControlLayout = new QHBoxLayout();
    oversampleControlLayout->addWidget(oversampleSpinBox);
    oversampleControlLayout->addStretch();
    oversampleControlLayout->addWidget(new QLabel("1x"));

    oversampleSlider = new QSlider(Qt::Horizontal);
    oversampleSlider->setMinimum(1);
    oversampleSlider->setMaximum(64);
    oversampleSlider->setValue(m_oversample);
    oversampleControlLayout->addWidget(oversampleSlider, 1);
    oversampleControlLayout->addWidget(new QLabel("64x"));

    auto oversampleHint = new QLabel("Higher = better quality, more CPU");
    oversampleHint->setStyleSheet("color: gray; font-size: 11px;");
    oversampleLayout->addLayout(oversampleControlLayout);
    oversampleLayout->addWidget(oversampleHint);
    mainLayout->addWidget(oversampleGroup);

    // Connect threshold slider and spinbox (dB)
    connect(thresholdSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_threshold = dbToLinear(db);
        
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(db);
        thresholdSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_threshold = dbToLinear(db);
        
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>(db * 10));
        thresholdSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect output slider and spinbox (dB)
    connect(outputSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_output = dbToLinear(db);
        
        outputSpinBox->blockSignals(true);
        outputSpinBox->setValue(db);
        outputSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(outputSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_output = dbToLinear(db);
        
        outputSlider->blockSignals(true);
        outputSlider->setValue(static_cast<int>(db * 10));
        outputSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect param slider and spinbox
    connect(paramSlider, &QSlider::valueChanged, [this](int value) {
        m_param = value / 100.0;
        
        paramSpinBox->blockSignals(true);
        paramSpinBox->setValue(m_param);
        paramSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(paramSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_param = value;
        
        paramSlider->blockSignals(true);
        paramSlider->setValue(static_cast<int>(value * 100));
        paramSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect oversample slider and spinbox
    connect(oversampleSlider, &QSlider::valueChanged, [this](int value) {
        m_oversample = value;
        
        oversampleSpinBox->blockSignals(true);
        oversampleSpinBox->setValue(m_oversample);
        oversampleSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(oversampleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_oversample = static_cast<int>(value);
        
        oversampleSlider->blockSignals(true);
        oversampleSlider->setValue(m_oversample);
        oversampleSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAsoftclip::updateFFmpegFlags() {
    QStringList typeNames = {"tanh", "atan", "cubic", "exp", "alg", "quintic", "sin", "erf"};
    QString typeName = (m_type >= 0 && m_type < typeNames.size()) ? typeNames[m_type] : "tanh";
    
    ffmpegFlags = QString("asoftclip=type=%1:threshold=%2:output=%3:param=%4:oversample=%5")
                      .arg(typeName)
                      .arg(m_threshold, 0, 'f', 6)
                      .arg(m_output, 0, 'f', 6)
                      .arg(m_param, 0, 'f', 2)
                      .arg(m_oversample);
}

QString FFAsoftclip::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsoftclip::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asoftclip";
    json["m_type"] = m_type;
    json["threshold"] = m_threshold;
    json["output"] = m_output;
    json["param"] = m_param;
    json["oversample"] = m_oversample;
}

void FFAsoftclip::fromJSON(const QJsonObject& json) {
    m_type = json["m_type"].toInt(0);
    m_threshold = json["threshold"].toDouble(1.0);
    m_output = json["output"].toDouble(1.0);
    m_param = json["param"].toDouble(1.0);
    m_oversample = json["oversample"].toInt(1);
    updateFFmpegFlags();
}