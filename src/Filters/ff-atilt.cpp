#include "ff-atilt.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFAtilt::FFAtilt() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAtilt::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: TILT ==========
    auto tiltTab = new QWidget();
    auto tiltLayout = new QVBoxLayout(tiltTab);

    // Slope parameter (-1 to 1)
    auto slopeGroup = new QGroupBox("Tilt Slope");
    auto slopeLayout = new QVBoxLayout(slopeGroup);

    slopeSpinBox = new QDoubleSpinBox();
    slopeSpinBox->setMinimum(-1.0);
    slopeSpinBox->setMaximum(1.0);
    slopeSpinBox->setValue(m_slope);
    slopeSpinBox->setDecimals(2);
    slopeSpinBox->setSingleStep(0.05);

    auto slopeControlLayout = new QHBoxLayout();
    slopeControlLayout->addWidget(slopeSpinBox);
    slopeControlLayout->addStretch();
    slopeControlLayout->addWidget(new QLabel("-1"));
    slopeSlider = new QSlider(Qt::Horizontal);
    slopeSlider->setMinimum(-100);
    slopeSlider->setMaximum(100);
    slopeSlider->setValue(static_cast<int>(m_slope * 100));
    slopeControlLayout->addWidget(slopeSlider, 1);
    slopeControlLayout->addWidget(new QLabel("+1"));

    // Add scale hints
    auto scaleLayout = new QHBoxLayout();
    auto bassLabel = new QLabel("← Bass boost");
    bassLabel->setStyleSheet("color: gray; font-size: 10px;");
    auto trebleLabel = new QLabel("Treble boost →");
    trebleLabel->setStyleSheet("color: gray; font-size: 10px;");
    trebleLabel->setAlignment(Qt::AlignRight);
    scaleLayout->addWidget(bassLabel);
    scaleLayout->addStretch();
    scaleLayout->addWidget(trebleLabel);

    slopeLayout->addLayout(slopeControlLayout);
    slopeLayout->addLayout(scaleLayout);
    tiltLayout->addWidget(slopeGroup);

    // Frequency parameter (20 - 20000 Hz)
    auto freqGroup = new QGroupBox("Center Frequency (Hz)");
    auto freqLayout = new QVBoxLayout(freqGroup);

    freqSpinBox = new QDoubleSpinBox();
    freqSpinBox->setMinimum(20.0);
    freqSpinBox->setMaximum(20000.0);
    freqSpinBox->setValue(m_freq);
    freqSpinBox->setDecimals(0);
    freqSpinBox->setSingleStep(100.0);
    freqSpinBox->setSuffix(" Hz");

    auto freqControlLayout = new QHBoxLayout();
    freqControlLayout->addWidget(freqSpinBox);
    freqControlLayout->addStretch();
    freqControlLayout->addWidget(new QLabel("20"));
    freqSlider = new QSlider(Qt::Horizontal);
    freqSlider->setMinimum(0);
    freqSlider->setMaximum(1000);
    freqSlider->setValue(freqToSlider(m_freq, 20.0, 20000.0));
    freqControlLayout->addWidget(freqSlider, 1);
    freqControlLayout->addWidget(new QLabel("20k"));

    auto freqHint = new QLabel("Pivot frequency for the tilt");
    freqHint->setStyleSheet("color: gray; font-size: 11px;");
    freqLayout->addLayout(freqControlLayout);
    freqLayout->addWidget(freqHint);
    tiltLayout->addWidget(freqGroup);

    tiltLayout->addStretch();
    tabWidget->addTab(tiltTab, "Tilt");

    // ========== TAB 2: SHAPE ==========
    auto shapeTab = new QWidget();
    auto shapeLayout = new QVBoxLayout(shapeTab);

    // Width parameter (100 - 10000)
    auto widthGroup = new QGroupBox("Filter Width");
    auto widthLayout = new QVBoxLayout(widthGroup);

    widthSpinBox = new QDoubleSpinBox();
    widthSpinBox->setMinimum(100.0);
    widthSpinBox->setMaximum(10000.0);
    widthSpinBox->setValue(m_width);
    widthSpinBox->setDecimals(0);
    widthSpinBox->setSingleStep(100.0);

    auto widthControlLayout = new QHBoxLayout();
    widthControlLayout->addWidget(widthSpinBox);
    widthControlLayout->addStretch();
    widthControlLayout->addWidget(new QLabel("100"));
    widthSlider = new QSlider(Qt::Horizontal);
    widthSlider->setMinimum(100);
    widthSlider->setMaximum(10000);
    widthSlider->setValue(static_cast<int>(m_width));
    widthControlLayout->addWidget(widthSlider, 1);
    widthControlLayout->addWidget(new QLabel("10000"));

    auto widthHint = new QLabel("Transition width around center frequency");
    widthHint->setStyleSheet("color: gray; font-size: 11px;");
    widthLayout->addLayout(widthControlLayout);
    widthLayout->addWidget(widthHint);
    shapeLayout->addWidget(widthGroup);

    // Order parameter (2 - 30)
    auto orderGroup = new QGroupBox("Filter Order");
    auto orderLayout = new QVBoxLayout(orderGroup);

    orderSpinBox = new QSpinBox();
    orderSpinBox->setMinimum(2);
    orderSpinBox->setMaximum(30);
    orderSpinBox->setValue(m_order);

    auto orderControlLayout = new QHBoxLayout();
    orderControlLayout->addWidget(orderSpinBox);
    orderControlLayout->addStretch();
    orderControlLayout->addWidget(new QLabel("2"));
    orderSlider = new QSlider(Qt::Horizontal);
    orderSlider->setMinimum(2);
    orderSlider->setMaximum(30);
    orderSlider->setValue(m_order);
    orderControlLayout->addWidget(orderSlider, 1);
    orderControlLayout->addWidget(new QLabel("30"));

    auto orderHint = new QLabel("Higher = steeper transition slope");
    orderHint->setStyleSheet("color: gray; font-size: 11px;");
    orderLayout->addLayout(orderControlLayout);
    orderLayout->addWidget(orderHint);
    shapeLayout->addWidget(orderGroup);

    // Level parameter (0 - 4)
    auto levelGroup = new QGroupBox("Input Level");
    auto levelLayout = new QVBoxLayout(levelGroup);

    levelSpinBox = new QDoubleSpinBox();
    levelSpinBox->setMinimum(0.0);
    levelSpinBox->setMaximum(4.0);
    levelSpinBox->setValue(m_level);
    levelSpinBox->setDecimals(2);
    levelSpinBox->setSingleStep(0.1);

    auto levelControlLayout = new QHBoxLayout();
    levelControlLayout->addWidget(levelSpinBox);
    levelControlLayout->addStretch();
    levelControlLayout->addWidget(new QLabel("0"));
    levelSlider = new QSlider(Qt::Horizontal);
    levelSlider->setMinimum(0);
    levelSlider->setMaximum(400);
    levelSlider->setValue(static_cast<int>(m_level * 100));
    levelControlLayout->addWidget(levelSlider, 1);
    levelControlLayout->addWidget(new QLabel("4"));

    levelLayout->addLayout(levelControlLayout);
    shapeLayout->addWidget(levelGroup);

    shapeLayout->addStretch();
    tabWidget->addTab(shapeTab, "Shape");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Slope
    connect(slopeSlider, &QSlider::valueChanged, [this](int value) {
        m_slope = value / 100.0;
        slopeSpinBox->blockSignals(true);
        slopeSpinBox->setValue(m_slope);
        slopeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(slopeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_slope = value;
        slopeSlider->blockSignals(true);
        slopeSlider->setValue(static_cast<int>(value * 100));
        slopeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Frequency
    connect(freqSlider, &QSlider::valueChanged, [this](int value) {
        m_freq = sliderToFreq(value, 20.0, 20000.0);
        freqSpinBox->blockSignals(true);
        freqSpinBox->setValue(m_freq);
        freqSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(freqSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_freq = value;
        freqSlider->blockSignals(true);
        freqSlider->setValue(freqToSlider(value, 20.0, 20000.0));
        freqSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Width
    connect(widthSlider, &QSlider::valueChanged, [this](int value) {
        m_width = value;
        widthSpinBox->blockSignals(true);
        widthSpinBox->setValue(m_width);
        widthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_width = value;
        widthSlider->blockSignals(true);
        widthSlider->setValue(static_cast<int>(value));
        widthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Order
    connect(orderSlider, &QSlider::valueChanged, [this](int value) {
        m_order = value;
        orderSpinBox->blockSignals(true);
        orderSpinBox->setValue(m_order);
        orderSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(orderSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_order = value;
        orderSlider->blockSignals(true);
        orderSlider->setValue(value);
        orderSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Level
    connect(levelSlider, &QSlider::valueChanged, [this](int value) {
        m_level = value / 100.0;
        levelSpinBox->blockSignals(true);
        levelSpinBox->setValue(m_level);
        levelSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_level = value;
        levelSlider->blockSignals(true);
        levelSlider->setValue(static_cast<int>(value * 100));
        levelSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAtilt::updateFFmpegFlags() {
    ffmpegFlags = QString("atilt=freq=%1:slope=%2:width=%3:order=%4:level=%5")
                      .arg(m_freq, 0, 'f', 0)
                      .arg(m_slope, 0, 'f', 2)
                      .arg(m_width, 0, 'f', 0)
                      .arg(m_order)
                      .arg(m_level, 0, 'f', 2);
}

QString FFAtilt::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAtilt::toJSON(QJsonObject& json) const {
    json["type"] = "ff-atilt";
    json["freq"] = m_freq;
    json["slope"] = m_slope;
    json["width"] = m_width;
    json["order"] = m_order;
    json["level"] = m_level;
}

void FFAtilt::fromJSON(const QJsonObject& json) {
    m_freq = json["freq"].toDouble(10000.0);
    m_slope = json["slope"].toDouble(0.0);
    m_width = json["width"].toDouble(1000.0);
    m_order = json["order"].toInt(5);
    m_level = json["level"].toDouble(1.0);
    updateFFmpegFlags();
}
