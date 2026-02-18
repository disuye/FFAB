#include "ff-flanger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFFlanger::FFFlanger() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFFlanger::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(10);

    // Delay (0 - 30 ms)
    auto delayGroup = new QGroupBox("Base Delay (ms)");
    auto delayLayout = new QVBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(0.0);
    delaySpinBox->setMaximum(30.0);
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
    delaySlider->setMaximum(3000);
    delaySlider->setValue(0);
    delayControlLayout->addWidget(delaySlider, 1);
    
    auto delayMaxLabel = new QLabel("30");
    delayControlLayout->addWidget(delayMaxLabel);

    delayLayout->addLayout(delayControlLayout);
    mainLayout->addWidget(delayGroup);

    // Depth (0 - 10 ms)
    auto depthGroup = new QGroupBox("Swept Delay Depth (ms)");
    auto depthLayout = new QVBoxLayout(depthGroup);

    depthSpinBox = new QDoubleSpinBox();
    depthSpinBox->setMinimum(0.0);
    depthSpinBox->setMaximum(10.0);
    depthSpinBox->setValue(m_depth);
    depthSpinBox->setDecimals(2);
    depthSpinBox->setSuffix(" ms");

    auto depthControlLayout = new QHBoxLayout();
    depthControlLayout->addWidget(depthSpinBox);
    depthControlLayout->addStretch();
    
    auto depthMinLabel = new QLabel("0");
    depthControlLayout->addWidget(depthMinLabel);

    depthSlider = new QSlider(Qt::Horizontal);
    depthSlider->setMinimum(0);
    depthSlider->setMaximum(1000);
    depthSlider->setValue(200);
    depthControlLayout->addWidget(depthSlider, 1);
    
    auto depthMaxLabel = new QLabel("10");
    depthControlLayout->addWidget(depthMaxLabel);

    depthLayout->addLayout(depthControlLayout);
    mainLayout->addWidget(depthGroup);

    // Regen (-95 - 95 %)
    auto regenGroup = new QGroupBox("Regeneration (%)");
    auto regenLayout = new QVBoxLayout(regenGroup);

    regenSpinBox = new QDoubleSpinBox();
    regenSpinBox->setMinimum(-95.0);
    regenSpinBox->setMaximum(95.0);
    regenSpinBox->setValue(m_regen);
    regenSpinBox->setDecimals(2);
    regenSpinBox->setSuffix(" %");

    auto regenControlLayout = new QHBoxLayout();
    regenControlLayout->addWidget(regenSpinBox);
    regenControlLayout->addStretch();
    
    auto regenMinLabel = new QLabel("-95");
    regenControlLayout->addWidget(regenMinLabel);

    regenSlider = new QSlider(Qt::Horizontal);
    regenSlider->setMinimum(-9500);
    regenSlider->setMaximum(9500);
    regenSlider->setValue(0);
    regenControlLayout->addWidget(regenSlider, 1);
    
    auto regenMaxLabel = new QLabel("+95");
    regenControlLayout->addWidget(regenMaxLabel);

    regenLayout->addLayout(regenControlLayout);
    mainLayout->addWidget(regenGroup);

    // Width (0 - 100 %)
    auto widthGroup = new QGroupBox("Width (%)");
    auto widthLayout = new QVBoxLayout(widthGroup);

    widthSpinBox = new QDoubleSpinBox();
    widthSpinBox->setMinimum(0.0);
    widthSpinBox->setMaximum(100.0);
    widthSpinBox->setValue(m_width);
    widthSpinBox->setDecimals(2);
    widthSpinBox->setSuffix(" %");

    auto widthControlLayout = new QHBoxLayout();
    widthControlLayout->addWidget(widthSpinBox);
    widthControlLayout->addStretch();
    
    auto widthMinLabel = new QLabel("0");
    widthControlLayout->addWidget(widthMinLabel);

    widthSlider = new QSlider(Qt::Horizontal);
    widthSlider->setMinimum(0);
    widthSlider->setMaximum(10000);
    widthSlider->setValue(7100);
    widthControlLayout->addWidget(widthSlider, 1);
    
    auto widthMaxLabel = new QLabel("100");
    widthControlLayout->addWidget(widthMaxLabel);

    widthLayout->addLayout(widthControlLayout);
    mainLayout->addWidget(widthGroup);

    // Speed (0.1 - 10 Hz)
    auto speedGroup = new QGroupBox("Speed (Hz)");
    auto speedLayout = new QVBoxLayout(speedGroup);

    speedSpinBox = new QDoubleSpinBox();
    speedSpinBox->setMinimum(0.1);
    speedSpinBox->setMaximum(10.0);
    speedSpinBox->setValue(m_speed);
    speedSpinBox->setDecimals(2);
    speedSpinBox->setSuffix(" Hz");

    auto speedControlLayout = new QHBoxLayout();
    speedControlLayout->addWidget(speedSpinBox);
    speedControlLayout->addStretch();
    
    auto speedMinLabel = new QLabel("0.1");
    speedControlLayout->addWidget(speedMinLabel);

    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setMinimum(10);
    speedSlider->setMaximum(1000);
    speedSlider->setValue(50);
    speedControlLayout->addWidget(speedSlider, 1);
    
    auto speedMaxLabel = new QLabel("10");
    speedControlLayout->addWidget(speedMaxLabel);

    speedLayout->addLayout(speedControlLayout);
    mainLayout->addWidget(speedGroup);

    // Phase (0 - 100 %)
    auto phaseGroup = new QGroupBox("Phase Shift (%)");
    auto phaseLayout = new QVBoxLayout(phaseGroup);

    phaseSpinBox = new QDoubleSpinBox();
    phaseSpinBox->setMinimum(0.0);
    phaseSpinBox->setMaximum(100.0);
    phaseSpinBox->setValue(m_phase);
    phaseSpinBox->setDecimals(2);
    phaseSpinBox->setSuffix(" %");

    auto phaseControlLayout = new QHBoxLayout();
    phaseControlLayout->addWidget(phaseSpinBox);
    phaseControlLayout->addStretch();
    
    auto phaseMinLabel = new QLabel("0");
    phaseControlLayout->addWidget(phaseMinLabel);

    phaseSlider = new QSlider(Qt::Horizontal);
    phaseSlider->setMinimum(0);
    phaseSlider->setMaximum(10000);
    phaseSlider->setValue(2500);
    phaseControlLayout->addWidget(phaseSlider, 1);
    
    auto phaseMaxLabel = new QLabel("100");
    phaseControlLayout->addWidget(phaseMaxLabel);

    phaseLayout->addLayout(phaseControlLayout);
    mainLayout->addWidget(phaseGroup);

    // Shape and Interp combos
    auto optionsGroup = new QGroupBox("Options");
    auto optionsLayout = new QHBoxLayout(optionsGroup);
    
    optionsLayout->addWidget(new QLabel("Wave Shape:"));
    shapeCombo = new QComboBox();
    shapeCombo->addItem("Triangular", 0);
    shapeCombo->addItem("Sinusoidal", 1);
    shapeCombo->setCurrentIndex(m_shape);
    optionsLayout->addWidget(shapeCombo);
    
    optionsLayout->addWidget(new QLabel("Interpolation:"));
    interpCombo = new QComboBox();
    interpCombo->addItem("Linear", 0);
    interpCombo->addItem("Quadratic", 1);
    interpCombo->setCurrentIndex(m_interp);
    optionsLayout->addWidget(interpCombo);
    
    mainLayout->addWidget(optionsGroup);

    // Connect all controls
    connect(delaySlider, &QSlider::valueChanged, [this](int v) {
        m_delay = v / 100.0;
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_delay = v;
        delaySlider->blockSignals(true);
        delaySlider->setValue(v * 100);
        delaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(depthSlider, &QSlider::valueChanged, [this](int v) {
        m_depth = v / 100.0;
        depthSpinBox->blockSignals(true);
        depthSpinBox->setValue(m_depth);
        depthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(depthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_depth = v;
        depthSlider->blockSignals(true);
        depthSlider->setValue(v * 100);
        depthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(regenSlider, &QSlider::valueChanged, [this](int v) {
        m_regen = v / 100.0;
        regenSpinBox->blockSignals(true);
        regenSpinBox->setValue(m_regen);
        regenSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(regenSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_regen = v;
        regenSlider->blockSignals(true);
        regenSlider->setValue(v * 100);
        regenSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(widthSlider, &QSlider::valueChanged, [this](int v) {
        m_width = v / 100.0;
        widthSpinBox->blockSignals(true);
        widthSpinBox->setValue(m_width);
        widthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_width = v;
        widthSlider->blockSignals(true);
        widthSlider->setValue(v * 100);
        widthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(speedSlider, &QSlider::valueChanged, [this](int v) {
        m_speed = v / 100.0;
        speedSpinBox->blockSignals(true);
        speedSpinBox->setValue(m_speed);
        speedSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(speedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_speed = v;
        speedSlider->blockSignals(true);
        speedSlider->setValue(v * 100);
        speedSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(phaseSlider, &QSlider::valueChanged, [this](int v) {
        m_phase = v / 100.0;
        phaseSpinBox->blockSignals(true);
        phaseSpinBox->setValue(m_phase);
        phaseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(phaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_phase = v;
        phaseSlider->blockSignals(true);
        phaseSlider->setValue(v * 100);
        phaseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(shapeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_shape = shapeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(interpCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_interp = interpCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    mainLayout->addStretch();
    return parametersWidget;
}

void FFFlanger::updateFFmpegFlags() {
    QStringList shapeNames = {"triangular", "sinusoidal"};
    QStringList interpNames = {"linear", "quadratic"};
    
    QString shapeName = (m_shape < shapeNames.size()) ? shapeNames[m_shape] : "sinusoidal";
    QString interpName = (m_interp < interpNames.size()) ? interpNames[m_interp] : "linear";
    
    ffmpegFlags = QString("flanger=delay=%1:depth=%2:regen=%3:width=%4:speed=%5:shape=%6:phase=%7:interp=%8")
                      .arg(m_delay, 0, 'f', 1)
                      .arg(m_depth, 0, 'f', 1)
                      .arg(m_regen, 0, 'f', 1)
                      .arg(m_width, 0, 'f', 1)
                      .arg(m_speed, 0, 'f', 1)
                      .arg(shapeName)
                      .arg(m_phase, 0, 'f', 1)
                      .arg(interpName);
}

QString FFFlanger::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFFlanger::toJSON(QJsonObject& json) const {
    json["type"] = "ff-flanger";
    json["delay"] = m_delay;
    json["depth"] = m_depth;
    json["regen"] = m_regen;
    json["width"] = m_width;
    json["speed"] = m_speed;
    json["shape"] = m_shape;
    json["phase"] = m_phase;
    json["interp"] = m_interp;
}

void FFFlanger::fromJSON(const QJsonObject& json) {
    m_delay = json["delay"].toDouble(0.0);
    m_depth = json["depth"].toDouble(2.0);
    m_regen = json["regen"].toDouble(0.0);
    m_width = json["width"].toDouble(71.0);
    m_speed = json["speed"].toDouble(0.5);
    m_shape = json["shape"].toInt(1);
    m_phase = json["phase"].toDouble(25.0);
    m_interp = json["interp"].toInt(0);
    updateFFmpegFlags();
}
