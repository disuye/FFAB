#include "ff-aformat.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>

FFAformat::FFAformat() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAformat::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Format"));

    // Preset sample rates
    auto presetLayout = new QHBoxLayout();
    presetLayout->addWidget(new QLabel("Sample Rate:"));
    
    presetCombo = new QComboBox();
    presetCombo->addItem("192000 Hz", 192000);
    presetCombo->addItem("176400 Hz", 176400);
    presetCombo->addItem("96000 Hz", 96000);
    presetCombo->addItem("88200 Hz", 88200);
    presetCombo->addItem("64000 Hz", 64000);
    presetCombo->addItem("48000 Hz", 48000);
    presetCombo->addItem("44100 Hz", 44100);
    presetCombo->addItem("32000 Hz", 32000);
    presetCombo->addItem("24000 Hz", 24000);
    presetCombo->addItem("22050 Hz", 22050);
    presetCombo->addItem("16000 Hz", 16000);
    presetCombo->addItem("12000 Hz", 12000);
    presetCombo->addItem("11025 Hz", 11025);
    presetCombo->addItem("8000 Hz", 8000);
    presetCombo->setCurrentIndex(6); // 44100 Hz
    presetLayout->addWidget(presetCombo);
    presetLayout->addStretch();
    
    layout->addLayout(presetLayout);

    // Custom checkbox
    customCheckBox = new QCheckBox("Custom Sample Rate");
    customCheckBox->setChecked(m_useCustomRate);
    layout->addWidget(customCheckBox);

    // Custom rate input
    auto customLayout = new QHBoxLayout();
    customLayout->addWidget(new QLabel("Custom Rate:"));
    
    customSpinBox = new QSpinBox();
    customSpinBox->setMinimum(2000);
    customSpinBox->setMaximum(192000);
    customSpinBox->setValue(m_customRate);
    customSpinBox->setSuffix(" Hz");
    customSpinBox->setEnabled(m_useCustomRate);
    customLayout->addWidget(customSpinBox);
    customLayout->addStretch();
    
    layout->addLayout(customLayout);

    // Connect signals
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_presetRate = presetCombo->itemData(index).toInt();
        if (!m_useCustomRate) {
            updateFFmpegFlags();
        }
    });

    connect(customCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_useCustomRate = checked;
        if (customSpinBox) {
            customSpinBox->setEnabled(checked);
        }
        if (presetCombo) {
            presetCombo->setEnabled(!checked);
        }
        updateFFmpegFlags();
    });

    connect(customSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_customRate = value;
        if (m_useCustomRate) {
            updateFFmpegFlags();
        }
    });

    layout->addStretch();
    return parametersWidget;
}

void FFAformat::updateFFmpegFlags() {
    int rate = m_useCustomRate ? m_customRate : m_presetRate;
    ffmpegFlags = QString("aformat=sample_rates=%1").arg(rate);
}

QString FFAformat::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAformat::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aformat";
    json["use_custom"] = m_useCustomRate;
    json["preset_rate"] = m_presetRate;
    json["custom_rate"] = m_customRate;
}

void FFAformat::fromJSON(const QJsonObject& json) {
    m_useCustomRate = json["use_custom"].toBool(false);
    m_presetRate = json["preset_rate"].toInt(44100);
    m_customRate = json["custom_rate"].toInt(44100);
    updateFFmpegFlags();
}
