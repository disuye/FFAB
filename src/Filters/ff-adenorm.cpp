#include "ff-adenorm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAdenorm::FFAdenorm() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdenorm::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Level parameter (-451 to -90 dB)
    auto levelGroup = new QGroupBox("Noise Level (dB)");
    auto levelLayout = new QVBoxLayout(levelGroup);

    levelSpinBox = new QDoubleSpinBox();
    levelSpinBox->setMinimum(-451.0);
    levelSpinBox->setMaximum(-90.0);
    levelSpinBox->setValue(m_level);
    levelSpinBox->setDecimals(1);
    levelSpinBox->setSingleStep(1.0);
    levelSpinBox->setSuffix(" dB");

    auto levelControlLayout = new QHBoxLayout();
    levelControlLayout->addWidget(levelSpinBox);
    levelControlLayout->addStretch();
    
    auto levelMinLabel = new QLabel("-451");
    levelControlLayout->addWidget(levelMinLabel);

    levelSlider = new QSlider(Qt::Horizontal);
    levelSlider->setMinimum(-4510);
    levelSlider->setMaximum(-900);
    levelSlider->setValue(static_cast<int>(m_level * 10));
    levelControlLayout->addWidget(levelSlider, 1);

    auto levelMaxLabel = new QLabel("-90");
    levelControlLayout->addWidget(levelMaxLabel);

    levelLayout->addLayout(levelControlLayout);

    auto levelHint = new QLabel("Level of noise to add. Lower values are quieter but may not fully prevent denormals.");
    levelHint->setStyleSheet("color: gray; font-size: 11px;");
    levelHint->setWordWrap(true);
    levelLayout->addWidget(levelHint);

    mainLayout->addWidget(levelGroup);

    // Type parameter
    auto typeGroup = new QGroupBox("Noise Type");
    auto typeLayout = new QVBoxLayout(typeGroup);

    typeCombo = new QComboBox();
    typeCombo->addItem("DC Offset", 0);
    typeCombo->addItem("AC (Alternating)", 1);
    typeCombo->addItem("Square Wave", 2);
    typeCombo->addItem("Pulse", 3);
    typeCombo->setCurrentIndex(m_type);

    typeLayout->addWidget(typeCombo);

    auto typeHint = new QLabel("Type of noise signal used to remedy denormals.");
    typeHint->setStyleSheet("color: gray; font-size: 11px;");
    typeLayout->addWidget(typeHint);

    mainLayout->addWidget(typeGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Level
    connect(levelSlider, &QSlider::valueChanged, [this](int value) {
        m_level = value / 10.0;
        levelSpinBox->blockSignals(true);
        levelSpinBox->setValue(m_level);
        levelSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(levelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_level = value;
        levelSlider->blockSignals(true);
        levelSlider->setValue(static_cast<int>(value * 10));
        levelSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Type
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_type = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdenorm::updateFFmpegFlags() {
    QStringList typeNames = {"dc", "ac", "square", "pulse"};
    QString typeName = (m_type >= 0 && m_type < typeNames.size()) ? typeNames[m_type] : "dc";
    
    ffmpegFlags = QString("adenorm=level=%1:type=%2")
                      .arg(m_level, 0, 'f', 1)
                      .arg(typeName);
}

QString FFAdenorm::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdenorm::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adenorm";
    json["level"] = m_level;
    json["noise_type"] = m_type;
}

void FFAdenorm::fromJSON(const QJsonObject& json) {
    m_level = json["level"].toDouble(-351.0);
    m_type = json["noise_type"].toInt(0);
    updateFFmpegFlags();
}
