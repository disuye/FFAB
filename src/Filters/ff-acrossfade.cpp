#include "ff-acrossfade.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>

FFAcrossfade::FFAcrossfade() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcrossfade::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Crossfade"));
    layout->addWidget(new QLabel("Note: Requires two input streams"));

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

    // Overlap checkbox
    overlapCheckBox = new QCheckBox("Overlap 1st stream end with 2nd stream start");
    overlapCheckBox->setChecked(m_overlap);
    layout->addWidget(overlapCheckBox);

    // Curve 1
    auto curve1Layout = new QHBoxLayout();
    curve1Layout->addWidget(new QLabel("Curve 1:"));
    
    curve1Combo = new QComboBox();
    curve1Combo->addItem("No Fade", -1);
    curve1Combo->addItem("Linear (tri)", 0);
    curve1Combo->addItem("Quarter Sine (qsin)", 1);
    curve1Combo->addItem("Exponential Sine (esin)", 2);
    curve1Combo->addItem("Half Sine (hsin)", 3);
    curve1Combo->addItem("Logarithmic (log)", 4);
    curve1Combo->addItem("Inverted Parabola (ipar)", 5);
    curve1Combo->addItem("Quadratic (qua)", 6);
    curve1Combo->addItem("Cubic (cub)", 7);
    curve1Combo->addItem("Square Root (squ)", 8);
    curve1Combo->addItem("Cubic Root (cbr)", 9);
    curve1Combo->addItem("Parabola (par)", 10);
    curve1Combo->addItem("Exponential (exp)", 11);
    curve1Combo->addItem("Inverted Quarter Sine (iqsin)", 12);
    curve1Combo->addItem("Inverted Half Sine (ihsin)", 13);
    curve1Combo->addItem("Double-Exponential Seat (dese)", 14);
    curve1Combo->addItem("Double-Exponential Sigmoid (desi)", 15);
    curve1Combo->addItem("Logistic Sigmoid (losi)", 16);
    curve1Combo->addItem("Sine Cardinal (sinc)", 17);
    curve1Combo->addItem("Inverted Sine Cardinal (isinc)", 18);
    curve1Combo->addItem("Quartic (quat)", 19);
    curve1Combo->addItem("Quartic Root (quatr)", 20);
    curve1Combo->addItem("Squared Quarter Sine (qsin2)", 21);
    curve1Combo->addItem("Squared Half Sine (hsin2)", 22);
    curve1Combo->setCurrentIndex(1); // tri
    curve1Layout->addWidget(curve1Combo);
    curve1Layout->addStretch();
    
    layout->addLayout(curve1Layout);

    // Curve 2
    auto curve2Layout = new QHBoxLayout();
    curve2Layout->addWidget(new QLabel("Curve 2:"));
    
    curve2Combo = new QComboBox();
    curve2Combo->addItem("No Fade", -1);
    curve2Combo->addItem("Linear (tri)", 0);
    curve2Combo->addItem("Quarter Sine (qsin)", 1);
    curve2Combo->addItem("Exponential Sine (esin)", 2);
    curve2Combo->addItem("Half Sine (hsin)", 3);
    curve2Combo->addItem("Logarithmic (log)", 4);
    curve2Combo->addItem("Inverted Parabola (ipar)", 5);
    curve2Combo->addItem("Quadratic (qua)", 6);
    curve2Combo->addItem("Cubic (cub)", 7);
    curve2Combo->addItem("Square Root (squ)", 8);
    curve2Combo->addItem("Cubic Root (cbr)", 9);
    curve2Combo->addItem("Parabola (par)", 10);
    curve2Combo->addItem("Exponential (exp)", 11);
    curve2Combo->addItem("Inverted Quarter Sine (iqsin)", 12);
    curve2Combo->addItem("Inverted Half Sine (ihsin)", 13);
    curve2Combo->addItem("Double-Exponential Seat (dese)", 14);
    curve2Combo->addItem("Double-Exponential Sigmoid (desi)", 15);
    curve2Combo->addItem("Logistic Sigmoid (losi)", 16);
    curve2Combo->addItem("Sine Cardinal (sinc)", 17);
    curve2Combo->addItem("Inverted Sine Cardinal (isinc)", 18);
    curve2Combo->addItem("Quartic (quat)", 19);
    curve2Combo->addItem("Quartic Root (quatr)", 20);
    curve2Combo->addItem("Squared Quarter Sine (qsin2)", 21);
    curve2Combo->addItem("Squared Half Sine (hsin2)", 22);
    curve2Combo->setCurrentIndex(1); // tri
    curve2Layout->addWidget(curve2Combo);
    curve2Layout->addStretch();
    
    layout->addLayout(curve2Layout);

    // Connect signals
    connect(durationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_durationValue = value;
        updateFFmpegFlags();
    });

    connect(durationUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_durationUnit = durationUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(overlapCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_overlap = checked;
        updateFFmpegFlags();
    });

    connect(curve1Combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_curve1 = curve1Combo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(curve2Combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_curve2 = curve2Combo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

QString FFAcrossfade::formatDuration(double value, const QString& unit) const {
    if (unit == "ms") {
        return QString::number(value) + "ms";
    } else if (unit == "samples") {
        return QString::number(static_cast<int>(value)) + "s";
    } else {
        return QString::number(value);
    }
}

void FFAcrossfade::updateFFmpegFlags() {
    QStringList params;
    
    if (m_durationValue != 0.0) {
        params << QString("d=%1").arg(formatDuration(m_durationValue, m_durationUnit));
    }
    
    if (!m_overlap) {
        params << "o=0";
    }
    
    if (m_curve1 != 0) {
        params << QString("c1=%1").arg(m_curve1);
    }
    
    if (m_curve2 != 0) {
        params << QString("c2=%1").arg(m_curve2);
    }
    
    int scIdx = (m_sidechainInputIndex >= 0) ? m_sidechainInputIndex : 1;
    QString inputLabels = QString("[0:a][%1:a]").arg(scIdx);

    if (params.isEmpty()) {
        ffmpegFlags = inputLabels + "acrossfade";
    } else {
        ffmpegFlags = inputLabels + "acrossfade=" + params.join(":");
    }
}

QString FFAcrossfade::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcrossfade::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acrossfade";
    json["duration_value"] = m_durationValue;
    json["duration_unit"] = m_durationUnit;
    json["overlap"] = m_overlap;
    json["curve1"] = m_curve1;
    json["curve2"] = m_curve2;
    json["sidechain_input_index"] = m_sidechainInputIndex;
}

void FFAcrossfade::fromJSON(const QJsonObject& json) {
    m_durationValue = json["duration_value"].toDouble(0.0);
    m_durationUnit = json["duration_unit"].toString("s");
    m_overlap = json["overlap"].toBool(true);
    m_curve1 = json["curve1"].toInt(0);
    m_curve2 = json["curve2"].toInt(0);
    m_sidechainInputIndex = json["sidechain_input_index"].toInt(-1);
    updateFFmpegFlags();
}
