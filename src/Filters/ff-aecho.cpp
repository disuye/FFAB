#include "ff-aecho.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <cmath>

FFAecho::FFAecho() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAecho::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    layout->addWidget(new QLabel("Echo"));

    // Input gain (-96 to 0 dB)
    auto inGainLayout = new QHBoxLayout();
    inGainLayout->addWidget(new QLabel("Input Gain (dB):"));

    inGainSlider = new QSlider(Qt::Horizontal);
    inGainSlider->setMinimum(0);
    inGainSlider->setMaximum(960);
    inGainSlider->setValue(static_cast<int>((linearToDb(m_inGain) + 96.0) * 10.0));
    inGainLayout->addWidget(inGainSlider, 1);

    inGainSpinBox = new QDoubleSpinBox();
    inGainSpinBox->setMinimum(-96.0);
    inGainSpinBox->setMaximum(0.0);
    inGainSpinBox->setValue(linearToDb(m_inGain));
    inGainSpinBox->setSingleStep(0.1);
    inGainSpinBox->setDecimals(1);
    inGainSpinBox->setSuffix(" dB");
    inGainLayout->addWidget(inGainSpinBox);

    layout->addLayout(inGainLayout);

    // Output gain (-96 to 0 dB)
    auto outGainLayout = new QHBoxLayout();
    outGainLayout->addWidget(new QLabel("Output Gain (dB):"));

    outGainSlider = new QSlider(Qt::Horizontal);
    outGainSlider->setMinimum(0);
    outGainSlider->setMaximum(960);
    outGainSlider->setValue(static_cast<int>((linearToDb(m_outGain) + 96.0) * 10.0));
    outGainLayout->addWidget(outGainSlider, 1);

    outGainSpinBox = new QDoubleSpinBox();
    outGainSpinBox->setMinimum(-96.0);
    outGainSpinBox->setMaximum(0.0);
    outGainSpinBox->setValue(linearToDb(m_outGain));
    outGainSpinBox->setSingleStep(0.1);
    outGainSpinBox->setDecimals(1);
    outGainSpinBox->setSuffix(" dB");
    outGainLayout->addWidget(outGainSpinBox);
    
    layout->addLayout(outGainLayout);

    // Delays string
    auto delaysLayout = new QHBoxLayout();
    delaysLayout->addWidget(new QLabel("Delays (ms):"));
    
    delaysEdit = new QLineEdit();
    delaysEdit->setText(m_delays);
    delaysEdit->setPlaceholderText("1000|1800");
    delaysLayout->addWidget(delaysEdit, 1);
    
    layout->addLayout(delaysLayout);

    // Decays string
    auto decaysLayout = new QHBoxLayout();
    decaysLayout->addWidget(new QLabel("Decays:"));
    
    decaysEdit = new QLineEdit();
    decaysEdit->setText(m_decays);
    decaysEdit->setPlaceholderText("0.5|0.3");
    decaysLayout->addWidget(decaysEdit, 1);
    
    layout->addLayout(decaysLayout);

    // Connect signals (dB conversion)
    connect(inGainSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_inGain = dbToLinear(db);
        updateFFmpegFlags();

        if (inGainSpinBox) {
            inGainSpinBox->blockSignals(true);
            inGainSpinBox->setValue(db);
            inGainSpinBox->blockSignals(false);
        }
    });

    connect(inGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_inGain = dbToLinear(db);
        updateFFmpegFlags();

        if (inGainSlider) {
            inGainSlider->blockSignals(true);
            inGainSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
            inGainSlider->blockSignals(false);
        }
    });

    connect(outGainSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_outGain = dbToLinear(db);
        updateFFmpegFlags();

        if (outGainSpinBox) {
            outGainSpinBox->blockSignals(true);
            outGainSpinBox->setValue(db);
            outGainSpinBox->blockSignals(false);
        }
    });

    connect(outGainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_outGain = dbToLinear(db);
        updateFFmpegFlags();

        if (outGainSlider) {
            outGainSlider->blockSignals(true);
            outGainSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
            outGainSlider->blockSignals(false);
        }
    });

    connect(delaysEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_delays = text;
        updateFFmpegFlags();
    });

    connect(decaysEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_decays = text;
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

void FFAecho::updateFFmpegFlags() {
    ffmpegFlags = QString("aecho=%1:%2:%3:%4")
        .arg(m_inGain)
        .arg(m_outGain)
        .arg(m_delays)
        .arg(m_decays);
}

QString FFAecho::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAecho::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aecho";
    json["in_gain"] = m_inGain;
    json["out_gain"] = m_outGain;
    json["delays"] = m_delays;
    json["decays"] = m_decays;
}

void FFAecho::fromJSON(const QJsonObject& json) {
    m_inGain = json["in_gain"].toDouble(0.6);
    m_outGain = json["out_gain"].toDouble(0.3);
    m_delays = json["delays"].toString("1000|1800");
    m_decays = json["decays"].toString("0.5|0.3");
    updateFFmpegFlags();
}
