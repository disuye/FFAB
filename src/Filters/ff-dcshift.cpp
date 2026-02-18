#include "ff-dcshift.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>

FFDcshift::FFDcshift() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFDcshift::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("DC Shift"));

    // Shift parameter
    auto shiftLayout = new QHBoxLayout();
    shiftLayout->addWidget(new QLabel("Shift:"));
    
    shiftSlider = new QSlider(Qt::Horizontal);
    shiftSlider->setMinimum(-100);
    shiftSlider->setMaximum(100);
    shiftSlider->setValue(static_cast<int>(m_shift * 100));
    shiftLayout->addWidget(shiftSlider, 1);
    
    shiftSpinBox = new QDoubleSpinBox();
    shiftSpinBox->setMinimum(-1.0);
    shiftSpinBox->setMaximum(1.0);
    shiftSpinBox->setValue(m_shift);
    shiftSpinBox->setSingleStep(0.01);
    shiftSpinBox->setDecimals(2);
    shiftLayout->addWidget(shiftSpinBox);
    
    layout->addLayout(shiftLayout);

    // Limiter gain parameter
    auto limiterLayout = new QHBoxLayout();
    limiterLayout->addWidget(new QLabel("Limiter Gain:"));
    
    limiterSlider = new QSlider(Qt::Horizontal);
    limiterSlider->setMinimum(0);
    limiterSlider->setMaximum(100);
    limiterSlider->setValue(static_cast<int>(m_limiterGain * 100));
    limiterLayout->addWidget(limiterSlider, 1);
    
    limiterSpinBox = new QDoubleSpinBox();
    limiterSpinBox->setMinimum(0.0);
    limiterSpinBox->setMaximum(1.0);
    limiterSpinBox->setValue(m_limiterGain);
    limiterSpinBox->setSingleStep(0.01);
    limiterSpinBox->setDecimals(2);
    limiterLayout->addWidget(limiterSpinBox);
    
    layout->addLayout(limiterLayout);

    // Connect shift signals
    connect(shiftSlider, &QSlider::valueChanged, [this](int value) {
        m_shift = value / 100.0;
        updateFFmpegFlags();
        
        if (shiftSpinBox) {
            shiftSpinBox->blockSignals(true);
            shiftSpinBox->setValue(m_shift);
            shiftSpinBox->blockSignals(false);
        }
    });

    connect(shiftSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_shift = value;
        updateFFmpegFlags();
        
        if (shiftSlider) {
            shiftSlider->blockSignals(true);
            shiftSlider->setValue(static_cast<int>(m_shift * 100));
            shiftSlider->blockSignals(false);
        }
    });

    // Connect limiter signals
    connect(limiterSlider, &QSlider::valueChanged, [this](int value) {
        m_limiterGain = value / 100.0;
        updateFFmpegFlags();
        
        if (limiterSpinBox) {
            limiterSpinBox->blockSignals(true);
            limiterSpinBox->setValue(m_limiterGain);
            limiterSpinBox->blockSignals(false);
        }
    });

    connect(limiterSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_limiterGain = value;
        updateFFmpegFlags();
        
        if (limiterSlider) {
            limiterSlider->blockSignals(true);
            limiterSlider->setValue(static_cast<int>(m_limiterGain * 100));
            limiterSlider->blockSignals(false);
        }
    });

    layout->addStretch();
    return parametersWidget;
}

void FFDcshift::updateFFmpegFlags() {
    QStringList params;
    
    if (m_shift != 0.0) {
        params << QString("shift=%1").arg(m_shift);
    }
    
    if (m_limiterGain != 0.0) {
        params << QString("limitergain=%1").arg(m_limiterGain);
    }
    
    if (params.isEmpty()) {
        ffmpegFlags = "dcshift";
    } else {
        ffmpegFlags = "dcshift=" + params.join(":");
    }
}

QString FFDcshift::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFDcshift::toJSON(QJsonObject& json) const {
    json["type"] = "ff-dcshift";
    json["shift"] = m_shift;
    json["limiter_gain"] = m_limiterGain;
}

void FFDcshift::fromJSON(const QJsonObject& json) {
    m_shift = json["shift"].toDouble(0.0);
    m_limiterGain = json["limiter_gain"].toDouble(0.0);
    updateFFmpegFlags();
}
