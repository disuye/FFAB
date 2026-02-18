#include "ff-aphaser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAphaser::FFAphaser() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAphaser::getParametersWidget() {
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

    // Input Gain (-96 to 0 dB)
    auto inGainGroup = new QGroupBox("Input Gain (dB)");
    auto inGainLayout = new QVBoxLayout(inGainGroup);

    inGainSpinBox = new QDoubleSpinBox();
    inGainSpinBox->setMinimum(-96.0);
    inGainSpinBox->setMaximum(0.0);
    inGainSpinBox->setValue(linearToDb(m_inGain));
    inGainSpinBox->setDecimals(1);
    inGainSpinBox->setSingleStep(0.1);
    inGainSpinBox->setSuffix(" dB");

    auto inGainControlLayout = new QHBoxLayout();
    inGainControlLayout->addWidget(inGainSpinBox);
    inGainControlLayout->addStretch();

    auto inGainMinLabel = new QLabel("-96");
    inGainControlLayout->addWidget(inGainMinLabel);

    inGainSlider = new QSlider(Qt::Horizontal);
    inGainSlider->setMinimum(0);
    inGainSlider->setMaximum(960);
    inGainSlider->setValue(static_cast<int>((linearToDb(m_inGain) + 96.0) * 10.0));
    inGainControlLayout->addWidget(inGainSlider, 1);

    auto inGainMaxLabel = new QLabel("0");
    inGainControlLayout->addWidget(inGainMaxLabel);

    inGainLayout->addLayout(inGainControlLayout);
    mainLayout->addWidget(inGainGroup);

    // Output Gain (-96 to +20 dB)
    auto outGainGroup = new QGroupBox("Output Gain (dB)");
    auto outGainLayout = new QVBoxLayout(outGainGroup);

    outGainSpinBox = new QDoubleSpinBox();
    outGainSpinBox->setMinimum(-96.0);
    outGainSpinBox->setMaximum(20.0);
    outGainSpinBox->setValue(linearToDb(m_outGain));
    outGainSpinBox->setDecimals(1);
    outGainSpinBox->setSingleStep(0.1);
    outGainSpinBox->setSuffix(" dB");

    auto outGainControlLayout = new QHBoxLayout();
    outGainControlLayout->addWidget(outGainSpinBox);
    outGainControlLayout->addStretch();

    auto outGainMinLabel = new QLabel("-96");
    outGainControlLayout->addWidget(outGainMinLabel);

    outGainSlider = new QSlider(Qt::Horizontal);
    outGainSlider->setMinimum(0);
    outGainSlider->setMaximum(1160);
    outGainSlider->setValue(static_cast<int>((linearToDb(m_outGain) + 96.0) * 10.0));
    outGainControlLayout->addWidget(outGainSlider, 1);

    auto outGainMaxLabel = new QLabel("+20");
    outGainControlLayout->addWidget(outGainMaxLabel);

    outGainLayout->addLayout(outGainControlLayout);
    mainLayout->addWidget(outGainGroup);

    // Delay (0 - 5 ms)
    auto delayGroup = new QGroupBox("Delay (ms)");
    auto delayLayout = new QVBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(0.0);
    delaySpinBox->setMaximum(5.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(2);
    delaySpinBox->setSuffix(" ms");

    auto delayControlLayout = new QHBoxLayout();
    delayControlLayout->addWidget(delaySpinBox);
    delayControlLayout->addStretch();
    
    auto delayMinLabel = new QLabel("0");
    delayControlLayout->addWidget(delayMinLabel);

    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(0);
    delaySlider->setMaximum(500);  // 5 ms * 100
    delaySlider->setValue(300);    // default 3
    delayControlLayout->addWidget(delaySlider, 1);
    
    auto delayMaxLabel = new QLabel("5");
    delayControlLayout->addWidget(delayMaxLabel);

    delayLayout->addLayout(delayControlLayout);
    mainLayout->addWidget(delayGroup);

    // Decay (0 - 0.99)
    auto decayGroup = new QGroupBox("Decay");
    auto decayLayout = new QVBoxLayout(decayGroup);

    decaySpinBox = new QDoubleSpinBox();
    decaySpinBox->setMinimum(0.0);
    decaySpinBox->setMaximum(0.99);
    decaySpinBox->setValue(m_decay);
    decaySpinBox->setDecimals(2);

    auto decayControlLayout = new QHBoxLayout();
    decayControlLayout->addWidget(decaySpinBox);
    decayControlLayout->addStretch();
    
    auto decayMinLabel = new QLabel("0.0");
    decayControlLayout->addWidget(decayMinLabel);

    decaySlider = new QSlider(Qt::Horizontal);
    decaySlider->setMinimum(0);
    decaySlider->setMaximum(99);
    decaySlider->setValue(40); // default 0.4
    decayControlLayout->addWidget(decaySlider, 1);
    
    auto decayMaxLabel = new QLabel("0.99");
    decayControlLayout->addWidget(decayMaxLabel);

    decayLayout->addLayout(decayControlLayout);
    mainLayout->addWidget(decayGroup);

    // Speed (0.1 - 2 Hz)
    auto speedGroup = new QGroupBox("Modulation Speed (Hz)");
    auto speedLayout = new QVBoxLayout(speedGroup);

    speedSpinBox = new QDoubleSpinBox();
    speedSpinBox->setMinimum(0.1);
    speedSpinBox->setMaximum(2.0);
    speedSpinBox->setValue(m_speed);
    speedSpinBox->setDecimals(2);
    speedSpinBox->setSuffix(" Hz");

    auto speedControlLayout = new QHBoxLayout();
    speedControlLayout->addWidget(speedSpinBox);
    speedControlLayout->addStretch();
    
    auto speedMinLabel = new QLabel("0.1");
    speedControlLayout->addWidget(speedMinLabel);

    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setMinimum(10);  // 0.1 * 100
    speedSlider->setMaximum(200); // 2 * 100
    speedSlider->setValue(50);    // default 0.5
    speedControlLayout->addWidget(speedSlider, 1);
    
    auto speedMaxLabel = new QLabel("2.0");
    speedControlLayout->addWidget(speedMaxLabel);

    speedLayout->addLayout(speedControlLayout);
    mainLayout->addWidget(speedGroup);

    // Type selection
    auto typeGroup = new QGroupBox("Modulation Type");
    auto typeLayout = new QVBoxLayout(typeGroup);

    typeCombo = new QComboBox();
    typeCombo->addItem("Triangular", 0);
    typeCombo->addItem("Sinusoidal", 1);
    typeCombo->setCurrentIndex(m_type);
    
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_type = typeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    typeLayout->addWidget(typeCombo);
    mainLayout->addWidget(typeGroup);

    // Connect all slider-spinbox pairs
    // Input Gain (dB conversion)
    connect(inGainSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_inGain = dbToLinear(db);

        inGainSpinBox->blockSignals(true);
        inGainSpinBox->setValue(db);
        inGainSpinBox->blockSignals(false);

        updateFFmpegFlags();
    });

    connect(inGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_inGain = dbToLinear(db);

        inGainSlider->blockSignals(true);
        inGainSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        inGainSlider->blockSignals(false);

        updateFFmpegFlags();
    });

    connect(outGainSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_outGain = dbToLinear(db);

        outGainSpinBox->blockSignals(true);
        outGainSpinBox->setValue(db);
        outGainSpinBox->blockSignals(false);

        updateFFmpegFlags();
    });

    connect(outGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_outGain = dbToLinear(db);

        outGainSlider->blockSignals(true);
        outGainSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        outGainSlider->blockSignals(false);

        updateFFmpegFlags();
    });

    connect(delaySlider, &QSlider::valueChanged, [this](int value) {
        m_delay = value / 100.0;
        
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_delay = value;
        
        delaySlider->blockSignals(true);
        delaySlider->setValue(static_cast<int>(value * 100));
        delaySlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(decaySlider, &QSlider::valueChanged, [this](int value) {
        m_decay = value / 100.0;
        
        decaySpinBox->blockSignals(true);
        decaySpinBox->setValue(m_decay);
        decaySpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(decaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_decay = value;
        
        decaySlider->blockSignals(true);
        decaySlider->setValue(static_cast<int>(value * 100));
        decaySlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(speedSlider, &QSlider::valueChanged, [this](int value) {
        m_speed = value / 100.0;
        
        speedSpinBox->blockSignals(true);
        speedSpinBox->setValue(m_speed);
        speedSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(speedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_speed = value;
        
        speedSlider->blockSignals(true);
        speedSlider->setValue(static_cast<int>(value * 100));
        speedSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAphaser::updateFFmpegFlags() {
    QStringList typeNames = {"triangular", "sinusoidal"};
    QString typeName = (m_type >= 0 && m_type < typeNames.size()) ? typeNames[m_type] : "sinusoidal";
    
    ffmpegFlags = QString("aphaser=in_gain=%1:out_gain=%2:delay=%3:decay=%4:speed=%5:type=%6")
                      .arg(m_inGain, 0, 'f', 2)
                      .arg(m_outGain, 0, 'f', 2)
                      .arg(m_delay, 0, 'f', 1)
                      .arg(m_decay, 0, 'f', 2)
                      .arg(m_speed, 0, 'f', 1)
                      .arg(typeName);
}

QString FFAphaser::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAphaser::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aphaser";
    json["in_gain"] = m_inGain;
    json["out_gain"] = m_outGain;
    json["delay"] = m_delay;
    json["decay"] = m_decay;
    json["speed"] = m_speed;
    json["modulation_type"] = m_type;
}

void FFAphaser::fromJSON(const QJsonObject& json) {
    m_inGain = json["in_gain"].toDouble(0.4);
    m_outGain = json["out_gain"].toDouble(0.74);
    m_delay = json["delay"].toDouble(3.0);
    m_decay = json["decay"].toDouble(0.4);
    m_speed = json["speed"].toDouble(0.5);
    m_type = json["modulation_type"].toInt(1);
    updateFFmpegFlags();
}
