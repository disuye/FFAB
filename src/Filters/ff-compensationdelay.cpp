#include "ff-compensationdelay.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <QGridLayout>
#include <cmath>

FFCompensationdelay::FFCompensationdelay() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFCompensationdelay::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: DISTANCE ==========
    auto distanceTab = new QWidget();
    auto distanceLayout = new QVBoxLayout(distanceTab);

    // Info label
    auto infoLabel = new QLabel(
        "Delay based on physical distance (speed of sound).\n"
        "Useful for time-aligning multiple microphones or speakers."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    distanceLayout->addWidget(infoLabel);

    // Distance inputs
    auto distGroup = new QGroupBox("Distance");
    auto distGridLayout = new QGridLayout(distGroup);

    // Meters
    distGridLayout->addWidget(new QLabel("Meters:"), 0, 0);
    mSpinBox = new QSpinBox();
    mSpinBox->setMinimum(0);
    mSpinBox->setMaximum(100);
    mSpinBox->setValue(m_m);
    mSpinBox->setSuffix(" m");
    distGridLayout->addWidget(mSpinBox, 0, 1);

    // Centimeters
    distGridLayout->addWidget(new QLabel("Centimeters:"), 1, 0);
    cmSpinBox = new QSpinBox();
    cmSpinBox->setMinimum(0);
    cmSpinBox->setMaximum(99);
    cmSpinBox->setValue(m_cm);
    cmSpinBox->setSuffix(" cm");
    distGridLayout->addWidget(cmSpinBox, 1, 1);

    // Millimeters
    distGridLayout->addWidget(new QLabel("Millimeters:"), 2, 0);
    mmSpinBox = new QSpinBox();
    mmSpinBox->setMinimum(0);
    mmSpinBox->setMaximum(9);
    mmSpinBox->setValue(m_mm);
    mmSpinBox->setSuffix(" mm");
    distGridLayout->addWidget(mmSpinBox, 2, 1);

    distanceLayout->addWidget(distGroup);

    // Total distance display
    auto totalGroup = new QGroupBox("Calculated Delay");
    auto totalLayout = new QVBoxLayout(totalGroup);

    totalDistanceLabel = new QLabel();
    totalDistanceLabel->setStyleSheet("font-weight: bold;");
    delayMsLabel = new QLabel();
    delayMsLabel->setStyleSheet("color: #666;");

    totalLayout->addWidget(totalDistanceLabel);
    totalLayout->addWidget(delayMsLabel);
    distanceLayout->addWidget(totalGroup);

    distanceLayout->addStretch();
    tabWidget->addTab(distanceTab, "Distance");

    // ========== TAB 2: MIX ==========
    auto mixTab = new QWidget();
    auto mixLayout = new QVBoxLayout(mixTab);

    // Dry (0 - 100%)
    auto dryGroup = new QGroupBox("Dry Signal");
    auto dryLayout = new QVBoxLayout(dryGroup);

    drySpinBox = new QDoubleSpinBox();
    drySpinBox->setMinimum(0.0);
    drySpinBox->setMaximum(100.0);
    drySpinBox->setValue(m_dry * 100.0);
    drySpinBox->setDecimals(1);
    drySpinBox->setSingleStep(5.0);
    drySpinBox->setSuffix(" %");

    auto dryControlLayout = new QHBoxLayout();
    dryControlLayout->addWidget(drySpinBox);
    dryControlLayout->addStretch();
    dryControlLayout->addWidget(new QLabel("0%"));
    drySlider = new QSlider(Qt::Horizontal);
    drySlider->setMinimum(0);
    drySlider->setMaximum(100);
    drySlider->setValue(static_cast<int>(m_dry * 100));
    dryControlLayout->addWidget(drySlider, 1);
    dryControlLayout->addWidget(new QLabel("100%"));

    auto dryHint = new QLabel("Undelayed original signal");
    dryHint->setStyleSheet("color: gray; font-size: 11px;");
    dryLayout->addLayout(dryControlLayout);
    dryLayout->addWidget(dryHint);
    mixLayout->addWidget(dryGroup);

    // Wet (0 - 100%)
    auto wetGroup = new QGroupBox("Wet Signal");
    auto wetLayout = new QVBoxLayout(wetGroup);

    wetSpinBox = new QDoubleSpinBox();
    wetSpinBox->setMinimum(0.0);
    wetSpinBox->setMaximum(100.0);
    wetSpinBox->setValue(m_wet * 100.0);
    wetSpinBox->setDecimals(1);
    wetSpinBox->setSingleStep(5.0);
    wetSpinBox->setSuffix(" %");

    auto wetControlLayout = new QHBoxLayout();
    wetControlLayout->addWidget(wetSpinBox);
    wetControlLayout->addStretch();
    wetControlLayout->addWidget(new QLabel("0%"));
    wetSlider = new QSlider(Qt::Horizontal);
    wetSlider->setMinimum(0);
    wetSlider->setMaximum(100);
    wetSlider->setValue(static_cast<int>(m_wet * 100));
    wetControlLayout->addWidget(wetSlider, 1);
    wetControlLayout->addWidget(new QLabel("100%"));

    auto wetHint = new QLabel("Delayed signal");
    wetHint->setStyleSheet("color: gray; font-size: 11px;");
    wetLayout->addLayout(wetControlLayout);
    wetLayout->addWidget(wetHint);
    mixLayout->addWidget(wetGroup);

    mixLayout->addStretch();
    tabWidget->addTab(mixTab, "Mix");

    // ========== TAB 3: ENVIRONMENT ==========
    auto envTab = new QWidget();
    auto envLayout = new QVBoxLayout(envTab);

    // Temperature (-50 to 50 °C)
    auto tempGroup = new QGroupBox("Temperature (°C)");
    auto tempLayout = new QVBoxLayout(tempGroup);

    tempSpinBox = new QSpinBox();
    tempSpinBox->setMinimum(-50);
    tempSpinBox->setMaximum(50);
    tempSpinBox->setValue(m_temp);
    tempSpinBox->setSuffix(" °C");

    auto tempControlLayout = new QHBoxLayout();
    tempControlLayout->addWidget(tempSpinBox);
    tempControlLayout->addStretch();
    tempControlLayout->addWidget(new QLabel("-50"));
    tempSlider = new QSlider(Qt::Horizontal);
    tempSlider->setMinimum(-50);
    tempSlider->setMaximum(50);
    tempSlider->setValue(m_temp);
    tempControlLayout->addWidget(tempSlider, 1);
    tempControlLayout->addWidget(new QLabel("50"));

    auto tempHint = new QLabel("Affects speed of sound calculation (~343 m/s at 20°C)");
    tempHint->setStyleSheet("color: gray; font-size: 11px;");
    tempLayout->addLayout(tempControlLayout);
    tempLayout->addWidget(tempHint);
    envLayout->addWidget(tempGroup);

    envLayout->addStretch();
    tabWidget->addTab(envTab, "Environment");

    mainLayout->addWidget(tabWidget);

    // Initialize display
    updateTotalDistance();

    // ========== SIGNAL CONNECTIONS ==========

    // Distance
    connect(mSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_m = value;
        updateTotalDistance();
        updateFFmpegFlags();
    });
    connect(cmSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_cm = value;
        updateTotalDistance();
        updateFFmpegFlags();
    });
    connect(mmSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_mm = value;
        updateTotalDistance();
        updateFFmpegFlags();
    });

    // Dry
    connect(drySlider, &QSlider::valueChanged, [this](int value) {
        m_dry = value / 100.0;
        drySpinBox->blockSignals(true);
        drySpinBox->setValue(value);
        drySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(drySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_dry = value / 100.0;
        drySlider->blockSignals(true);
        drySlider->setValue(static_cast<int>(value));
        drySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Wet
    connect(wetSlider, &QSlider::valueChanged, [this](int value) {
        m_wet = value / 100.0;
        wetSpinBox->blockSignals(true);
        wetSpinBox->setValue(value);
        wetSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(wetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_wet = value / 100.0;
        wetSlider->blockSignals(true);
        wetSlider->setValue(static_cast<int>(value));
        wetSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Temperature
    connect(tempSlider, &QSlider::valueChanged, [this](int value) {
        m_temp = value;
        tempSpinBox->blockSignals(true);
        tempSpinBox->setValue(m_temp);
        tempSpinBox->blockSignals(false);
        updateTotalDistance();
        updateFFmpegFlags();
    });
    connect(tempSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_temp = value;
        tempSlider->blockSignals(true);
        tempSlider->setValue(value);
        tempSlider->blockSignals(false);
        updateTotalDistance();
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFCompensationdelay::updateTotalDistance() {
    double totalMeters = m_m + (m_cm / 100.0) + (m_mm / 1000.0);
    
    // Speed of sound: approximately 331.3 + 0.606 * T (m/s) where T is in Celsius
    double speedOfSound = 331.3 + 0.606 * m_temp;
    double delayMs = (totalMeters / speedOfSound) * 1000.0;
    
    if (totalDistanceLabel) {
        totalDistanceLabel->setText(QString("Total: %1 m").arg(totalMeters, 0, 'f', 3));
    }
    if (delayMsLabel) {
        delayMsLabel->setText(QString("Delay: %1 ms (at %2 m/s)")
                                  .arg(delayMs, 0, 'f', 3)
                                  .arg(speedOfSound, 0, 'f', 1));
    }
}

void FFCompensationdelay::updateFFmpegFlags() {
    ffmpegFlags = QString("compensationdelay=mm=%1:cm=%2:m=%3:dry=%4:wet=%5:temp=%6")
                      .arg(m_mm)
                      .arg(m_cm)
                      .arg(m_m)
                      .arg(m_dry, 0, 'f', 2)
                      .arg(m_wet, 0, 'f', 2)
                      .arg(m_temp);
}

QString FFCompensationdelay::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFCompensationdelay::toJSON(QJsonObject& json) const {
    json["type"] = "ff-compensationdelay";
    json["mm"] = m_mm;
    json["cm"] = m_cm;
    json["m"] = m_m;
    json["dry"] = m_dry;
    json["wet"] = m_wet;
    json["temp"] = m_temp;
}

void FFCompensationdelay::fromJSON(const QJsonObject& json) {
    m_mm = json["mm"].toInt(0);
    m_cm = json["cm"].toInt(0);
    m_m = json["m"].toInt(0);
    m_dry = json["dry"].toDouble(0.0);
    m_wet = json["wet"].toDouble(1.0);
    m_temp = json["temp"].toInt(20);
    updateFFmpegFlags();
}
