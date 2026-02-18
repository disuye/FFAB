#include "ff-drmeter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFDrmeter::FFDrmeter() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFDrmeter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Dynamic Range Measurement");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Measures the dynamic range (DR) of the audio signal.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("Note: Audio passes through unchanged");
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // Length parameter (0.01 - 10 seconds)
    auto lengthGroup = new QGroupBox("Window Length (seconds)");
    auto lengthLayout = new QVBoxLayout(lengthGroup);

    lengthSpinBox = new QDoubleSpinBox();
    lengthSpinBox->setMinimum(0.01);
    lengthSpinBox->setMaximum(10.0);
    lengthSpinBox->setValue(m_length);
    lengthSpinBox->setDecimals(2);
    lengthSpinBox->setSingleStep(0.1);
    lengthSpinBox->setSuffix(" s");

    auto lengthControlLayout = new QHBoxLayout();
    lengthControlLayout->addWidget(lengthSpinBox);
    lengthControlLayout->addStretch();
    
    auto lengthMinLabel = new QLabel("0.01");
    lengthControlLayout->addWidget(lengthMinLabel);

    lengthSlider = new QSlider(Qt::Horizontal);
    lengthSlider->setMinimum(1);
    lengthSlider->setMaximum(1000);
    lengthSlider->setValue(static_cast<int>(m_length * 100));
    lengthControlLayout->addWidget(lengthSlider, 1);

    auto lengthMaxLabel = new QLabel("10.0");
    lengthControlLayout->addWidget(lengthMaxLabel);

    lengthLayout->addLayout(lengthControlLayout);

    auto lengthHint = new QLabel("Analysis window length. Longer windows give more stable readings.");
    lengthHint->setStyleSheet("color: gray; font-size: 11px;");
    lengthHint->setWordWrap(true);
    lengthLayout->addWidget(lengthHint);

    mainLayout->addWidget(lengthGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "Results are output to FFmpeg's stderr after processing:\n\n"
        "• DR value per channel (higher = more dynamic range)\n"
        "• Overall DR rating\n\n"
        "DR14+ = Excellent dynamics\n"
        "DR8-14 = Good dynamics\n"
        "DR0-8 = Heavily compressed"
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Length
    connect(lengthSlider, &QSlider::valueChanged, [this](int value) {
        m_length = value / 100.0;
        lengthSpinBox->blockSignals(true);
        lengthSpinBox->setValue(m_length);
        lengthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(lengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_length = value;
        lengthSlider->blockSignals(true);
        lengthSlider->setValue(static_cast<int>(value * 100));
        lengthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFDrmeter::updateFFmpegFlags() {
    ffmpegFlags = QString("drmeter=length=%1")
                      .arg(m_length, 0, 'f', 2);
}

QString FFDrmeter::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFDrmeter::toJSON(QJsonObject& json) const {
    json["type"] = "ff-drmeter";
    json["length"] = m_length;
}

void FFDrmeter::fromJSON(const QJsonObject& json) {
    m_length = json["length"].toDouble(3.0);
    updateFFmpegFlags();
}
