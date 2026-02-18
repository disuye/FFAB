#include "ff-aloop.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QCheckBox>

FFAloop::FFAloop() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAloop::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto layout = new QVBoxLayout(parametersWidget);

    layout->addWidget(new QLabel("Loop"));

    // Loop count
    auto loopLayout = new QHBoxLayout();
    loopLayout->addWidget(new QLabel("Loop Count:"));
    
    loopSpinBox = new QSpinBox();
    loopSpinBox->setMinimum(-1);
    loopSpinBox->setMaximum(999999);
    loopSpinBox->setValue(m_loop);
    loopSpinBox->setSpecialValueText("Infinite");
    loopLayout->addWidget(loopSpinBox);
    loopLayout->addStretch();
    
    layout->addLayout(loopLayout);

    // Size (samples)
    auto sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Loop Length (Samples):"));

    sizeSpinBox = new QSpinBox();
    sizeSpinBox->setMinimum(0);
    sizeSpinBox->setMaximum(2147483647);
    sizeSpinBox->setValue(m_size);
    sizeLayout->addWidget(sizeSpinBox);

    // Loop entire file checkbox
    loopEntireFileCheckBox = new QCheckBox("Loop entire file");
    loopEntireFileCheckBox->setChecked(m_size == 2147483647);
    connect(loopEntireFileCheckBox, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            sizeSpinBox->setValue(2147483647);
            sizeSpinBox->setEnabled(false);  // Disable manual editing when looping
        } else {
            sizeSpinBox->setValue(48000);  // Reset to default
            sizeSpinBox->setEnabled(true);
        }
        m_size = sizeSpinBox->value();
        updateFFmpegFlags();
    });
    // Also update checkbox if spinbox is manually changed to max value
    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        loopEntireFileCheckBox->setChecked(value == 2147483647);
        m_size = value;
        updateFFmpegFlags();
    });
    
    sizeLayout->addWidget(loopEntireFileCheckBox);
    sizeLayout->addStretch();
    
    layout->addLayout(sizeLayout);

    // Start sample
    auto startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel("Start Sample:"));
    
    startSpinBox = new QSpinBox();
    startSpinBox->setMinimum(0);
    startSpinBox->setMaximum(2147483647);
    startSpinBox->setValue(m_start);
    startLayout->addWidget(startSpinBox);
    startLayout->addStretch();
    
    layout->addLayout(startLayout);

    // Time (duration with unit selector)
    auto timeLayout = new QHBoxLayout();
    timeLayout->addWidget(new QLabel("Start Time:"));
    
    timeSpinBox = new QDoubleSpinBox();
    timeSpinBox->setMinimum(0.0);
    timeSpinBox->setMaximum(999999.0);
    timeSpinBox->setValue(m_timeValue);
    timeSpinBox->setDecimals(3);
    timeLayout->addWidget(timeSpinBox);
    
    timeUnitCombo = new QComboBox();
    timeUnitCombo->addItem("seconds", "s");
    timeUnitCombo->addItem("ms", "ms");
    timeUnitCombo->addItem("samples", "samples");
    timeUnitCombo->setCurrentIndex(0);
    timeLayout->addWidget(timeUnitCombo);
    timeLayout->addStretch();
    
    layout->addLayout(timeLayout);

    // Connect signals
    connect(loopSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_loop = value;
        updateFFmpegFlags();
    });

    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_size = value;
        updateFFmpegFlags();
    });

    connect(startSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_start = value;
        updateFFmpegFlags();
    });

    connect(timeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_timeValue = value;
        updateFFmpegFlags();
    });

    connect(timeUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_timeUnit = timeUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    layout->addStretch();
    return parametersWidget;
}

QString FFAloop::formatDuration(double value, const QString& unit) const {
    if (unit == "ms") {
        return QString::number(value) + "ms";
    } else if (unit == "samples") {
        return QString::number(static_cast<int>(value)) + "s";
    } else {
        return QString::number(value);
    }
}

void FFAloop::updateFFmpegFlags() {
    QStringList params;
    
    if (m_loop != 0) {
        params << QString("loop=%1").arg(m_loop);
    }
    
    if (m_size != 0) {
        params << QString("size=%1").arg(m_size);
    }
    
    if (m_start != 0) {
        params << QString("start=%1").arg(m_start);
    }
    
    if (m_timeValue != 0.0) {
        params << QString("time=%1").arg(formatDuration(m_timeValue, m_timeUnit));
    }
    
    if (params.isEmpty()) {
        ffmpegFlags = "aloop";
    } else {
        ffmpegFlags = "aloop=" + params.join(":");
    }
}

QString FFAloop::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAloop::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aloop";
    json["loop"] = m_loop;
    json["size"] = static_cast<double>(m_size);
    json["start"] = static_cast<double>(m_start);
    json["time_value"] = m_timeValue;
    json["time_unit"] = m_timeUnit;
}

void FFAloop::fromJSON(const QJsonObject& json) {
    m_loop = json["loop"].toInt(0);
    m_size = static_cast<qint64>(json["size"].toDouble(0));
    m_start = static_cast<qint64>(json["start"].toDouble(0));
    m_timeValue = json["time_value"].toDouble(0.0);
    m_timeUnit = json["time_unit"].toString("s");
    updateFFmpegFlags();
}
