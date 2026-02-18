#include "ff-afade.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>

FFAfade::FFAfade() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAfade::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Fade"));

    // Fade type
    auto typeLayout = new QHBoxLayout();
    typeLayout->addWidget(new QLabel("Type:"));
    
    typeCombo = new QComboBox();
    typeCombo->addItem("Fade In", 0);
    typeCombo->addItem("Fade Out", 1);
    typeCombo->setCurrentIndex(m_type);
    typeLayout->addWidget(typeCombo);
    typeLayout->addStretch();
    
    layout->addLayout(typeLayout);

    // Start time
    auto startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel("Start Time:"));
    
    startTimeSpinBox = new QDoubleSpinBox();
    startTimeSpinBox->setMinimum(0.0);
    startTimeSpinBox->setMaximum(999999.0);
    startTimeSpinBox->setValue(m_startTimeValue);
    startTimeSpinBox->setDecimals(3);
    startLayout->addWidget(startTimeSpinBox);
    
    startTimeUnitCombo = new QComboBox();
    startTimeUnitCombo->addItem("seconds", "s");
    startTimeUnitCombo->addItem("ms", "ms");
    startTimeUnitCombo->addItem("samples", "samples");
    startTimeUnitCombo->setCurrentIndex(0);
    startLayout->addWidget(startTimeUnitCombo);
    startLayout->addStretch();
    
    layout->addLayout(startLayout);

    // Duration
    auto durLayout = new QHBoxLayout();
    durLayout->addWidget(new QLabel("Duration:"));
    
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

    // Curve
    auto curveLayout = new QHBoxLayout();
    curveLayout->addWidget(new QLabel("Curve:"));
    
    curveCombo = new QComboBox();
    curveCombo->addItem("No Fade", -1);
    curveCombo->addItem("Linear (tri)", 0);
    curveCombo->addItem("Quarter Sine (qsin)", 1);
    curveCombo->addItem("Exponential Sine (esin)", 2);
    curveCombo->addItem("Half Sine (hsin)", 3);
    curveCombo->addItem("Logarithmic (log)", 4);
    curveCombo->addItem("Inverted Parabola (ipar)", 5);
    curveCombo->addItem("Quadratic (qua)", 6);
    curveCombo->addItem("Cubic (cub)", 7);
    curveCombo->addItem("Square Root (squ)", 8);
    curveCombo->addItem("Cubic Root (cbr)", 9);
    curveCombo->addItem("Parabola (par)", 10);
    curveCombo->addItem("Exponential (exp)", 11);
    curveCombo->addItem("Inverted Quarter Sine (iqsin)", 12);
    curveCombo->addItem("Inverted Half Sine (ihsin)", 13);
    curveCombo->addItem("Double-Exponential Seat (dese)", 14);
    curveCombo->addItem("Double-Exponential Sigmoid (desi)", 15);
    curveCombo->addItem("Logistic Sigmoid (losi)", 16);
    curveCombo->addItem("Sine Cardinal (sinc)", 17);
    curveCombo->addItem("Inverted Sine Cardinal (isinc)", 18);
    curveCombo->addItem("Quartic (quat)", 19);
    curveCombo->addItem("Quartic Root (quatr)", 20);
    curveCombo->addItem("Squared Quarter Sine (qsin2)", 21);
    curveCombo->addItem("Squared Half Sine (hsin2)", 22);
    curveCombo->setCurrentIndex(1); // tri
    curveLayout->addWidget(curveCombo);
    curveLayout->addStretch();
    
    layout->addLayout(curveLayout);

    // Silence gain
    auto silenceLayout = new QHBoxLayout();
    silenceLayout->addWidget(new QLabel("Silence Gain:"));
    
    silenceSpinBox = new QDoubleSpinBox();
    silenceSpinBox->setMinimum(0.0);
    silenceSpinBox->setMaximum(1.0);
    silenceSpinBox->setValue(m_silence);
    silenceSpinBox->setDecimals(2);
    silenceSpinBox->setSingleStep(0.01);
    silenceLayout->addWidget(silenceSpinBox);
    silenceLayout->addStretch();
    
    layout->addLayout(silenceLayout);

    // Unity gain
    auto unityLayout = new QHBoxLayout();
    unityLayout->addWidget(new QLabel("Unity Gain:"));
    
    unitySpinBox = new QDoubleSpinBox();
    unitySpinBox->setMinimum(0.0);
    unitySpinBox->setMaximum(1.0);
    unitySpinBox->setValue(m_unity);
    unitySpinBox->setDecimals(2);
    unitySpinBox->setSingleStep(0.01);
    unityLayout->addWidget(unitySpinBox);
    unityLayout->addStretch();
    
    layout->addLayout(unityLayout);

    // Connect signals
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_type = typeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(startTimeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_startTimeValue = value;
        updateFFmpegFlags();
    });

    connect(startTimeUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_startTimeUnit = startTimeUnitCombo->currentData().toString();
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

    connect(curveCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_curve = curveCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(silenceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_silence = value;
        updateFFmpegFlags();
    });

    connect(unitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_unity = value;
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

QString FFAfade::formatDuration(double value, const QString& unit) const {
    if (unit == "ms") {
        return QString::number(value) + "ms";
    } else if (unit == "samples") {
        return QString::number(static_cast<int>(value)) + "s";
    } else {
        return QString::number(value);
    }
}

void FFAfade::updateFFmpegFlags() {
    QStringList params;
    
    params << QString("t=%1").arg(m_type == 0 ? "in" : "out");
    
    if (m_startTimeValue != 0.0) {
        params << QString("st=%1").arg(formatDuration(m_startTimeValue, m_startTimeUnit));
    }
    
    if (m_durationValue != 0.0) {
        params << QString("d=%1").arg(formatDuration(m_durationValue, m_durationUnit));
    }
    
    if (m_curve != 0) {
        params << QString("c=%1").arg(m_curve);
    }
    
    if (m_silence != 0.0) {
        params << QString("silence=%1").arg(m_silence);
    }
    
    if (m_unity != 1.0) {
        params << QString("unity=%1").arg(m_unity);
    }
    
    ffmpegFlags = "afade=" + params.join(":");
}

QString FFAfade::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAfade::toJSON(QJsonObject& json) const {
    json["type"] = "ff-afade";
    json["fade_type"] = m_type;
    json["start_time_value"] = m_startTimeValue;
    json["start_time_unit"] = m_startTimeUnit;
    json["duration_value"] = m_durationValue;
    json["duration_unit"] = m_durationUnit;
    json["curve"] = m_curve;
    json["silence"] = m_silence;
    json["unity"] = m_unity;
}

void FFAfade::fromJSON(const QJsonObject& json) {
    m_type = json["fade_type"].toInt(0);
    m_startTimeValue = json["start_time_value"].toDouble(0.0);
    m_startTimeUnit = json["start_time_unit"].toString("s");
    m_durationValue = json["duration_value"].toDouble(0.0);
    m_durationUnit = json["duration_unit"].toString("s");
    m_curve = json["curve"].toInt(0);
    m_silence = json["silence"].toDouble(0.0);
    m_unity = json["unity"].toDouble(1.0);
    updateFFmpegFlags();
}
