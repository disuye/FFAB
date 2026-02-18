#include "ff-loudnorm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFLoudnorm::FFLoudnorm() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFLoudnorm::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: TARGETS ==========
    auto targetsTab = new QWidget();
    auto targetsLayout = new QVBoxLayout(targetsTab);

    // Integrated Loudness (-70 to -5 LUFS)
    auto integratedGroup = new QGroupBox("Integrated Loudness (LUFS)");
    auto integratedLayout = new QVBoxLayout(integratedGroup);

    integratedSpinBox = new QDoubleSpinBox();
    integratedSpinBox->setMinimum(-70.0);
    integratedSpinBox->setMaximum(-5.0);
    integratedSpinBox->setValue(m_integratedLoudness);
    integratedSpinBox->setDecimals(1);
    integratedSpinBox->setSingleStep(0.5);
    integratedSpinBox->setSuffix(" LUFS");

    auto integratedControlLayout = new QHBoxLayout();
    integratedControlLayout->addWidget(integratedSpinBox);
    integratedControlLayout->addStretch();
    integratedControlLayout->addWidget(new QLabel("-70"));
    integratedSlider = new QSlider(Qt::Horizontal);
    integratedSlider->setMinimum(-700);
    integratedSlider->setMaximum(-50);
    integratedSlider->setValue(static_cast<int>(m_integratedLoudness * 10));
    integratedControlLayout->addWidget(integratedSlider, 1);
    integratedControlLayout->addWidget(new QLabel("-5"));

    auto integratedHint = new QLabel("Target: -24 LUFS (EBU), -16 LUFS (streaming), -14 LUFS (podcast)");
    integratedHint->setStyleSheet("color: gray; font-size: 11px;");
    integratedLayout->addLayout(integratedControlLayout);
    integratedLayout->addWidget(integratedHint);
    targetsLayout->addWidget(integratedGroup);

    // Loudness Range (1 - 50 LU)
    auto lraGroup = new QGroupBox("Loudness Range (LU)");
    auto lraLayout = new QVBoxLayout(lraGroup);

    lraSpinBox = new QDoubleSpinBox();
    lraSpinBox->setMinimum(1.0);
    lraSpinBox->setMaximum(50.0);
    lraSpinBox->setValue(m_loudnessRange);
    lraSpinBox->setDecimals(1);
    lraSpinBox->setSingleStep(0.5);
    lraSpinBox->setSuffix(" LU");

    auto lraControlLayout = new QHBoxLayout();
    lraControlLayout->addWidget(lraSpinBox);
    lraControlLayout->addStretch();
    lraControlLayout->addWidget(new QLabel("1"));
    lraSlider = new QSlider(Qt::Horizontal);
    lraSlider->setMinimum(10);
    lraSlider->setMaximum(500);
    lraSlider->setValue(static_cast<int>(m_loudnessRange * 10));
    lraControlLayout->addWidget(lraSlider, 1);
    lraControlLayout->addWidget(new QLabel("50"));

    auto lraHint = new QLabel("Dynamic range target. Lower = more compressed");
    lraHint->setStyleSheet("color: gray; font-size: 11px;");
    lraLayout->addLayout(lraControlLayout);
    lraLayout->addWidget(lraHint);
    targetsLayout->addWidget(lraGroup);

    // True Peak (-9 to 0 dBTP)
    auto truePeakGroup = new QGroupBox("True Peak (dBTP)");
    auto truePeakLayout = new QVBoxLayout(truePeakGroup);

    truePeakSpinBox = new QDoubleSpinBox();
    truePeakSpinBox->setMinimum(-9.0);
    truePeakSpinBox->setMaximum(0.0);
    truePeakSpinBox->setValue(m_truePeak);
    truePeakSpinBox->setDecimals(1);
    truePeakSpinBox->setSingleStep(0.1);
    truePeakSpinBox->setSuffix(" dBTP");

    auto truePeakControlLayout = new QHBoxLayout();
    truePeakControlLayout->addWidget(truePeakSpinBox);
    truePeakControlLayout->addStretch();
    truePeakControlLayout->addWidget(new QLabel("-9"));
    truePeakSlider = new QSlider(Qt::Horizontal);
    truePeakSlider->setMinimum(-90);
    truePeakSlider->setMaximum(0);
    truePeakSlider->setValue(static_cast<int>(m_truePeak * 10));
    truePeakControlLayout->addWidget(truePeakSlider, 1);
    truePeakControlLayout->addWidget(new QLabel("0"));

    auto truePeakHint = new QLabel("Maximum true peak. -1 to -2 dBTP typical for streaming");
    truePeakHint->setStyleSheet("color: gray; font-size: 11px;");
    truePeakLayout->addLayout(truePeakControlLayout);
    truePeakLayout->addWidget(truePeakHint);
    targetsLayout->addWidget(truePeakGroup);

    targetsLayout->addStretch();
    tabWidget->addTab(targetsTab, "Targets");

    // ========== TAB 2: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Offset gain (-99 to 99 dB)
    auto offsetGroup = new QGroupBox("Offset Gain (dB)");
    auto offsetLayout = new QVBoxLayout(offsetGroup);

    offsetSpinBox = new QDoubleSpinBox();
    offsetSpinBox->setMinimum(-99.0);
    offsetSpinBox->setMaximum(99.0);
    offsetSpinBox->setValue(m_offset);
    offsetSpinBox->setDecimals(1);
    offsetSpinBox->setSingleStep(0.5);
    offsetSpinBox->setSuffix(" dB");

    auto offsetControlLayout = new QHBoxLayout();
    offsetControlLayout->addWidget(offsetSpinBox);
    offsetControlLayout->addStretch();
    offsetControlLayout->addWidget(new QLabel("-99"));
    offsetSlider = new QSlider(Qt::Horizontal);
    offsetSlider->setMinimum(-990);
    offsetSlider->setMaximum(990);
    offsetSlider->setValue(static_cast<int>(m_offset * 10));
    offsetControlLayout->addWidget(offsetSlider, 1);
    offsetControlLayout->addWidget(new QLabel("+99"));

    auto offsetHint = new QLabel("Additional gain offset applied after normalization");
    offsetHint->setStyleSheet("color: gray; font-size: 11px;");
    offsetLayout->addLayout(offsetControlLayout);
    offsetLayout->addWidget(offsetHint);
    optionsLayout->addWidget(offsetGroup);

    // Linear mode checkbox
    auto linearGroup = new QGroupBox("Processing Mode");
    auto linearLayout = new QVBoxLayout(linearGroup);

    linearCheck = new QCheckBox("Linear normalization (if possible)");
    linearCheck->setChecked(m_linear);
    auto linearHint = new QLabel("When enabled, uses simple gain if target can be reached without limiting");
    linearHint->setStyleSheet("color: gray; font-size: 11px;");
    linearLayout->addWidget(linearCheck);
    linearLayout->addWidget(linearHint);
    optionsLayout->addWidget(linearGroup);

    // Dual mono checkbox
    auto dualMonoGroup = new QGroupBox("Mono Handling");
    auto dualMonoLayout = new QVBoxLayout(dualMonoGroup);

    dualMonoCheck = new QCheckBox("Treat mono as dual-mono");
    dualMonoCheck->setChecked(m_dualMono);
    auto dualMonoHint = new QLabel("Process mono input as if it were stereo");
    dualMonoHint->setStyleSheet("color: gray; font-size: 11px;");
    dualMonoLayout->addWidget(dualMonoCheck);
    dualMonoLayout->addWidget(dualMonoHint);
    optionsLayout->addWidget(dualMonoGroup);

    // Print format combo
    auto printGroup = new QGroupBox("Statistics Output");
    auto printLayout = new QVBoxLayout(printGroup);

    printFormatCombo = new QComboBox();
    printFormatCombo->addItem("None", 0);
    printFormatCombo->addItem("JSON", 1);
    printFormatCombo->addItem("Summary", 2);
    printFormatCombo->setCurrentIndex(m_printFormat);
    auto printHint = new QLabel("Output measured loudness statistics (for two-pass workflows)");
    printHint->setStyleSheet("color: gray; font-size: 11px;");
    printLayout->addWidget(printFormatCombo);
    printLayout->addWidget(printHint);
    optionsLayout->addWidget(printGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Integrated Loudness
    connect(integratedSlider, &QSlider::valueChanged, [this](int value) {
        m_integratedLoudness = value / 10.0;
        integratedSpinBox->blockSignals(true);
        integratedSpinBox->setValue(m_integratedLoudness);
        integratedSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(integratedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_integratedLoudness = value;
        integratedSlider->blockSignals(true);
        integratedSlider->setValue(static_cast<int>(value * 10));
        integratedSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Loudness Range
    connect(lraSlider, &QSlider::valueChanged, [this](int value) {
        m_loudnessRange = value / 10.0;
        lraSpinBox->blockSignals(true);
        lraSpinBox->setValue(m_loudnessRange);
        lraSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(lraSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_loudnessRange = value;
        lraSlider->blockSignals(true);
        lraSlider->setValue(static_cast<int>(value * 10));
        lraSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // True Peak
    connect(truePeakSlider, &QSlider::valueChanged, [this](int value) {
        m_truePeak = value / 10.0;
        truePeakSpinBox->blockSignals(true);
        truePeakSpinBox->setValue(m_truePeak);
        truePeakSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(truePeakSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_truePeak = value;
        truePeakSlider->blockSignals(true);
        truePeakSlider->setValue(static_cast<int>(value * 10));
        truePeakSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Offset
    connect(offsetSlider, &QSlider::valueChanged, [this](int value) {
        m_offset = value / 10.0;
        offsetSpinBox->blockSignals(true);
        offsetSpinBox->setValue(m_offset);
        offsetSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(offsetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_offset = value;
        offsetSlider->blockSignals(true);
        offsetSlider->setValue(static_cast<int>(value * 10));
        offsetSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Checkboxes
    connect(linearCheck, &QCheckBox::toggled, [this](bool checked) {
        m_linear = checked;
        updateFFmpegFlags();
    });
    connect(dualMonoCheck, &QCheckBox::toggled, [this](bool checked) {
        m_dualMono = checked;
        updateFFmpegFlags();
    });

    // Print format
    connect(printFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_printFormat = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFLoudnorm::updateFFmpegFlags() {
    QStringList printNames = {"none", "json", "summary"};
    QString printName = (m_printFormat >= 0 && m_printFormat < printNames.size()) ? printNames[m_printFormat] : "none";
    
    ffmpegFlags = QString("loudnorm=I=%1:LRA=%2:TP=%3:offset=%4:linear=%5:dual_mono=%6:print_format=%7")
                      .arg(m_integratedLoudness, 0, 'f', 1)
                      .arg(m_loudnessRange, 0, 'f', 1)
                      .arg(m_truePeak, 0, 'f', 1)
                      .arg(m_offset, 0, 'f', 1)
                      .arg(m_linear ? "true" : "false")
                      .arg(m_dualMono ? "true" : "false")
                      .arg(printName);
}

QString FFLoudnorm::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFLoudnorm::toJSON(QJsonObject& json) const {
    json["type"] = "ff-loudnorm";
    json["integratedLoudness"] = m_integratedLoudness;
    json["loudnessRange"] = m_loudnessRange;
    json["truePeak"] = m_truePeak;
    json["offset"] = m_offset;
    json["linear"] = m_linear;
    json["dualMono"] = m_dualMono;
    json["printFormat"] = m_printFormat;
}

void FFLoudnorm::fromJSON(const QJsonObject& json) {
    m_integratedLoudness = json["integratedLoudness"].toDouble(-24.0);
    m_loudnessRange = json["loudnessRange"].toDouble(7.0);
    m_truePeak = json["truePeak"].toDouble(-2.0);
    m_offset = json["offset"].toDouble(0.0);
    m_linear = json["linear"].toBool(true);
    m_dualMono = json["dualMono"].toBool(false);
    m_printFormat = json["printFormat"].toInt(0);
    updateFFmpegFlags();
}
