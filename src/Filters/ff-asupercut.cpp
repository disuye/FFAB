#include "ff-asupercut.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAsupercut::FFAsupercut() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsupercut::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "High-order Butterworth lowpass filter for removing ultrasonic frequencies.\n"
        "Useful for anti-aliasing and removing inaudible high frequencies."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Cutoff parameter (20000 - 96000 Hz for practical use)
    auto cutoffGroup = new QGroupBox("Cutoff Frequency (Hz)");
    auto cutoffLayout = new QVBoxLayout(cutoffGroup);

    cutoffSpinBox = new QDoubleSpinBox();
    cutoffSpinBox->setMinimum(20000.0);
    cutoffSpinBox->setMaximum(96000.0);
    cutoffSpinBox->setValue(m_cutoff);
    cutoffSpinBox->setDecimals(0);
    cutoffSpinBox->setSingleStep(1000.0);
    cutoffSpinBox->setSuffix(" Hz");

    auto cutoffControlLayout = new QHBoxLayout();
    cutoffControlLayout->addWidget(cutoffSpinBox);
    cutoffControlLayout->addStretch();
    cutoffControlLayout->addWidget(new QLabel("20k"));
    cutoffSlider = new QSlider(Qt::Horizontal);
    cutoffSlider->setMinimum(20000);
    cutoffSlider->setMaximum(96000);
    cutoffSlider->setValue(static_cast<int>(m_cutoff));
    cutoffControlLayout->addWidget(cutoffSlider, 1);
    cutoffControlLayout->addWidget(new QLabel("96k"));

    auto cutoffHint = new QLabel("Frequencies above this are attenuated");
    cutoffHint->setStyleSheet("color: gray; font-size: 11px;");
    cutoffLayout->addLayout(cutoffControlLayout);
    cutoffLayout->addWidget(cutoffHint);
    mainLayout->addWidget(cutoffGroup);

    // Order parameter (3 - 20)
    auto orderGroup = new QGroupBox("Filter Order");
    auto orderLayout = new QVBoxLayout(orderGroup);

    orderSpinBox = new QSpinBox();
    orderSpinBox->setMinimum(3);
    orderSpinBox->setMaximum(20);
    orderSpinBox->setValue(m_order);

    auto orderControlLayout = new QHBoxLayout();
    orderControlLayout->addWidget(orderSpinBox);
    orderControlLayout->addStretch();
    orderControlLayout->addWidget(new QLabel("3"));
    orderSlider = new QSlider(Qt::Horizontal);
    orderSlider->setMinimum(3);
    orderSlider->setMaximum(20);
    orderSlider->setValue(m_order);
    orderControlLayout->addWidget(orderSlider, 1);
    orderControlLayout->addWidget(new QLabel("20"));

    auto orderHint = new QLabel("Higher = steeper rolloff (6 dB/octave per order)");
    orderHint->setStyleSheet("color: gray; font-size: 11px;");
    orderLayout->addLayout(orderControlLayout);
    orderLayout->addWidget(orderHint);
    mainLayout->addWidget(orderGroup);

    // Level parameter (0 - 100%)
    auto levelGroup = new QGroupBox("Input Level");
    auto levelLayout = new QVBoxLayout(levelGroup);

    levelSpinBox = new QDoubleSpinBox();
    levelSpinBox->setMinimum(0.0);
    levelSpinBox->setMaximum(100.0);
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
    levelSlider->setMaximum(100);
    levelSlider->setValue(static_cast<int>(m_level * 100));
    levelControlLayout->addWidget(levelSlider, 1);
    levelControlLayout->addWidget(new QLabel("100%"));

    levelLayout->addLayout(levelControlLayout);
    mainLayout->addWidget(levelGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Cutoff
    connect(cutoffSlider, &QSlider::valueChanged, [this](int value) {
        m_cutoff = value;
        cutoffSpinBox->blockSignals(true);
        cutoffSpinBox->setValue(m_cutoff);
        cutoffSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(cutoffSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_cutoff = value;
        cutoffSlider->blockSignals(true);
        cutoffSlider->setValue(static_cast<int>(value));
        cutoffSlider->blockSignals(false);
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

void FFAsupercut::updateFFmpegFlags() {
    ffmpegFlags = QString("asupercut=cutoff=%1:order=%2:level=%3")
                      .arg(m_cutoff, 0, 'f', 0)
                      .arg(m_order)
                      .arg(m_level, 0, 'f', 2);
}

QString FFAsupercut::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsupercut::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asupercut";
    json["cutoff"] = m_cutoff;
    json["order"] = m_order;
    json["level"] = m_level;
}

void FFAsupercut::fromJSON(const QJsonObject& json) {
    m_cutoff = json["cutoff"].toDouble(20000.0);
    m_order = json["order"].toInt(10);
    m_level = json["level"].toDouble(1.0);
    updateFFmpegFlags();
}
