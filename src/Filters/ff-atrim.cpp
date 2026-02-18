#include "ff-atrim.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>

FFAtrim::FFAtrim() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAtrim::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Trim"));

    // Start time
    auto startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel("Trim Start:"));
    
    startSpinBox = new QDoubleSpinBox();
    startSpinBox->setMinimum(0.0);
    startSpinBox->setMaximum(999999.0);
    startSpinBox->setValue(m_startValue);
    startSpinBox->setDecimals(3);
    startLayout->addWidget(startSpinBox);
    
    startUnitCombo = new QComboBox();
    startUnitCombo->addItem("seconds", "s");
    startUnitCombo->addItem("ms", "ms");
    startUnitCombo->addItem("samples", "samples");
    startUnitCombo->setCurrentIndex(0);
    startLayout->addWidget(startUnitCombo);
    startLayout->addStretch();
    
    layout->addLayout(startLayout);
    
    // Duration
    auto durLayout = new QHBoxLayout();
    durLayout->addWidget(new QLabel("Duration:  "));
    
    durationSpinBox = new QDoubleSpinBox();
    durationSpinBox->setMinimum(0.0);
    durationSpinBox->setMaximum(999999.0);
    durationSpinBox->setValue(m_durationValue);
    durationSpinBox->setDecimals(3);
    durLayout->addWidget(durationSpinBox);
    
    durationUnitCombo = new QComboBox();
    durationUnitCombo->addItem("seconds", "s");
    durationUnitCombo->addItem("ms", "ms");
    durationUnitCombo->addItem("samples", "samples");
    durationUnitCombo->setCurrentIndex(0);
    durLayout->addWidget(durationUnitCombo);
    durLayout->addStretch();
    
    layout->addLayout(durLayout);

    layout->addWidget(new QLabel("|····>|-------->|····|\n"
                                 "      S         D    "));    
     // End time
    auto endLayout = new QHBoxLayout();
    endLayout->addWidget(new QLabel("Trim End:  "));
    
    endSpinBox = new QDoubleSpinBox();
    endSpinBox->setMinimum(0.0);
    endSpinBox->setMaximum(999999.0);
    endSpinBox->setValue(m_endValue);
    endSpinBox->setDecimals(3);
    endLayout->addWidget(endSpinBox);
    
    endUnitCombo = new QComboBox();
    endUnitCombo->addItem("seconds", "s");
    endUnitCombo->addItem("ms", "ms");
    endUnitCombo->addItem("samples", "samples");
    endUnitCombo->setCurrentIndex(0);
    endLayout->addWidget(endUnitCombo);
    endLayout->addStretch();
    
    layout->addLayout(endLayout);

    layout->addWidget(new QLabel("|-------------->|····|\n" 
                                 "                E    "));
 
    // Connect signals
    connect(startSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_startValue = value;
        updateFFmpegFlags();
    });

    connect(startUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_startUnit = startUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(endSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_endValue = value;
        updateFFmpegFlags();
    });

    connect(endUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_endUnit = endUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(durationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_durationValue = value;
        updateFFmpegFlags();
    });

    connect(durationUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_durationUnit = durationUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

QString FFAtrim::formatDuration(double value, const QString& unit) const {
    if (unit == "ms") {
        return QString::number(value) + "ms";
    } else if (unit == "samples") {
        return QString::number(static_cast<int>(value)) + "s";
    } else {
        return QString::number(value);
    }
}

void FFAtrim::updateFFmpegFlags() {
    QStringList params;
    
    if (m_startValue != 0.0) {
        params << QString("start=%1").arg(formatDuration(m_startValue, m_startUnit));
    }
    
    if (m_endValue != 0.0) {
        params << QString("end=%1").arg(formatDuration(m_endValue, m_endUnit));
    }
    
    if (m_durationValue != 0.0) {
        params << QString("duration=%1").arg(formatDuration(m_durationValue, m_durationUnit));
    }
    
    if (params.isEmpty()) {
        ffmpegFlags = "atrim";
    } else {
        ffmpegFlags = "atrim=" + params.join(":");
    }
}

QString FFAtrim::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAtrim::toJSON(QJsonObject& json) const {
    json["type"] = "ff-atrim";
    json["start_value"] = m_startValue;
    json["start_unit"] = m_startUnit;
    json["end_value"] = m_endValue;
    json["end_unit"] = m_endUnit;
    json["duration_value"] = m_durationValue;
    json["duration_unit"] = m_durationUnit;
}

void FFAtrim::fromJSON(const QJsonObject& json) {
    m_startValue = json["start_value"].toDouble(0.0);
    m_startUnit = json["start_unit"].toString("s");
    m_endValue = json["end_value"].toDouble(0.0);
    m_endUnit = json["end_unit"].toString("s");
    m_durationValue = json["duration_value"].toDouble(0.0);
    m_durationUnit = json["duration_unit"].toString("s");
    updateFFmpegFlags();
}
