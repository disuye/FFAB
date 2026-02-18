#include "ff-aphaseshift.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAphaseshift::FFAphaseshift() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAphaseshift::getParametersWidget() {
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

    // Shift parameter (-1 to 1)
    auto shiftGroup = new QGroupBox("Phase Shift");
    auto shiftLayout = new QVBoxLayout(shiftGroup);

    shiftSpinBox = new QDoubleSpinBox();
    shiftSpinBox->setMinimum(-1.0);
    shiftSpinBox->setMaximum(1.0);
    shiftSpinBox->setValue(m_shift);
    shiftSpinBox->setDecimals(2);
    shiftSpinBox->setSingleStep(0.01);

    auto shiftControlLayout = new QHBoxLayout();
    shiftControlLayout->addWidget(shiftSpinBox);
    shiftControlLayout->addStretch();
    
    auto shiftMinLabel = new QLabel("-1.0");
    shiftControlLayout->addWidget(shiftMinLabel);

    shiftSlider = new QSlider(Qt::Horizontal);
    shiftSlider->setMinimum(-100);
    shiftSlider->setMaximum(100);
    shiftSlider->setValue(0); // default 0
    shiftControlLayout->addWidget(shiftSlider, 1);

    auto shiftMaxLabel = new QLabel("+1.0");
    shiftControlLayout->addWidget(shiftMaxLabel);

    shiftLayout->addLayout(shiftControlLayout);
    mainLayout->addWidget(shiftGroup);

    // Level parameter (-96 to 0 dB)
    auto levelGroup = new QGroupBox("Output Level (dB)");
    auto levelLayout = new QVBoxLayout(levelGroup);

    levelSpinBox = new QDoubleSpinBox();
    levelSpinBox->setMinimum(-96.0);
    levelSpinBox->setMaximum(0.0);
    levelSpinBox->setValue(linearToDb(m_level));
    levelSpinBox->setDecimals(1);
    levelSpinBox->setSingleStep(0.1);
    levelSpinBox->setSuffix(" dB");

    auto levelControlLayout = new QHBoxLayout();
    levelControlLayout->addWidget(levelSpinBox);
    levelControlLayout->addStretch();

    auto levelMinLabel = new QLabel("-96");
    levelControlLayout->addWidget(levelMinLabel);

    levelSlider = new QSlider(Qt::Horizontal);
    levelSlider->setMinimum(0);
    levelSlider->setMaximum(960);
    levelSlider->setValue(static_cast<int>((linearToDb(m_level) + 96.0) * 10.0));
    levelControlLayout->addWidget(levelSlider, 1);

    auto levelMaxLabel = new QLabel("0");
    levelControlLayout->addWidget(levelMaxLabel);

    levelLayout->addLayout(levelControlLayout);
    mainLayout->addWidget(levelGroup);

    // Order parameter (1 - 16)
    auto orderGroup = new QGroupBox("Filter Order");
    auto orderLayout = new QVBoxLayout(orderGroup);

    orderSpinBox = new QDoubleSpinBox();
    orderSpinBox->setMinimum(1);
    orderSpinBox->setMaximum(16);
    orderSpinBox->setValue(m_order);
    orderSpinBox->setDecimals(0);

    auto orderControlLayout = new QHBoxLayout();
    orderControlLayout->addWidget(orderSpinBox);
    orderControlLayout->addStretch();
    
    auto orderMinLabel = new QLabel("1");
    orderControlLayout->addWidget(orderMinLabel);

    orderSlider = new QSlider(Qt::Horizontal);
    orderSlider->setMinimum(1);
    orderSlider->setMaximum(16);
    orderSlider->setValue(m_order);
    orderControlLayout->addWidget(orderSlider, 1);

    auto orderMaxLabel = new QLabel("16");
    orderControlLayout->addWidget(orderMaxLabel);

    orderLayout->addLayout(orderControlLayout);
    mainLayout->addWidget(orderGroup);

    // Connect shift slider and spinbox
    connect(shiftSlider, &QSlider::valueChanged, [this](int value) {
        m_shift = value / 100.0;
        shiftSpinBox->blockSignals(true);
        shiftSpinBox->setValue(m_shift);
        shiftSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(shiftSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_shift = value;
        shiftSlider->blockSignals(true);
        shiftSlider->setValue(static_cast<int>(value * 100));
        shiftSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Connect level slider and spinbox (dB conversion)
    connect(levelSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_level = dbToLinear(db);
        levelSpinBox->blockSignals(true);
        levelSpinBox->setValue(db);
        levelSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_level = dbToLinear(db);
        levelSlider->blockSignals(true);
        levelSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        levelSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Connect order slider and spinbox
    connect(orderSlider, &QSlider::valueChanged, [this](int value) {
        m_order = value;
        orderSpinBox->blockSignals(true);
        orderSpinBox->setValue(m_order);
        orderSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(orderSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_order = static_cast<int>(value);
        orderSlider->blockSignals(true);
        orderSlider->setValue(m_order);
        orderSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAphaseshift::updateFFmpegFlags() {
    ffmpegFlags = QString("aphaseshift=shift=%1:level=%2:order=%3")
                      .arg(m_shift, 0, 'f', 2)
                      .arg(m_level, 0, 'f', 2)
                      .arg(m_order);
}

QString FFAphaseshift::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAphaseshift::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aphaseshift";
    json["shift"] = m_shift;
    json["level"] = m_level;
    json["order"] = m_order;
}

void FFAphaseshift::fromJSON(const QJsonObject& json) {
    m_shift = json["shift"].toDouble(0.0);
    m_level = json["level"].toDouble(1.0);
    m_order = json["order"].toInt(8);
    updateFFmpegFlags();
}
