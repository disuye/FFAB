#include "ff-acontrast.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAcontrast::FFAcontrast() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcontrast::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "Simple dynamic range processor using a sigmoid transfer function.\n"
        "Low values compress, high values expand dynamic range."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Contrast parameter (0 - 100)
    auto contrastGroup = new QGroupBox("Contrast");
    auto contrastLayout = new QVBoxLayout(contrastGroup);

    contrastSpinBox = new QDoubleSpinBox();
    contrastSpinBox->setMinimum(0.0);
    contrastSpinBox->setMaximum(100.0);
    contrastSpinBox->setValue(m_contrast);
    contrastSpinBox->setDecimals(1);
    contrastSpinBox->setSingleStep(1.0);

    auto contrastControlLayout = new QHBoxLayout();
    contrastControlLayout->addWidget(contrastSpinBox);
    contrastControlLayout->addStretch();
    
    auto contrastMinLabel = new QLabel("0");
    contrastControlLayout->addWidget(contrastMinLabel);

    contrastSlider = new QSlider(Qt::Horizontal);
    contrastSlider->setMinimum(0);
    contrastSlider->setMaximum(1000);
    contrastSlider->setValue(static_cast<int>(m_contrast * 10));
    contrastControlLayout->addWidget(contrastSlider, 1);

    auto contrastMaxLabel = new QLabel("100");
    contrastControlLayout->addWidget(contrastMaxLabel);

    // Add scale hints
    auto scaleLayout = new QHBoxLayout();
    auto compressLabel = new QLabel("← Compress");
    compressLabel->setStyleSheet("color: gray; font-size: 10px;");
    auto expandLabel = new QLabel("Expand →");
    expandLabel->setStyleSheet("color: gray; font-size: 10px;");
    expandLabel->setAlignment(Qt::AlignRight);
    scaleLayout->addWidget(compressLabel);
    scaleLayout->addStretch();
    scaleLayout->addWidget(expandLabel);

    contrastLayout->addLayout(contrastControlLayout);
    contrastLayout->addLayout(scaleLayout);
    mainLayout->addWidget(contrastGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    connect(contrastSlider, &QSlider::valueChanged, [this](int value) {
        m_contrast = value / 10.0;
        contrastSpinBox->blockSignals(true);
        contrastSpinBox->setValue(m_contrast);
        contrastSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(contrastSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_contrast = value;
        contrastSlider->blockSignals(true);
        contrastSlider->setValue(static_cast<int>(value * 10));
        contrastSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAcontrast::updateFFmpegFlags() {
    ffmpegFlags = QString("acontrast=contrast=%1")
                      .arg(m_contrast, 0, 'f', 1);
}

QString FFAcontrast::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcontrast::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acontrast";
    json["contrast"] = m_contrast;
}

void FFAcontrast::fromJSON(const QJsonObject& json) {
    m_contrast = json["contrast"].toDouble(33.0);
    updateFFmpegFlags();
}
