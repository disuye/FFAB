#include "ff-atempo.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>

FFAtempo::FFAtempo() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAtempo::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Tempo"));

    // Slider with endpoint labels (0.5x to 2.0x common range)
    auto sliderLayout = new QHBoxLayout();
    
    auto slowerLabel = new QLabel("0.5x");
    sliderLayout->addWidget(slowerLabel);

    tempoSlider = new QSlider(Qt::Horizontal);
    tempoSlider->setMinimum(50);   // 0.5x * 100
    tempoSlider->setMaximum(200);  // 2.0x * 100
    tempoSlider->setValue(static_cast<int>(m_tempo * 100));
    sliderLayout->addWidget(tempoSlider, 1);

    auto fasterLabel = new QLabel("2.0x");
    sliderLayout->addWidget(fasterLabel);

    layout->addLayout(sliderLayout);

    // Value display and precise control
    auto valueLayout = new QHBoxLayout();
    
    tempoLabel = new QLabel(QString("%1x").arg(m_tempo, 0, 'f', 2));
    valueLayout->addWidget(tempoLabel);
    
    valueLayout->addStretch();
    
    auto preciseLabel = new QLabel("Precise:");
    valueLayout->addWidget(preciseLabel);
    
    tempoSpinBox = new QDoubleSpinBox();
    tempoSpinBox->setMinimum(0.5);
    tempoSpinBox->setMaximum(100.0);
    tempoSpinBox->setValue(m_tempo);
    tempoSpinBox->setSingleStep(0.01);
    tempoSpinBox->setDecimals(2);
    tempoSpinBox->setSuffix("x");
    valueLayout->addWidget(tempoSpinBox);
    
    layout->addLayout(valueLayout);

    // Connect slider
    connect(tempoSlider, &QSlider::valueChanged, [this](int value) {
        m_tempo = value / 100.0;
        updateFFmpegFlags();
        
        if (tempoSpinBox) {
            tempoSpinBox->blockSignals(true);
            tempoSpinBox->setValue(m_tempo);
            tempoSpinBox->blockSignals(false);
        }
        if (tempoLabel) {
            tempoLabel->setText(QString("%1x").arg(m_tempo, 0, 'f', 2));
        }
    });

    // Connect spinbox
    connect(tempoSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_tempo = value;
        updateFFmpegFlags();
        
        if (value >= 0.5 && value <= 2.0) {
            tempoSlider->blockSignals(true);
            tempoSlider->setValue(static_cast<int>(m_tempo * 100));
            tempoSlider->blockSignals(false);
        }
        
        if (tempoLabel) {
            tempoLabel->setText(QString("%1x").arg(m_tempo, 0, 'f', 2));
        }
    });

    layout->addStretch();
    return parametersWidget;
}

void FFAtempo::updateFFmpegFlags() {
    if (m_tempo != 1.0) {
        ffmpegFlags = QString("atempo=%1").arg(m_tempo);
    } else {
        ffmpegFlags = "atempo=1.0";
    }
}

QString FFAtempo::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAtempo::toJSON(QJsonObject& json) const {
    json["type"] = "ff-atempo";
    json["tempo"] = m_tempo;
}

void FFAtempo::fromJSON(const QJsonObject& json) {
    m_tempo = json["tempo"].toDouble(1.0);
    updateFFmpegFlags();
}
