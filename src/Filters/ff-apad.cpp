#include "ff-apad.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>
#include <QJsonObject>

FFApad::FFApad() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFApad::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);

    mainLayout->addWidget(new QLabel("Pad"));

    // Packet size (always available)
    auto packetLayout = new QHBoxLayout();
    packetLayout->addWidget(new QLabel("Packet Size:"));
    
    packetSizeSpinBox = new QSpinBox();
    packetSizeSpinBox->setMinimum(0);
    packetSizeSpinBox->setMaximum(999999);
    packetSizeSpinBox->setValue(m_packetSize);
    packetLayout->addWidget(packetSizeSpinBox);
    packetLayout->addStretch();
    
    mainLayout->addLayout(packetLayout);

    // Mode selection group
    auto modeGroup = new QGroupBox("Padding Mode");
    auto modeLayout = new QVBoxLayout(modeGroup);

    // Add Silence Mode
    addSilenceRadio = new QRadioButton("Add Silence");
    addSilenceRadio->setChecked(m_mode == PadMode::AddSilence);
    modeLayout->addWidget(addSilenceRadio);

    auto addSilenceHelp = new QLabel("  Add a specific amount of silence to the end");
    QFont helpFont = addSilenceHelp->font();
    helpFont.setPointSize(helpFont.pointSize() - 1);
    addSilenceHelp->setFont(helpFont);
    addSilenceHelp->setStyleSheet("color: gray;");
    modeLayout->addWidget(addSilenceHelp);

    // Target Total Length Mode
    targetLengthRadio = new QRadioButton("Target Total Length");
    targetLengthRadio->setChecked(m_mode == PadMode::TargetLength);
    modeLayout->addWidget(targetLengthRadio);

    auto targetLengthHelp = new QLabel("  Pad to reach a minimum total duration/length");
    targetLengthHelp->setFont(helpFont);
    targetLengthHelp->setStyleSheet("color: gray;");
    modeLayout->addWidget(targetLengthHelp);

    mainLayout->addWidget(modeGroup);

    // Value controls (shared between modes)
    auto valueGroup = new QGroupBox("Amount");
    auto valueLayout = new QHBoxLayout(valueGroup);
    
    valueSpinBox = new QDoubleSpinBox();
    valueSpinBox->setMinimum(0.0);
    valueSpinBox->setMaximum(999999.0);
    valueSpinBox->setValue(m_value);
    valueSpinBox->setDecimals(3);
    valueLayout->addWidget(valueSpinBox);
    
    unitCombo = new QComboBox();
    unitCombo->addItem("seconds", "s");
    unitCombo->addItem("ms", "ms");
    unitCombo->addItem("samples", "samples");
    unitCombo->setCurrentIndex(0);  // seconds default
    valueLayout->addWidget(unitCombo);
    
    valueLayout->addStretch();
    
    mainLayout->addWidget(valueGroup);

    // Connect signals
    connect(packetSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_packetSize = value;
        updateFFmpegFlags();
    });

    connect(addSilenceRadio, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            m_mode = PadMode::AddSilence;
            updateFFmpegFlags();
        }
    });

    connect(targetLengthRadio, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            m_mode = PadMode::TargetLength;
            updateFFmpegFlags();
        }
    });

    connect(valueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_value = value;
        updateFFmpegFlags();
    });

    connect(unitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_unit = unitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    mainLayout->addStretch();
    return parametersWidget;
}

QString FFApad::formatDuration(double value, const QString& unit) const {
    if (unit == "ms") {
        return QString::number(value) + "ms";
    } else if (unit == "samples") {
        return QString::number(static_cast<int>(value)) + "s";
    } else {
        return QString::number(value);
    }
}

void FFApad::updateFFmpegFlags() {
    QStringList params;
    
    if (m_packetSize != 4096) {
        params << QString("packet_size=%1").arg(m_packetSize);
    }
    
    // CRITICAL: Don't add padding if value is == 0 (causes infinite padding!!)
    if (m_value > 0.0) {
        QString paramName;
        
        if (m_mode == PadMode::AddSilence) {
            paramName = (m_unit == "samples") ? "pad_len" : "pad_dur";
        } else {
            paramName = (m_unit == "samples") ? "whole_len" : "whole_dur";
        }
        
        QString formattedValue;
        if (m_unit == "samples") {
            formattedValue = QString::number(static_cast<int>(m_value));
        } else {
            formattedValue = formatDuration(m_value, m_unit);
        }
        
        params << QString("%1=%2").arg(paramName).arg(formattedValue);
    }
    
    // If no padding specified, skip the filter entirely
    if (params.isEmpty() || (params.size() == 1 && params[0].startsWith("packet_size"))) {
        ffmpegFlags = "";  // Empty = filter won't be added to chain
    } else {
        ffmpegFlags = "apad=" + params.join(":");
    }
}

QString FFApad::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFApad::toJSON(QJsonObject& json) const {
    json["type"] = "ff-apad";
    json["packet_size"] = m_packetSize;
    json["mode"] = static_cast<int>(m_mode);
    json["value"] = m_value;
    json["unit"] = m_unit;
}

void FFApad::fromJSON(const QJsonObject& json) {
    m_packetSize = json["packet_size"].toInt(4096);
    m_mode = static_cast<PadMode>(json["mode"].toInt(0));
    m_value = json["value"].toDouble(0.0);
    m_unit = json["unit"].toString("s");
    updateFFmpegFlags();
}