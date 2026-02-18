#include "ff-astats.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAstats::FFAstats() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAstats::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "Measures and reports audio statistics including:\n"
        "• Peak level, RMS level, crest factor\n"
        "• DC offset, dynamic range, entropy\n"
        "• Zero crossings, bit depth, noise floor\n\n"
        "Statistics are printed to stderr during processing."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Window length (0 - 10 seconds)
    auto lengthGroup = new QGroupBox("Analysis Window Length");
    auto lengthLayout = new QVBoxLayout(lengthGroup);

    lengthSpinBox = new QDoubleSpinBox();
    lengthSpinBox->setMinimum(0.001);
    lengthSpinBox->setMaximum(10.0);
    lengthSpinBox->setValue(m_length);
    lengthSpinBox->setDecimals(3);
    lengthSpinBox->setSingleStep(0.01);
    lengthSpinBox->setSuffix(" sec");

    auto lengthControlLayout = new QHBoxLayout();
    lengthControlLayout->addWidget(lengthSpinBox);
    lengthControlLayout->addStretch();
    lengthControlLayout->addWidget(new QLabel("0.001"));
    lengthSlider = new QSlider(Qt::Horizontal);
    lengthSlider->setMinimum(1);
    lengthSlider->setMaximum(10000);
    lengthSlider->setValue(static_cast<int>(m_length * 1000));
    lengthControlLayout->addWidget(lengthSlider, 1);
    lengthControlLayout->addWidget(new QLabel("10"));

    auto lengthHint = new QLabel("Time window for rolling statistics calculation");
    lengthHint->setStyleSheet("color: gray; font-size: 11px;");
    lengthLayout->addLayout(lengthControlLayout);
    lengthLayout->addWidget(lengthHint);
    mainLayout->addWidget(lengthGroup);

    // Metadata injection
    auto metadataGroup = new QGroupBox("Metadata Output");
    auto metadataLayout = new QVBoxLayout(metadataGroup);

    metadataCheck = new QCheckBox("Inject metadata into filtergraph");
    metadataCheck->setChecked(m_metadata);
    auto metadataHint = new QLabel("Makes statistics available to other filters via metadata");
    metadataHint->setStyleSheet("color: gray; font-size: 11px;");
    metadataLayout->addWidget(metadataCheck);
    metadataLayout->addWidget(metadataHint);
    mainLayout->addWidget(metadataGroup);

    // Reset interval
    auto resetGroup = new QGroupBox("Statistics Reset");
    auto resetLayout = new QVBoxLayout(resetGroup);

    resetSpinBox = new QSpinBox();
    resetSpinBox->setMinimum(0);
    resetSpinBox->setMaximum(100000);
    resetSpinBox->setValue(m_reset);
    resetSpinBox->setSpecialValueText("Never reset");
    resetSpinBox->setSuffix(" frames");

    auto resetHint = new QLabel("Number of frames before resetting cumulative stats (0 = never)");
    resetHint->setStyleSheet("color: gray; font-size: 11px;");
    resetLayout->addWidget(resetSpinBox);
    resetLayout->addWidget(resetHint);
    mainLayout->addWidget(resetGroup);

    // Measured parameters info
    auto measuresGroup = new QGroupBox("Measured Parameters");
    auto measuresLayout = new QVBoxLayout(measuresGroup);
    
    auto measuresLabel = new QLabel(
        "Per-channel: DC offset, Min/Max level, Peak level, RMS level,\n"
        "Crest factor, Flat factor, Peak count, Noise floor, Entropy,\n"
        "Bit depth, Dynamic range, Zero crossings\n\n"
        "Overall: Number of samples, Number of NaN/Inf/denormals"
    );
    measuresLabel->setStyleSheet("color: #666; font-size: 10px;");
    measuresLayout->addWidget(measuresLabel);
    mainLayout->addWidget(measuresGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Length
    connect(lengthSlider, &QSlider::valueChanged, [this](int value) {
        m_length = value / 1000.0;
        lengthSpinBox->blockSignals(true);
        lengthSpinBox->setValue(m_length);
        lengthSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(lengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_length = value;
        lengthSlider->blockSignals(true);
        lengthSlider->setValue(static_cast<int>(value * 1000));
        lengthSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Metadata
    connect(metadataCheck, &QCheckBox::toggled, [this](bool checked) {
        m_metadata = checked;
        updateFFmpegFlags();
    });

    // Reset
    connect(resetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_reset = value;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAstats::updateFFmpegFlags() {
    ffmpegFlags = QString("astats=length=%1:metadata=%2:reset=%3")
                      .arg(m_length, 0, 'f', 3)
                      .arg(m_metadata ? "1" : "0")
                      .arg(m_reset);
}

QString FFAstats::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAstats::toJSON(QJsonObject& json) const {
    json["type"] = "ff-astats";
    json["length"] = m_length;
    json["metadata"] = m_metadata;
    json["reset"] = m_reset;
}

void FFAstats::fromJSON(const QJsonObject& json) {
    m_length = json["length"].toDouble(0.05);
    m_metadata = json["metadata"].toBool(false);
    m_reset = json["reset"].toInt(0);
    updateFFmpegFlags();
}
