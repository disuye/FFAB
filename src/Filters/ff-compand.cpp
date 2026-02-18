#include "ff-compand.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFCompand::FFCompand() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFCompand::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Attacks parameter (string)
    auto attacksGroup = new QGroupBox("Attack Times");
    auto attacksLayout = new QVBoxLayout(attacksGroup);

    auto attacksLabel = new QLabel("Comma or space separated attack times in seconds:");
    attacksLabel->setWordWrap(true);
    attacksLayout->addWidget(attacksLabel);

    attacksEdit = new QLineEdit();
    attacksEdit->setText(m_attacks);
    attacksEdit->setPlaceholderText("0.1,0.2,0.3 or 0.1 0.2 0.3");
    
    connect(attacksEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_attacks = text;
        updateFFmpegFlags();
    });

    attacksLayout->addWidget(attacksEdit);
    mainLayout->addWidget(attacksGroup);

    // Decays parameter (string)
    auto decaysGroup = new QGroupBox("Decay Times");
    auto decaysLayout = new QVBoxLayout(decaysGroup);

    auto decaysLabel = new QLabel("Comma or space separated decay times in seconds:");
    decaysLabel->setWordWrap(true);
    decaysLayout->addWidget(decaysLabel);

    decaysEdit = new QLineEdit();
    decaysEdit->setText(m_decays);
    decaysEdit->setPlaceholderText("0.8,0.9,1.0 or 0.8 0.9 1.0");
    
    connect(decaysEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_decays = text;
        updateFFmpegFlags();
    });

    decaysLayout->addWidget(decaysEdit);
    mainLayout->addWidget(decaysGroup);

    // Points parameter (string - transfer function)
    auto pointsGroup = new QGroupBox("Transfer Function Points");
    auto pointsLayout = new QVBoxLayout(pointsGroup);

    auto pointsLabel = new QLabel("Pipe-separated input/output pairs (in dB):");
    pointsLabel->setWordWrap(true);
    pointsLayout->addWidget(pointsLabel);

    pointsEdit = new QLineEdit();
    pointsEdit->setText(m_points);
    pointsEdit->setPlaceholderText("-70/-70|-60/-20|1/0");
    
    connect(pointsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_points = text;
        updateFFmpegFlags();
    });

    pointsLayout->addWidget(pointsEdit);

    auto pointsHint = new QLabel("<i>Format: input_dB/output_dB|input_dB/output_dB|...</i>");
    pointsHint->setStyleSheet("color: gray; font-size: 10px;");
    pointsLayout->addWidget(pointsHint);

    mainLayout->addWidget(pointsGroup);

    // Gain parameter (-900 to 900 dB)
    auto gainGroup = new QGroupBox("Output Gain (dB)");
    auto gainLayout = new QVBoxLayout(gainGroup);

    gainSpinBox = new QDoubleSpinBox();
    gainSpinBox->setMinimum(-900.0);
    gainSpinBox->setMaximum(900.0);
    gainSpinBox->setValue(m_gain);
    gainSpinBox->setDecimals(2);
    gainSpinBox->setSingleStep(0.1);
    gainSpinBox->setSuffix(" dB");

    auto gainControlLayout = new QHBoxLayout();
    gainControlLayout->addWidget(gainSpinBox);
    gainControlLayout->addStretch();
    
    auto gainMinLabel = new QLabel("-30");
    gainControlLayout->addWidget(gainMinLabel);

    gainSlider = new QSlider(Qt::Horizontal);
    gainSlider->setMinimum(-3000);  // -30 dB for UI purposes
    gainSlider->setMaximum(3000);   // +30 dB for UI purposes
    gainSlider->setValue(0);        // default 0
    gainControlLayout->addWidget(gainSlider, 1);

    auto gainMaxLabel = new QLabel("+30");
    gainControlLayout->addWidget(gainMaxLabel);

    gainLayout->addLayout(gainControlLayout);
    mainLayout->addWidget(gainGroup);

    // Volume parameter (-900 to 0 dB)
    auto volumeGroup = new QGroupBox("Initial Volume (dB)");
    auto volumeLayout = new QVBoxLayout(volumeGroup);

    volumeSpinBox = new QDoubleSpinBox();
    volumeSpinBox->setMinimum(-900.0);
    volumeSpinBox->setMaximum(0.0);
    volumeSpinBox->setValue(m_volume);
    volumeSpinBox->setDecimals(2);
    volumeSpinBox->setSingleStep(0.1);
    volumeSpinBox->setSuffix(" dB");

    auto volumeControlLayout = new QHBoxLayout();
    volumeControlLayout->addWidget(volumeSpinBox);
    volumeControlLayout->addStretch();
    
    auto volumeMinLabel = new QLabel("-30");
    volumeControlLayout->addWidget(volumeMinLabel);

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setMinimum(-3000);  // -30 dB for UI
    volumeSlider->setMaximum(0);
    volumeSlider->setValue(0);        // default 0
    volumeControlLayout->addWidget(volumeSlider, 1);

    auto volumeMaxLabel = new QLabel("0");
    volumeControlLayout->addWidget(volumeMaxLabel);

    volumeLayout->addLayout(volumeControlLayout);
    mainLayout->addWidget(volumeGroup);

    // Delay parameter (0 to 20 seconds)
    auto delayGroup = new QGroupBox("Delay (seconds)");
    auto delayLayout = new QVBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(0.0);
    delaySpinBox->setMaximum(20.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(2);
    delaySpinBox->setSingleStep(0.1);
    delaySpinBox->setSuffix(" s");

    auto delayControlLayout = new QHBoxLayout();
    delayControlLayout->addWidget(delaySpinBox);
    delayControlLayout->addStretch();
    
    auto delayMinLabel = new QLabel("0");
    delayControlLayout->addWidget(delayMinLabel);

    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(0);
    delaySlider->setMaximum(2000);  // 20 seconds * 100
    delaySlider->setValue(0);       // default 0
    delayControlLayout->addWidget(delaySlider, 1);

    auto delayMaxLabel = new QLabel("20");
    delayControlLayout->addWidget(delayMaxLabel);

    delayLayout->addLayout(delayControlLayout);
    mainLayout->addWidget(delayGroup);

    // Connect gain slider and spinbox
    connect(gainSlider, &QSlider::valueChanged, [this](int value) {
        m_gain = value / 100.0;
        
        gainSpinBox->blockSignals(true);
        gainSpinBox->setValue(m_gain);
        gainSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(gainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_gain = value;
        
        gainSlider->blockSignals(true);
        gainSlider->setValue(static_cast<int>(value * 100));
        gainSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect volume slider and spinbox
    connect(volumeSlider, &QSlider::valueChanged, [this](int value) {
        m_volume = value / 100.0;
        
        volumeSpinBox->blockSignals(true);
        volumeSpinBox->setValue(m_volume);
        volumeSpinBox->blockSignals(false);
        
        updateFFmpegFlags();
    });

    connect(volumeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_volume = value;
        
        volumeSlider->blockSignals(true);
        volumeSlider->setValue(static_cast<int>(value * 100));
        volumeSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Connect delay slider and spinbox
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

    mainLayout->addStretch();

    return parametersWidget;
}

void FFCompand::updateFFmpegFlags() {
    ffmpegFlags = QString("compand=attacks=%1:decays=%2:points=%3:gain=%4:volume=%5:delay=%6")
                      .arg(m_attacks)
                      .arg(m_decays)
                      .arg(m_points)
                      .arg(m_gain, 0, 'f', 1)
                      .arg(m_volume, 0, 'f', 1)
                      .arg(m_delay, 0, 'f', 2);
}

QString FFCompand::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFCompand::toJSON(QJsonObject& json) const {
    json["type"] = "ff-compand";
    json["attacks"] = m_attacks;
    json["decays"] = m_decays;
    json["points"] = m_points;
    json["gain"] = m_gain;
    json["volume"] = m_volume;
    json["delay"] = m_delay;
}

void FFCompand::fromJSON(const QJsonObject& json) {
    m_attacks = json["attacks"].toString("0");
    m_decays = json["decays"].toString("0.8");
    m_points = json["points"].toString("-70/-70|-60/-20|1/0");
    m_gain = json["gain"].toDouble(0.0);
    m_volume = json["volume"].toDouble(0.0);
    m_delay = json["delay"].toDouble(0.0);
    updateFFmpegFlags();
}
