#include "ff-adynamicsmooth.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAdynamicsmooth::FFAdynamicsmooth() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdynamicsmooth::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Dynamic Smoothing");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Applies an adaptive low-pass filter that smooths audio while "
        "preserving transients. The cutoff frequency automatically adapts "
        "based on how quickly the signal is changing."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Sensitivity parameter (0 - 1000, using log scale for large range)
    auto sensitivityGroup = new QGroupBox("Sensitivity");
    auto sensitivityLayout = new QVBoxLayout(sensitivityGroup);

    sensitivitySpinBox = new QDoubleSpinBox();
    sensitivitySpinBox->setMinimum(0.0);
    sensitivitySpinBox->setMaximum(1000.0);
    sensitivitySpinBox->setValue(m_sensitivity);
    sensitivitySpinBox->setDecimals(2);
    sensitivitySpinBox->setSingleStep(0.1);

    auto sensitivityControlLayout = new QHBoxLayout();
    sensitivityControlLayout->addWidget(sensitivitySpinBox);
    sensitivityControlLayout->addStretch();
    
    auto sensitivityMinLabel = new QLabel("0");
    sensitivityControlLayout->addWidget(sensitivityMinLabel);

    sensitivitySlider = new QSlider(Qt::Horizontal);
    sensitivitySlider->setMinimum(0);
    sensitivitySlider->setMaximum(1000);
    sensitivitySlider->setValue(static_cast<int>(m_sensitivity * 10));
    sensitivityControlLayout->addWidget(sensitivitySlider, 1);

    auto sensitivityMaxLabel = new QLabel("100");
    sensitivityControlLayout->addWidget(sensitivityMaxLabel);

    sensitivityLayout->addLayout(sensitivityControlLayout);

    auto sensitivityHint = new QLabel(
        "How responsive the filter is to signal changes. "
        "Higher values = more smoothing of steady signals, less smoothing of transients."
    );
    sensitivityHint->setStyleSheet("color: gray; font-size: 11px;");
    sensitivityHint->setWordWrap(true);
    sensitivityLayout->addWidget(sensitivityHint);

    mainLayout->addWidget(sensitivityGroup);

    // Base frequency parameter (Hz)
    auto basefreqGroup = new QGroupBox("Base Frequency (Hz)");
    auto basefreqLayout = new QVBoxLayout(basefreqGroup);

    basefreqSpinBox = new QDoubleSpinBox();
    basefreqSpinBox->setMinimum(2.0);
    basefreqSpinBox->setMaximum(48000.0);
    basefreqSpinBox->setValue(m_basefreq);
    basefreqSpinBox->setDecimals(0);
    basefreqSpinBox->setSingleStep(100.0);
    basefreqSpinBox->setSuffix(" Hz");

    auto basefreqControlLayout = new QHBoxLayout();
    basefreqControlLayout->addWidget(basefreqSpinBox);
    basefreqControlLayout->addStretch();
    
    auto basefreqMinLabel = new QLabel("2");
    basefreqControlLayout->addWidget(basefreqMinLabel);

    // Use logarithmic scale for frequency slider
    basefreqSlider = new QSlider(Qt::Horizontal);
    basefreqSlider->setMinimum(0);
    basefreqSlider->setMaximum(1000);
    // Convert frequency to log scale position
    double logPos = (std::log10(m_basefreq) - std::log10(2.0)) / (std::log10(48000.0) - std::log10(2.0)) * 1000;
    basefreqSlider->setValue(static_cast<int>(logPos));
    basefreqControlLayout->addWidget(basefreqSlider, 1);

    auto basefreqMaxLabel = new QLabel("48k");
    basefreqControlLayout->addWidget(basefreqMaxLabel);

    basefreqLayout->addLayout(basefreqControlLayout);

    auto basefreqHint = new QLabel(
        "Maximum cutoff frequency of the adaptive filter. "
        "Set to Nyquist (half sample rate) for transparent operation."
    );
    basefreqHint->setStyleSheet("color: gray; font-size: 11px;");
    basefreqHint->setWordWrap(true);
    basefreqLayout->addWidget(basefreqHint);

    mainLayout->addWidget(basefreqGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Sensitivity (linear 0-100 for slider, actual value 0-1000)
    connect(sensitivitySlider, &QSlider::valueChanged, [this](int value) {
        m_sensitivity = value / 10.0;
        sensitivitySpinBox->blockSignals(true);
        sensitivitySpinBox->setValue(m_sensitivity);
        sensitivitySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(sensitivitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_sensitivity = value;
        sensitivitySlider->blockSignals(true);
        sensitivitySlider->setValue(static_cast<int>(value * 10));
        sensitivitySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Base frequency (logarithmic slider)
    connect(basefreqSlider, &QSlider::valueChanged, [this](int value) {
        // Convert log scale position to frequency
        double logMin = std::log10(2.0);
        double logMax = std::log10(48000.0);
        double logFreq = logMin + (value / 1000.0) * (logMax - logMin);
        m_basefreq = std::pow(10.0, logFreq);
        
        basefreqSpinBox->blockSignals(true);
        basefreqSpinBox->setValue(m_basefreq);
        basefreqSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(basefreqSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_basefreq = value;
        
        // Convert frequency to log scale position
        double logMin = std::log10(2.0);
        double logMax = std::log10(48000.0);
        double logPos = (std::log10(value) - logMin) / (logMax - logMin) * 1000;
        
        basefreqSlider->blockSignals(true);
        basefreqSlider->setValue(static_cast<int>(logPos));
        basefreqSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdynamicsmooth::updateFFmpegFlags() {
    ffmpegFlags = QString("adynamicsmooth=sensitivity=%1:basefreq=%2")
                      .arg(m_sensitivity, 0, 'f', 2)
                      .arg(m_basefreq, 0, 'f', 0);
}

QString FFAdynamicsmooth::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdynamicsmooth::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adynamicsmooth";
    json["sensitivity"] = m_sensitivity;
    json["basefreq"] = m_basefreq;
}

void FFAdynamicsmooth::fromJSON(const QJsonObject& json) {
    m_sensitivity = json["sensitivity"].toDouble(2.0);
    m_basefreq = json["basefreq"].toDouble(22050.0);
    updateFFmpegFlags();
}
