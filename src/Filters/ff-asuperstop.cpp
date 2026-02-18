#include "ff-asuperstop.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAsuperstop::FFAsuperstop() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsuperstop::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "High-order Butterworth band-stop (notch) filter.\n"
        "Attenuates frequencies around center, passes both low and high."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Center frequency (20 - 20000 Hz for practical use)
    auto centerfGroup = new QGroupBox("Center Frequency (Hz)");
    auto centerfLayout = new QVBoxLayout(centerfGroup);

    centerfSpinBox = new QDoubleSpinBox();
    centerfSpinBox->setMinimum(20.0);
    centerfSpinBox->setMaximum(20000.0);
    centerfSpinBox->setValue(m_centerf);
    centerfSpinBox->setDecimals(1);
    centerfSpinBox->setSingleStep(10.0);
    centerfSpinBox->setSuffix(" Hz");

    auto centerfControlLayout = new QHBoxLayout();
    centerfControlLayout->addWidget(centerfSpinBox);
    centerfControlLayout->addStretch();
    centerfControlLayout->addWidget(new QLabel("20"));
    centerfSlider = new QSlider(Qt::Horizontal);
    centerfSlider->setMinimum(20);
    centerfSlider->setMaximum(20000);
    centerfSlider->setValue(static_cast<int>(m_centerf));
    centerfControlLayout->addWidget(centerfSlider, 1);
    centerfControlLayout->addWidget(new QLabel("20k"));

    auto centerfHint = new QLabel("Frequency to notch out (e.g. 50/60 Hz for hum removal)");
    centerfHint->setStyleSheet("color: gray; font-size: 11px;");
    centerfLayout->addLayout(centerfControlLayout);
    centerfLayout->addWidget(centerfHint);
    mainLayout->addWidget(centerfGroup);

    // Q Factor (0.01 - 100)
    auto qfactorGroup = new QGroupBox("Q Factor (Notch Width)");
    auto qfactorLayout = new QVBoxLayout(qfactorGroup);

    qfactorSpinBox = new QDoubleSpinBox();
    qfactorSpinBox->setMinimum(0.01);
    qfactorSpinBox->setMaximum(100.0);
    qfactorSpinBox->setValue(m_qfactor);
    qfactorSpinBox->setDecimals(2);
    qfactorSpinBox->setSingleStep(0.1);

    auto qfactorControlLayout = new QHBoxLayout();
    qfactorControlLayout->addWidget(qfactorSpinBox);
    qfactorControlLayout->addStretch();
    qfactorControlLayout->addWidget(new QLabel("0.01"));
    qfactorSlider = new QSlider(Qt::Horizontal);
    qfactorSlider->setMinimum(1);
    qfactorSlider->setMaximum(10000);
    qfactorSlider->setValue(static_cast<int>(m_qfactor * 100));
    qfactorControlLayout->addWidget(qfactorSlider, 1);
    qfactorControlLayout->addWidget(new QLabel("100"));

    auto qfactorHint = new QLabel("Higher Q = narrower notch (more surgical)");
    qfactorHint->setStyleSheet("color: gray; font-size: 11px;");
    qfactorLayout->addLayout(qfactorControlLayout);
    qfactorLayout->addWidget(qfactorHint);
    mainLayout->addWidget(qfactorGroup);

    // Order (4 - 20)
    auto orderGroup = new QGroupBox("Filter Order");
    auto orderLayout = new QVBoxLayout(orderGroup);

    orderSpinBox = new QSpinBox();
    orderSpinBox->setMinimum(4);
    orderSpinBox->setMaximum(20);
    orderSpinBox->setValue(m_order);

    auto orderControlLayout = new QHBoxLayout();
    orderControlLayout->addWidget(orderSpinBox);
    orderControlLayout->addStretch();
    orderControlLayout->addWidget(new QLabel("4"));
    orderSlider = new QSlider(Qt::Horizontal);
    orderSlider->setMinimum(4);
    orderSlider->setMaximum(20);
    orderSlider->setValue(m_order);
    orderControlLayout->addWidget(orderSlider, 1);
    orderControlLayout->addWidget(new QLabel("20"));

    auto orderHint = new QLabel("Higher = deeper notch");
    orderHint->setStyleSheet("color: gray; font-size: 11px;");
    orderLayout->addLayout(orderControlLayout);
    orderLayout->addWidget(orderHint);
    mainLayout->addWidget(orderGroup);

    // Level (0 - 200%)
    auto levelGroup = new QGroupBox("Input Level");
    auto levelLayout = new QVBoxLayout(levelGroup);

    levelSpinBox = new QDoubleSpinBox();
    levelSpinBox->setMinimum(0.0);
    levelSpinBox->setMaximum(200.0);
    levelSpinBox->setValue(m_level * 100.0);
    levelSpinBox->setDecimals(1);
    levelSpinBox->setSingleStep(5.0);
    levelSpinBox->setSuffix(" %");

    auto levelControlLayout = new QHBoxLayout();
    levelControlLayout->addWidget(levelSpinBox);
    levelControlLayout->addStretch();
    levelControlLayout->addWidget(new QLabel("0%"));
    levelSlider = new QSlider(Qt::Horizontal);
    levelSlider->setMinimum(0);
    levelSlider->setMaximum(200);
    levelSlider->setValue(static_cast<int>(m_level * 100));
    levelControlLayout->addWidget(levelSlider, 1);
    levelControlLayout->addWidget(new QLabel("200%"));

    levelLayout->addLayout(levelControlLayout);
    mainLayout->addWidget(levelGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Center frequency
    connect(centerfSlider, &QSlider::valueChanged, [this](int value) {
        m_centerf = value;
        centerfSpinBox->blockSignals(true);
        centerfSpinBox->setValue(m_centerf);
        centerfSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(centerfSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_centerf = value;
        centerfSlider->blockSignals(true);
        centerfSlider->setValue(static_cast<int>(value));
        centerfSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Q factor
    connect(qfactorSlider, &QSlider::valueChanged, [this](int value) {
        m_qfactor = value / 100.0;
        qfactorSpinBox->blockSignals(true);
        qfactorSpinBox->setValue(m_qfactor);
        qfactorSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(qfactorSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_qfactor = value;
        qfactorSlider->blockSignals(true);
        qfactorSlider->setValue(static_cast<int>(value * 100));
        qfactorSlider->blockSignals(false);
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
        levelSpinBox->setValue(value);
        levelSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_level = value / 100.0;
        levelSlider->blockSignals(true);
        levelSlider->setValue(static_cast<int>(value));
        levelSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAsuperstop::updateFFmpegFlags() {
    ffmpegFlags = QString("asuperstop=centerf=%1:order=%2:qfactor=%3:level=%4")
                      .arg(m_centerf, 0, 'f', 1)
                      .arg(m_order)
                      .arg(m_qfactor, 0, 'f', 2)
                      .arg(m_level, 0, 'f', 2);
}

QString FFAsuperstop::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsuperstop::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asuperstop";
    json["centerf"] = m_centerf;
    json["order"] = m_order;
    json["qfactor"] = m_qfactor;
    json["level"] = m_level;
}

void FFAsuperstop::fromJSON(const QJsonObject& json) {
    m_centerf = json["centerf"].toDouble(1000.0);
    m_order = json["order"].toInt(4);
    m_qfactor = json["qfactor"].toDouble(1.0);
    m_level = json["level"].toDouble(1.0);
    updateFFmpegFlags();
}
