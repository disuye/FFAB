#include "ff-dialoguenhance.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFDialoguenhance::FFDialoguenhance() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFDialoguenhance::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "Enhances dialogue/voice clarity by extracting and boosting\n"
        "the center channel content from stereo audio."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Original level (0 - 100%)
    auto originalGroup = new QGroupBox("Original Center Level");
    auto originalLayout = new QVBoxLayout(originalGroup);

    originalSpinBox = new QDoubleSpinBox();
    originalSpinBox->setMinimum(0.0);
    originalSpinBox->setMaximum(100.0);
    originalSpinBox->setValue(m_original * 100.0);
    originalSpinBox->setDecimals(1);
    originalSpinBox->setSingleStep(5.0);
    originalSpinBox->setSuffix(" %");

    auto originalControlLayout = new QHBoxLayout();
    originalControlLayout->addWidget(originalSpinBox);
    originalControlLayout->addStretch();
    originalControlLayout->addWidget(new QLabel("0%"));
    originalSlider = new QSlider(Qt::Horizontal);
    originalSlider->setMinimum(0);
    originalSlider->setMaximum(100);
    originalSlider->setValue(static_cast<int>(m_original * 100));
    originalControlLayout->addWidget(originalSlider, 1);
    originalControlLayout->addWidget(new QLabel("100%"));

    auto originalHint = new QLabel("Amount of original center channel to preserve");
    originalHint->setStyleSheet("color: gray; font-size: 11px;");
    originalLayout->addLayout(originalControlLayout);
    originalLayout->addWidget(originalHint);
    mainLayout->addWidget(originalGroup);

    // Enhance level (0 - 300%)
    auto enhanceGroup = new QGroupBox("Dialogue Enhancement");
    auto enhanceLayout = new QVBoxLayout(enhanceGroup);

    enhanceSpinBox = new QDoubleSpinBox();
    enhanceSpinBox->setMinimum(0.0);
    enhanceSpinBox->setMaximum(300.0);
    enhanceSpinBox->setValue(m_enhance * 100.0);
    enhanceSpinBox->setDecimals(1);
    enhanceSpinBox->setSingleStep(10.0);
    enhanceSpinBox->setSuffix(" %");

    auto enhanceControlLayout = new QHBoxLayout();
    enhanceControlLayout->addWidget(enhanceSpinBox);
    enhanceControlLayout->addStretch();
    enhanceControlLayout->addWidget(new QLabel("0%"));
    enhanceSlider = new QSlider(Qt::Horizontal);
    enhanceSlider->setMinimum(0);
    enhanceSlider->setMaximum(300);
    enhanceSlider->setValue(static_cast<int>(m_enhance * 100));
    enhanceControlLayout->addWidget(enhanceSlider, 1);
    enhanceControlLayout->addWidget(new QLabel("300%"));

    auto enhanceHint = new QLabel("Amount of dialogue enhancement to apply");
    enhanceHint->setStyleSheet("color: gray; font-size: 11px;");
    enhanceLayout->addLayout(enhanceControlLayout);
    enhanceLayout->addWidget(enhanceHint);
    mainLayout->addWidget(enhanceGroup);

    // Voice detection (2 - 32)
    auto voiceGroup = new QGroupBox("Voice Detection Sensitivity");
    auto voiceLayout = new QVBoxLayout(voiceGroup);

    voiceSpinBox = new QDoubleSpinBox();
    voiceSpinBox->setMinimum(2.0);
    voiceSpinBox->setMaximum(32.0);
    voiceSpinBox->setValue(m_voice);
    voiceSpinBox->setDecimals(1);
    voiceSpinBox->setSingleStep(1.0);

    auto voiceControlLayout = new QHBoxLayout();
    voiceControlLayout->addWidget(voiceSpinBox);
    voiceControlLayout->addStretch();
    voiceControlLayout->addWidget(new QLabel("2"));
    voiceSlider = new QSlider(Qt::Horizontal);
    voiceSlider->setMinimum(20);
    voiceSlider->setMaximum(320);
    voiceSlider->setValue(static_cast<int>(m_voice * 10));
    voiceControlLayout->addWidget(voiceSlider, 1);
    voiceControlLayout->addWidget(new QLabel("32"));

    auto voiceHint = new QLabel("Higher = more selective voice detection");
    voiceHint->setStyleSheet("color: gray; font-size: 11px;");
    voiceLayout->addLayout(voiceControlLayout);
    voiceLayout->addWidget(voiceHint);
    mainLayout->addWidget(voiceGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Original
    connect(originalSlider, &QSlider::valueChanged, [this](int value) {
        m_original = value / 100.0;
        originalSpinBox->blockSignals(true);
        originalSpinBox->setValue(value);
        originalSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(originalSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_original = value / 100.0;
        originalSlider->blockSignals(true);
        originalSlider->setValue(static_cast<int>(value));
        originalSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Enhance
    connect(enhanceSlider, &QSlider::valueChanged, [this](int value) {
        m_enhance = value / 100.0;
        enhanceSpinBox->blockSignals(true);
        enhanceSpinBox->setValue(value);
        enhanceSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(enhanceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_enhance = value / 100.0;
        enhanceSlider->blockSignals(true);
        enhanceSlider->setValue(static_cast<int>(value));
        enhanceSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Voice
    connect(voiceSlider, &QSlider::valueChanged, [this](int value) {
        m_voice = value / 10.0;
        voiceSpinBox->blockSignals(true);
        voiceSpinBox->setValue(m_voice);
        voiceSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(voiceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_voice = value;
        voiceSlider->blockSignals(true);
        voiceSlider->setValue(static_cast<int>(value * 10));
        voiceSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFDialoguenhance::updateFFmpegFlags() {
    ffmpegFlags = QString("dialoguenhance=original=%1:enhance=%2:voice=%3")
                      .arg(m_original, 0, 'f', 2)
                      .arg(m_enhance, 0, 'f', 2)
                      .arg(m_voice, 0, 'f', 1);
}

QString FFDialoguenhance::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFDialoguenhance::toJSON(QJsonObject& json) const {
    json["type"] = "ff-dialoguenhance";
    json["original"] = m_original;
    json["enhance"] = m_enhance;
    json["voice"] = m_voice;
}

void FFDialoguenhance::fromJSON(const QJsonObject& json) {
    m_original = json["original"].toDouble(1.0);
    m_enhance = json["enhance"].toDouble(1.0);
    m_voice = json["voice"].toDouble(2.0);
    updateFFmpegFlags();
}
