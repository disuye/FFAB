#include "ff-silencedetect.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFSilencedetect::FFSilencedetect() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFSilencedetect::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // dB conversion helpers
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -120.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Info group
    auto infoGroup = new QGroupBox("Silence Detection");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel("Detects silent sections in the audio stream.");
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto passLabel = new QLabel("Note: Audio passes through unchanged");
    infoLayout->addWidget(passLabel);

    mainLayout->addWidget(infoGroup);

    // Noise threshold parameter (displayed in dB)
    auto noiseGroup = new QGroupBox("Noise Threshold (dB)");
    auto noiseLayout = new QVBoxLayout(noiseGroup);

    double noiseDb = linearToDb(m_noise);
    noiseSpinBox = new QDoubleSpinBox();
    noiseSpinBox->setMinimum(-90.0);
    noiseSpinBox->setMaximum(0.0);
    noiseSpinBox->setValue(noiseDb);
    noiseSpinBox->setDecimals(1);
    noiseSpinBox->setSingleStep(1.0);
    noiseSpinBox->setSuffix(" dB");

    auto noiseControlLayout = new QHBoxLayout();
    noiseControlLayout->addWidget(noiseSpinBox);
    noiseControlLayout->addStretch();
    
    auto noiseMinLabel = new QLabel("-90");
    noiseControlLayout->addWidget(noiseMinLabel);

    noiseSlider = new QSlider(Qt::Horizontal);
    noiseSlider->setMinimum(-900);
    noiseSlider->setMaximum(0);
    noiseSlider->setValue(static_cast<int>(noiseDb * 10));
    noiseControlLayout->addWidget(noiseSlider, 1);

    auto noiseMaxLabel = new QLabel("0");
    noiseControlLayout->addWidget(noiseMaxLabel);

    noiseLayout->addLayout(noiseControlLayout);

    auto noiseHint = new QLabel("Audio below this level is considered silence. -60 dB is a common starting point.");
    noiseHint->setStyleSheet("color: gray; font-size: 11px;");
    noiseHint->setWordWrap(true);
    noiseLayout->addWidget(noiseHint);

    mainLayout->addWidget(noiseGroup);

    // Duration parameter (seconds)
    auto durationGroup = new QGroupBox("Minimum Duration (seconds)");
    auto durationLayout = new QVBoxLayout(durationGroup);

    durationSpinBox = new QDoubleSpinBox();
    durationSpinBox->setMinimum(0.0);
    durationSpinBox->setMaximum(60.0);
    durationSpinBox->setValue(m_duration);
    durationSpinBox->setDecimals(2);
    durationSpinBox->setSingleStep(0.1);
    durationSpinBox->setSuffix(" s");

    auto durationControlLayout = new QHBoxLayout();
    durationControlLayout->addWidget(durationSpinBox);
    durationControlLayout->addStretch();
    
    auto durationMinLabel = new QLabel("0");
    durationControlLayout->addWidget(durationMinLabel);

    durationSlider = new QSlider(Qt::Horizontal);
    durationSlider->setMinimum(0);
    durationSlider->setMaximum(6000);
    durationSlider->setValue(static_cast<int>(m_duration * 100));
    durationControlLayout->addWidget(durationSlider, 1);

    auto durationMaxLabel = new QLabel("60");
    durationControlLayout->addWidget(durationMaxLabel);

    durationLayout->addLayout(durationControlLayout);

    auto durationHint = new QLabel("Only report silence longer than this duration.");
    durationHint->setStyleSheet("color: gray; font-size: 11px;");
    durationLayout->addWidget(durationHint);

    mainLayout->addWidget(durationGroup);

    // Mono option
    auto monoGroup = new QGroupBox("Channel Analysis");
    auto monoLayout = new QVBoxLayout(monoGroup);

    monoCheck = new QCheckBox("Analyze channels separately");
    monoCheck->setChecked(m_mono);
    monoLayout->addWidget(monoCheck);

    auto monoHint = new QLabel("When enabled, each channel is checked independently for silence.");
    monoHint->setStyleSheet("color: gray; font-size: 11px;");
    monoHint->setWordWrap(true);
    monoLayout->addWidget(monoHint);

    mainLayout->addWidget(monoGroup);

    // Output info group
    auto outputGroup = new QGroupBox("Output");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "Results are output to FFmpeg's stderr:\n\n"
        "• silence_start: Timestamp where silence begins\n"
        "• silence_end: Timestamp where silence ends\n"
        "• silence_duration: Length of silent section"
    );
    outputLabel->setWordWrap(true);
    outputLayout->addWidget(outputLabel);

    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Noise threshold (dB to linear conversion)
    connect(noiseSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_noise = dbToLinear(db);
        noiseSpinBox->blockSignals(true);
        noiseSpinBox->setValue(db);
        noiseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(noiseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_noise = dbToLinear(db);
        noiseSlider->blockSignals(true);
        noiseSlider->setValue(static_cast<int>(db * 10));
        noiseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Duration
    connect(durationSlider, &QSlider::valueChanged, [this](int value) {
        m_duration = value / 100.0;
        durationSpinBox->blockSignals(true);
        durationSpinBox->setValue(m_duration);
        durationSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(durationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_duration = value;
        durationSlider->blockSignals(true);
        durationSlider->setValue(static_cast<int>(value * 100));
        durationSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Mono
    connect(monoCheck, &QCheckBox::toggled, [this](bool checked) {
        m_mono = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFSilencedetect::updateFFmpegFlags() {
    ffmpegFlags = QString("silencedetect=n=%1:d=%2:mono=%3")
                      .arg(m_noise, 0, 'g', 6)
                      .arg(m_duration, 0, 'f', 2)
                      .arg(m_mono ? "1" : "0");
}

QString FFSilencedetect::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFSilencedetect::toJSON(QJsonObject& json) const {
    json["type"] = "ff-silencedetect";
    json["noise"] = m_noise;
    json["duration"] = m_duration;
    json["mono"] = m_mono;
}

void FFSilencedetect::fromJSON(const QJsonObject& json) {
    m_noise = json["noise"].toDouble(0.001);
    m_duration = json["duration"].toDouble(2.0);
    m_mono = json["mono"].toBool(false);
    updateFFmpegFlags();
}
