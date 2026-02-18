#include "ff-chorus.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFChorus::FFChorus() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFChorus::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Input Gain (-96 to 0 dB)
    auto inGainGroup = new QGroupBox("Input Gain (dB)");
    auto inGainLayout = new QVBoxLayout(inGainGroup);

    inGainSpinBox = new QDoubleSpinBox();
    inGainSpinBox->setMinimum(-96.0);
    inGainSpinBox->setMaximum(0.0);
    inGainSpinBox->setValue(linearToDb(m_inGain));
    inGainSpinBox->setDecimals(1);
    inGainSpinBox->setSingleStep(0.1);
    inGainSpinBox->setSuffix(" dB");

    auto inGainControlLayout = new QHBoxLayout();
    inGainControlLayout->addWidget(inGainSpinBox);
    inGainControlLayout->addStretch();

    auto inGainMinLabel = new QLabel("-96");
    inGainControlLayout->addWidget(inGainMinLabel);

    inGainSlider = new QSlider(Qt::Horizontal);
    inGainSlider->setMinimum(0);
    inGainSlider->setMaximum(960);
    inGainSlider->setValue(static_cast<int>((linearToDb(m_inGain) + 96.0) * 10.0));
    inGainControlLayout->addWidget(inGainSlider, 1);

    auto inGainMaxLabel = new QLabel("0");
    inGainControlLayout->addWidget(inGainMaxLabel);

    inGainLayout->addLayout(inGainControlLayout);
    mainLayout->addWidget(inGainGroup);

    // Output Gain (-96 to 0 dB)
    auto outGainGroup = new QGroupBox("Output Gain (dB)");
    auto outGainLayout = new QVBoxLayout(outGainGroup);

    outGainSpinBox = new QDoubleSpinBox();
    outGainSpinBox->setMinimum(-96.0);
    outGainSpinBox->setMaximum(0.0);
    outGainSpinBox->setValue(linearToDb(m_outGain));
    outGainSpinBox->setDecimals(1);
    outGainSpinBox->setSingleStep(0.1);
    outGainSpinBox->setSuffix(" dB");

    auto outGainControlLayout = new QHBoxLayout();
    outGainControlLayout->addWidget(outGainSpinBox);
    outGainControlLayout->addStretch();

    auto outGainMinLabel = new QLabel("-96");
    outGainControlLayout->addWidget(outGainMinLabel);

    outGainSlider = new QSlider(Qt::Horizontal);
    outGainSlider->setMinimum(0);
    outGainSlider->setMaximum(960);
    outGainSlider->setValue(static_cast<int>((linearToDb(m_outGain) + 96.0) * 10.0));
    outGainControlLayout->addWidget(outGainSlider, 1);

    auto outGainMaxLabel = new QLabel("0");
    outGainControlLayout->addWidget(outGainMaxLabel);

    outGainLayout->addLayout(outGainControlLayout);
    mainLayout->addWidget(outGainGroup);

    // Delays (string - pipe-separated milliseconds)
    auto delaysGroup = new QGroupBox("Delays (ms)");
    auto delaysLayout = new QVBoxLayout(delaysGroup);
    
    auto delaysHint = new QLabel("Pipe-separated delay values in milliseconds:");
    delaysHint->setWordWrap(true);
    delaysLayout->addWidget(delaysHint);

    delaysEdit = new QLineEdit();
    delaysEdit->setText(m_delays);
    delaysEdit->setPlaceholderText("");
    
    connect(delaysEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_delays = text;
        updateFFmpegFlags();
    });

    delaysLayout->addWidget(delaysEdit);
    mainLayout->addWidget(delaysGroup);

    // Decays (string - pipe-separated decay values)
    auto decaysGroup = new QGroupBox("Decays");
    auto decaysLayout = new QVBoxLayout(decaysGroup);

    auto decaysHint = new QLabel("Pipe-separated decay values:");
    decaysHint->setWordWrap(true);
    decaysLayout->addWidget(decaysHint);

    decaysEdit = new QLineEdit();
    decaysEdit->setText(m_decays);
    decaysEdit->setPlaceholderText("");
    
    connect(decaysEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_decays = text;
        updateFFmpegFlags();
    });

    decaysLayout->addWidget(decaysEdit);
    mainLayout->addWidget(decaysGroup);

    // Speeds (string - pipe-separated Hz values)
    auto speedsGroup = new QGroupBox("Speeds (Hz)");
    auto speedsLayout = new QVBoxLayout(speedsGroup);

    auto speedsHint = new QLabel("Pipe-separated modulation speeds in Hz:");
    speedsHint->setWordWrap(true);
    speedsLayout->addWidget(speedsHint);

    speedsEdit = new QLineEdit();
    speedsEdit->setText(m_speeds);
    speedsEdit->setPlaceholderText("");
    
    connect(speedsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_speeds = text;
        updateFFmpegFlags();
    });

    speedsLayout->addWidget(speedsEdit);
    mainLayout->addWidget(speedsGroup);

    // Depths (string - pipe-separated depth values)
    auto depthsGroup = new QGroupBox("Depths");
    auto depthsLayout = new QVBoxLayout(depthsGroup);

    auto depthsHint = new QLabel("Pipe-separated modulation depths:");
    depthsHint->setWordWrap(true);
    depthsLayout->addWidget(depthsHint);

    depthsEdit = new QLineEdit();
    depthsEdit->setText(m_depths);
    depthsEdit->setPlaceholderText("");
    
    connect(depthsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_depths = text;
        updateFFmpegFlags();
    });

    depthsLayout->addWidget(depthsEdit);
    mainLayout->addWidget(depthsGroup);

    // Connect gain sliders (dB conversion)
    connect(inGainSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_inGain = dbToLinear(db);
        inGainSpinBox->blockSignals(true);
        inGainSpinBox->setValue(db);
        inGainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(inGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_inGain = dbToLinear(db);
        inGainSlider->blockSignals(true);
        inGainSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        inGainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(outGainSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_outGain = dbToLinear(db);
        outGainSpinBox->blockSignals(true);
        outGainSpinBox->setValue(db);
        outGainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(outGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_outGain = dbToLinear(db);
        outGainSlider->blockSignals(true);
        outGainSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        outGainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

void FFChorus::updateFFmpegFlags() {
    QStringList parts;
    parts << QString("in_gain=%1").arg(m_inGain, 0, 'f', 2);
    parts << QString("out_gain=%1").arg(m_outGain, 0, 'f', 2);
    
    if (!m_delays.isEmpty()) {
        parts << QString("delays=%1").arg(m_delays);
    }
    if (!m_decays.isEmpty()) {
        parts << QString("decays=%1").arg(m_decays);
    }
    if (!m_speeds.isEmpty()) {
        parts << QString("speeds=%1").arg(m_speeds);
    }
    if (!m_depths.isEmpty()) {
        parts << QString("depths=%1").arg(m_depths);
    }
    
    ffmpegFlags = "chorus=" + parts.join(":");
}

QString FFChorus::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFChorus::toJSON(QJsonObject& json) const {
    json["type"] = "ff-chorus";
    json["in_gain"] = m_inGain;
    json["out_gain"] = m_outGain;
    json["delays"] = m_delays;
    json["decays"] = m_decays;
    json["speeds"] = m_speeds;
    json["depths"] = m_depths;
}

void FFChorus::fromJSON(const QJsonObject& json) {
    m_inGain = json["in_gain"].toDouble(0.4);
    m_outGain = json["out_gain"].toDouble(0.4);
    m_delays = json["delays"].toString("");
    m_decays = json["decays"].toString("");
    m_speeds = json["speeds"].toString("");
    m_depths = json["depths"].toString("");
    updateFFmpegFlags();
}
