#include "ff-silenceremove.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <cmath>

FFSilenceremove::FFSilenceremove() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFSilenceremove::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    mainLayout->addWidget(new QLabel("Silence Remove"));

    // Start section
    auto startGroup = new QGroupBox("Start Trimming");
    auto startLayout = new QVBoxLayout(startGroup);

    // Start periods
    auto startPeriodsLayout = new QHBoxLayout();
    startPeriodsLayout->addWidget(new QLabel("Periods:"));
    auto startPeriodsSpinBox = new QSpinBox();
    startPeriodsSpinBox->setMinimum(0);
    startPeriodsSpinBox->setMaximum(9000);
    startPeriodsSpinBox->setValue(m_startPeriods);
    startPeriodsLayout->addWidget(startPeriodsSpinBox);
    startPeriodsLayout->addStretch();
    startLayout->addLayout(startPeriodsLayout);

    // Start duration
    auto startDurLayout = new QHBoxLayout();
    startDurLayout->addWidget(new QLabel("Duration:"));
    auto startDurSpinBox = new QDoubleSpinBox();
    startDurSpinBox->setMinimum(0.0);
    startDurSpinBox->setMaximum(999999.0);
    startDurSpinBox->setValue(m_startDurationValue);
    startDurSpinBox->setDecimals(3);
    startDurLayout->addWidget(startDurSpinBox);
    auto startDurUnitCombo = new QComboBox();
    startDurUnitCombo->addItem("seconds", "s");
    startDurUnitCombo->addItem("ms", "ms");
    startDurUnitCombo->addItem("samples", "samples");
    startDurLayout->addWidget(startDurUnitCombo);
    startDurLayout->addStretch();
    startLayout->addLayout(startDurLayout);

    // Start threshold
    auto startThreshLayout = new QHBoxLayout();
    startThreshLayout->addWidget(new QLabel("Threshold (dB):"));
    auto startThreshSpinBox = new QDoubleSpinBox();
    startThreshSpinBox->setMinimum(-96.0);
    startThreshSpinBox->setMaximum(0.0);
    startThreshSpinBox->setValue(linearToDb(m_startThreshold));
    startThreshSpinBox->setDecimals(1);
    startThreshSpinBox->setSingleStep(0.1);
    startThreshSpinBox->setSuffix(" dB");
    startThreshLayout->addWidget(startThreshSpinBox);
    startThreshLayout->addStretch();
    startLayout->addLayout(startThreshLayout);

    // Start silence
    auto startSilLayout = new QHBoxLayout();
    startSilLayout->addWidget(new QLabel("Silence Keep:"));
    auto startSilSpinBox = new QDoubleSpinBox();
    startSilSpinBox->setMinimum(0.0);
    startSilSpinBox->setMaximum(999999.0);
    startSilSpinBox->setValue(m_startSilenceValue);
    startSilSpinBox->setDecimals(3);
    startSilLayout->addWidget(startSilSpinBox);
    auto startSilUnitCombo = new QComboBox();
    startSilUnitCombo->addItem("seconds", "s");
    startSilUnitCombo->addItem("ms", "ms");
    startSilUnitCombo->addItem("samples", "samples");
    startSilLayout->addWidget(startSilUnitCombo);
    startSilLayout->addStretch();
    startLayout->addLayout(startSilLayout);

    // Start mode
    auto startModeLayout = new QHBoxLayout();
    startModeLayout->addWidget(new QLabel("Mode:"));
    auto startModeCombo = new QComboBox();
    startModeCombo->addItem("Any Channel", 0);
    startModeCombo->addItem("All Channels", 1);
    startModeCombo->setCurrentIndex(m_startMode);
    startModeLayout->addWidget(startModeCombo);
    startModeLayout->addStretch();
    startLayout->addLayout(startModeLayout);

    mainLayout->addWidget(startGroup);

    // Stop section
    auto stopGroup = new QGroupBox("Stop Trimming");
    auto stopLayout = new QVBoxLayout(stopGroup);

    // Stop periods
    auto stopPeriodsLayout = new QHBoxLayout();
    stopPeriodsLayout->addWidget(new QLabel("Periods:"));
    auto stopPeriodsSpinBox = new QSpinBox();
    stopPeriodsSpinBox->setMinimum(-9000);
    stopPeriodsSpinBox->setMaximum(9000);
    stopPeriodsSpinBox->setValue(m_stopPeriods);
    stopPeriodsLayout->addWidget(stopPeriodsSpinBox);
    stopPeriodsLayout->addStretch();
    stopLayout->addLayout(stopPeriodsLayout);

    // Stop duration
    auto stopDurLayout = new QHBoxLayout();
    stopDurLayout->addWidget(new QLabel("Duration:"));
    auto stopDurSpinBox = new QDoubleSpinBox();
    stopDurSpinBox->setMinimum(0.0);
    stopDurSpinBox->setMaximum(999999.0);
    stopDurSpinBox->setValue(m_stopDurationValue);
    stopDurSpinBox->setDecimals(3);
    stopDurLayout->addWidget(stopDurSpinBox);
    auto stopDurUnitCombo = new QComboBox();
    stopDurUnitCombo->addItem("seconds", "s");
    stopDurUnitCombo->addItem("ms", "ms");
    stopDurUnitCombo->addItem("samples", "samples");
    stopDurLayout->addWidget(stopDurUnitCombo);
    stopDurLayout->addStretch();
    stopLayout->addLayout(stopDurLayout);

    // Stop threshold
    auto stopThreshLayout = new QHBoxLayout();
    stopThreshLayout->addWidget(new QLabel("Threshold (dB):"));
    auto stopThreshSpinBox = new QDoubleSpinBox();
    stopThreshSpinBox->setMinimum(-96.0);
    stopThreshSpinBox->setMaximum(0.0);
    stopThreshSpinBox->setValue(linearToDb(m_stopThreshold));
    stopThreshSpinBox->setDecimals(1);
    stopThreshSpinBox->setSingleStep(0.1);
    stopThreshSpinBox->setSuffix(" dB");
    stopThreshLayout->addWidget(stopThreshSpinBox);
    stopThreshLayout->addStretch();
    stopLayout->addLayout(stopThreshLayout);

    // Stop silence
    auto stopSilLayout = new QHBoxLayout();
    stopSilLayout->addWidget(new QLabel("Silence Keep:"));
    auto stopSilSpinBox = new QDoubleSpinBox();
    stopSilSpinBox->setMinimum(0.0);
    stopSilSpinBox->setMaximum(999999.0);
    stopSilSpinBox->setValue(m_stopSilenceValue);
    stopSilSpinBox->setDecimals(3);
    stopSilLayout->addWidget(stopSilSpinBox);
    auto stopSilUnitCombo = new QComboBox();
    stopSilUnitCombo->addItem("seconds", "s");
    stopSilUnitCombo->addItem("ms", "ms");
    stopSilUnitCombo->addItem("samples", "samples");
    stopSilLayout->addWidget(stopSilUnitCombo);
    stopSilLayout->addStretch();
    stopLayout->addLayout(stopSilLayout);

    // Stop mode
    auto stopModeLayout = new QHBoxLayout();
    stopModeLayout->addWidget(new QLabel("Mode:"));
    auto stopModeCombo = new QComboBox();
    stopModeCombo->addItem("Any Channel", 0);
    stopModeCombo->addItem("All Channels", 1);
    stopModeCombo->setCurrentIndex(m_stopMode);
    stopModeLayout->addWidget(stopModeCombo);
    stopModeLayout->addStretch();
    stopLayout->addLayout(stopModeLayout);

    mainLayout->addWidget(stopGroup);

    // Detection settings
    auto detectionGroup = new QGroupBox("Detection Settings");
    auto detectionLayout = new QVBoxLayout(detectionGroup);

    // Detection method
    auto detectionMethodLayout = new QHBoxLayout();
    detectionMethodLayout->addWidget(new QLabel("Detection:"));
    auto detectionCombo = new QComboBox();
    detectionCombo->addItem("Average", 0);
    detectionCombo->addItem("RMS", 1);
    detectionCombo->addItem("Peak", 2);
    detectionCombo->addItem("Median", 3);
    detectionCombo->addItem("Peak-to-Peak", 4);
    detectionCombo->addItem("Std Deviation", 5);
    detectionCombo->setCurrentIndex(m_detection);
    detectionMethodLayout->addWidget(detectionCombo);
    detectionMethodLayout->addStretch();
    detectionLayout->addLayout(detectionMethodLayout);

    // Window
    auto windowLayout = new QHBoxLayout();
    windowLayout->addWidget(new QLabel("Window:"));
    auto windowSpinBox = new QDoubleSpinBox();
    windowSpinBox->setMinimum(0.0);
    windowSpinBox->setMaximum(999999.0);
    windowSpinBox->setValue(m_windowValue);
    windowSpinBox->setDecimals(3);
    windowLayout->addWidget(windowSpinBox);
    auto windowUnitCombo = new QComboBox();
    windowUnitCombo->addItem("seconds", "s");
    windowUnitCombo->addItem("ms", "ms");
    windowUnitCombo->addItem("samples", "samples");
    windowLayout->addWidget(windowUnitCombo);
    windowLayout->addStretch();
    detectionLayout->addLayout(windowLayout);

    // Timestamp mode
    auto timestampLayout = new QHBoxLayout();
    timestampLayout->addWidget(new QLabel("Timestamp:"));
    auto timestampCombo = new QComboBox();
    timestampCombo->addItem("Write (rewrite)", 0);
    timestampCombo->addItem("Copy (preserve)", 1);
    timestampCombo->setCurrentIndex(m_timestamp);
    timestampLayout->addWidget(timestampCombo);
    timestampLayout->addStretch();
    detectionLayout->addLayout(timestampLayout);

    mainLayout->addWidget(detectionGroup);

    // Connect all signals
    connect(startPeriodsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_startPeriods = value;
        updateFFmpegFlags();
    });

    connect(startDurSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_startDurationValue = value;
        updateFFmpegFlags();
    });

    connect(startDurUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, startDurUnitCombo](int) {
        m_startDurationUnit = startDurUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(startThreshSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_startThreshold = (db <= -96.0) ? 0.0 : dbToLinear(db);
        updateFFmpegFlags();
    });

    connect(startSilSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_startSilenceValue = value;
        updateFFmpegFlags();
    });

    connect(startSilUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, startSilUnitCombo](int) {
        m_startSilenceUnit = startSilUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(startModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, startModeCombo](int index) {
        m_startMode = startModeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(stopPeriodsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_stopPeriods = value;
        updateFFmpegFlags();
    });

    connect(stopDurSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_stopDurationValue = value;
        updateFFmpegFlags();
    });

    connect(stopDurUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, stopDurUnitCombo](int) {
        m_stopDurationUnit = stopDurUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(stopThreshSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_stopThreshold = (db <= -96.0) ? 0.0 : dbToLinear(db);
        updateFFmpegFlags();
    });

    connect(stopSilSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_stopSilenceValue = value;
        updateFFmpegFlags();
    });

    connect(stopSilUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, stopSilUnitCombo](int) {
        m_stopSilenceUnit = stopSilUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(stopModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, stopModeCombo](int index) {
        m_stopMode = stopModeCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(detectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, detectionCombo](int index) {
        m_detection = detectionCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    connect(windowSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_windowValue = value;
        updateFFmpegFlags();
    });

    connect(windowUnitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, windowUnitCombo](int) {
        m_windowUnit = windowUnitCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(timestampCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, timestampCombo](int index) {
        m_timestamp = timestampCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    mainLayout->addStretch();
    return parametersWidget;
}

QString FFSilenceremove::formatDuration(double value, const QString& unit) const {
    if (unit == "ms") {
        return QString::number(value) + "ms";
    } else if (unit == "samples") {
        return QString::number(static_cast<int>(value)) + "s";
    } else {
        return QString::number(value);
    }
}

void FFSilenceremove::updateFFmpegFlags() {
    QStringList params;
    
    if (m_startPeriods != 0) {
        params << QString("start_periods=%1").arg(m_startPeriods);
    }
    if (m_startDurationValue != 0.0) {
        params << QString("start_duration=%1").arg(formatDuration(m_startDurationValue, m_startDurationUnit));
    }
    if (m_startThreshold != 0.0) {
        params << QString("start_threshold=%1").arg(m_startThreshold);
    }
    if (m_startSilenceValue != 0.0) {
        params << QString("start_silence=%1").arg(formatDuration(m_startSilenceValue, m_startSilenceUnit));
    }
    if (m_startMode != 0) {
        params << QString("start_mode=%1").arg(m_startMode == 0 ? "any" : "all");
    }
    
    if (m_stopPeriods != 0) {
        params << QString("stop_periods=%1").arg(m_stopPeriods);
    }
    if (m_stopDurationValue != 0.0) {
        params << QString("stop_duration=%1").arg(formatDuration(m_stopDurationValue, m_stopDurationUnit));
    }
    if (m_stopThreshold != 0.0) {
        params << QString("stop_threshold=%1").arg(m_stopThreshold);
    }
    if (m_stopSilenceValue != 0.0) {
        params << QString("stop_silence=%1").arg(formatDuration(m_stopSilenceValue, m_stopSilenceUnit));
    }
    if (m_stopMode != 1) {
        params << QString("stop_mode=%1").arg(m_stopMode == 0 ? "any" : "all");
    }
    
    if (m_detection != 1) {
        QStringList detectionModes = {"avg", "rms", "peak", "median", "ptp", "dev"};
        params << QString("detection=%1").arg(detectionModes[m_detection]);
    }
    
    if (m_windowValue != 0.02) {
        params << QString("window=%1").arg(formatDuration(m_windowValue, m_windowUnit));
    }
    
    if (m_timestamp != 0) {
        params << QString("timestamp=%1").arg(m_timestamp == 0 ? "write" : "copy");
    }
    
    if (params.isEmpty()) {
        ffmpegFlags = "silenceremove";
    } else {
        ffmpegFlags = "silenceremove=" + params.join(":");
    }
}

QString FFSilenceremove::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFSilenceremove::toJSON(QJsonObject& json) const {
    json["type"] = "ff-silenceremove";
    json["start_periods"] = m_startPeriods;
    json["start_duration_value"] = m_startDurationValue;
    json["start_duration_unit"] = m_startDurationUnit;
    json["start_threshold"] = m_startThreshold;
    json["start_silence_value"] = m_startSilenceValue;
    json["start_silence_unit"] = m_startSilenceUnit;
    json["start_mode"] = m_startMode;
    json["stop_periods"] = m_stopPeriods;
    json["stop_duration_value"] = m_stopDurationValue;
    json["stop_duration_unit"] = m_stopDurationUnit;
    json["stop_threshold"] = m_stopThreshold;
    json["stop_silence_value"] = m_stopSilenceValue;
    json["stop_silence_unit"] = m_stopSilenceUnit;
    json["stop_mode"] = m_stopMode;
    json["detection"] = m_detection;
    json["window_value"] = m_windowValue;
    json["window_unit"] = m_windowUnit;
    json["timestamp"] = m_timestamp;
}

void FFSilenceremove::fromJSON(const QJsonObject& json) {
    m_startPeriods = json["start_periods"].toInt(0);
    m_startDurationValue = json["start_duration_value"].toDouble(0.0);
    m_startDurationUnit = json["start_duration_unit"].toString("s");
    m_startThreshold = json["start_threshold"].toDouble(0.0);
    m_startSilenceValue = json["start_silence_value"].toDouble(0.0);
    m_startSilenceUnit = json["start_silence_unit"].toString("s");
    m_startMode = json["start_mode"].toInt(0);
    m_stopPeriods = json["stop_periods"].toInt(0);
    m_stopDurationValue = json["stop_duration_value"].toDouble(0.0);
    m_stopDurationUnit = json["stop_duration_unit"].toString("s");
    m_stopThreshold = json["stop_threshold"].toDouble(0.0);
    m_stopSilenceValue = json["stop_silence_value"].toDouble(0.0);
    m_stopSilenceUnit = json["stop_silence_unit"].toString("s");
    m_stopMode = json["stop_mode"].toInt(1);
    m_detection = json["detection"].toInt(1);
    m_windowValue = json["window_value"].toDouble(0.02);
    m_windowUnit = json["window_unit"].toString("s");
    m_timestamp = json["timestamp"].toInt(0);
    updateFFmpegFlags();
}
