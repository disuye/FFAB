#include "ff-firequalizer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFFirequalizer::FFFirequalizer() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFFirequalizer::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: GAIN CURVE ==========
    auto gainTab = new QWidget();
    auto gainLayout = new QVBoxLayout(gainTab);

    // Presets
    auto presetGroup = new QGroupBox("Presets");
    auto presetLayout = new QVBoxLayout(presetGroup);

    presetCombo = new QComboBox();
    presetCombo->addItem("(Custom)", -1);
    presetCombo->addItem("Flat (no EQ)", 0);
    presetCombo->addItem("Bass Boost (+6dB below 200Hz)", 1);
    presetCombo->addItem("Treble Boost (+6dB above 4kHz)", 2);
    presetCombo->addItem("Bass Cut (-6dB below 200Hz)", 3);
    presetCombo->addItem("Loudness Curve", 4);
    presetCombo->addItem("Vocal Presence (+3dB 2-4kHz)", 5);
    presetCombo->addItem("Hi-Fi Smile (+3dB lows & highs)", 6);
    presetLayout->addWidget(presetCombo);

    gainLayout->addWidget(presetGroup);

    // Gain entry
    auto entryGroup = new QGroupBox("Gain Entry Points");
    auto entryLayout = new QVBoxLayout(entryGroup);

    gainEntryEdit = new QTextEdit();
    gainEntryEdit->setPlaceholderText("entry(0,0); entry(100,5); entry(1000,0); entry(10000,-3)");
    gainEntryEdit->setText(m_gainEntry);
    gainEntryEdit->setMaximumHeight(100);
    entryLayout->addWidget(gainEntryEdit);

    auto entryHint = new QLabel(
        "Format: entry(freq_hz, gain_db); entry(freq_hz, gain_db); ...\n"
        "The gain curve is interpolated between entry points.\n"
        "Example: entry(0,0); entry(200,6); entry(500,0) — bass boost"
    );
    entryHint->setStyleSheet("color: gray; font-size: 11px;");
    entryHint->setWordWrap(true);
    entryLayout->addWidget(entryHint);

    gainLayout->addWidget(entryGroup);

    // Scale
    auto scaleGroup = new QGroupBox("Gain Scale");
    auto scaleLayout = new QVBoxLayout(scaleGroup);

    scaleCombo = new QComboBox();
    scaleCombo->addItem("Linear-Log (dB, smooth)", 0);
    scaleCombo->addItem("Linear (raw amplitude)", 1);
    scaleCombo->addItem("Logarithmic (dB)", 2);
    scaleCombo->addItem("Linear-Linear", 3);
    scaleCombo->setCurrentIndex(m_scale);
    scaleLayout->addWidget(scaleCombo);

    gainLayout->addWidget(scaleGroup);

    gainLayout->addStretch();
    tabWidget->addTab(gainTab, "Gain Curve");

    // ========== TAB 2: FILTER OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Delay
    auto delayGroup = new QGroupBox("Filter Delay (seconds)");
    auto delayLayout = new QVBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(0.001);
    delaySpinBox->setMaximum(1.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(3);
    delaySpinBox->setSingleStep(0.005);
    delaySpinBox->setSuffix(" s");

    auto delayControlLayout = new QHBoxLayout();
    delayControlLayout->addWidget(delaySpinBox);
    delayControlLayout->addWidget(new QLabel("0.001"));
    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(1);
    delaySlider->setMaximum(1000);
    delaySlider->setValue(static_cast<int>(m_delay * 1000));
    delayControlLayout->addWidget(delaySlider, 1);
    delayControlLayout->addWidget(new QLabel("1.0"));
    delayLayout->addLayout(delayControlLayout);
    delayLayout->addWidget(new QLabel("Longer delay = better frequency resolution."));

    optionsLayout->addWidget(delayGroup);

    // Accuracy
    auto accuracyGroup = new QGroupBox("Accuracy");
    auto accuracyLayout = new QVBoxLayout(accuracyGroup);

    accuracySpinBox = new QDoubleSpinBox();
    accuracySpinBox->setMinimum(0.0);
    accuracySpinBox->setMaximum(100.0);
    accuracySpinBox->setValue(m_accuracy);
    accuracySpinBox->setDecimals(1);

    auto accuracyControlLayout = new QHBoxLayout();
    accuracyControlLayout->addWidget(accuracySpinBox);
    accuracyControlLayout->addWidget(new QLabel("0"));
    accuracySlider = new QSlider(Qt::Horizontal);
    accuracySlider->setMinimum(0);
    accuracySlider->setMaximum(1000);
    accuracySlider->setValue(static_cast<int>(m_accuracy * 10));
    accuracyControlLayout->addWidget(accuracySlider, 1);
    accuracyControlLayout->addWidget(new QLabel("100"));
    accuracyLayout->addLayout(accuracyControlLayout);
    accuracyLayout->addWidget(new QLabel("Higher = more precise EQ curve but longer filter."));

    optionsLayout->addWidget(accuracyGroup);

    // Window function
    auto wfuncGroup = new QGroupBox("Window Function");
    auto wfuncLayout = new QVBoxLayout(wfuncGroup);

    wfuncCombo = new QComboBox();
    wfuncCombo->addItem("Rectangular (harsh)", 0);
    wfuncCombo->addItem("Hann (musical)", 1);
    wfuncCombo->addItem("Hamming (musical)", 2);
    wfuncCombo->addItem("Blackman (musical)", 3);
    wfuncCombo->addItem("Nuttall 3-term (precise)", 4);
    wfuncCombo->addItem("Nuttall 3-term MSL (precise)", 5);
    wfuncCombo->addItem("Nuttall 4-term (precise)", 6);
    wfuncCombo->addItem("Blackman-Nuttall (surgical)", 7);
    wfuncCombo->addItem("Blackman-Harris (surgical)", 8);
    wfuncCombo->addItem("Tukey (hybrid)", 9);
    wfuncCombo->setCurrentIndex(m_wfunc);
    wfuncLayout->addWidget(wfuncCombo);

    optionsLayout->addWidget(wfuncGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Filter Options");

    // ========== TAB 3: PHASE OPTIONS ==========
    auto phaseTab = new QWidget();
    auto phaseLayout = new QVBoxLayout(phaseTab);

    auto phaseGroup = new QGroupBox("Phase Response");
    auto phaseGroupLayout = new QVBoxLayout(phaseGroup);

    zeroPhaseCheck = new QCheckBox("Zero Phase (linear phase, symmetric)");
    zeroPhaseCheck->setChecked(m_zeroPhase);
    phaseGroupLayout->addWidget(zeroPhaseCheck);

    minPhaseCheck = new QCheckBox("Minimum Phase (reduced latency)");
    minPhaseCheck->setChecked(m_minPhase);
    phaseGroupLayout->addWidget(minPhaseCheck);

    auto phaseHint = new QLabel(
        "Zero phase: No phase distortion but adds latency\n"
        "Minimum phase: Lower latency but some phase shift\n"
        "Both off: Natural (mixed) phase response"
    );
    phaseHint->setStyleSheet("color: gray; font-size: 11px;");
    phaseHint->setWordWrap(true);
    phaseGroupLayout->addWidget(phaseHint);

    phaseLayout->addWidget(phaseGroup);

    auto miscGroup = new QGroupBox("Other Options");
    auto miscLayout = new QVBoxLayout(miscGroup);

    fixedCheck = new QCheckBox("Fixed frame samples");
    fixedCheck->setChecked(m_fixed);
    miscLayout->addWidget(fixedCheck);

    multiCheck = new QCheckBox("Multi-channel mode (process channels separately)");
    multiCheck->setChecked(m_multi);
    miscLayout->addWidget(multiCheck);

    phaseLayout->addWidget(miscGroup);

    phaseLayout->addStretch();
    tabWidget->addTab(phaseTab, "Phase");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Presets
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index > 0) {
            applyPreset(presetCombo->itemData(index).toInt());
        }
    });

    // Gain entry
    connect(gainEntryEdit, &QTextEdit::textChanged, [this]() {
        m_gainEntry = gainEntryEdit->toPlainText().simplified();
        presetCombo->blockSignals(true);
        presetCombo->setCurrentIndex(0);
        presetCombo->blockSignals(false);
        updateFFmpegFlags();
    });

    // Scale
    connect(scaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_scale = index;
        updateFFmpegFlags();
    });

    // Delay
    connect(delaySlider, &QSlider::valueChanged, [this](int value) {
        m_delay = value / 1000.0;
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_delay = value;
        delaySlider->blockSignals(true);
        delaySlider->setValue(static_cast<int>(value * 1000));
        delaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Accuracy
    connect(accuracySlider, &QSlider::valueChanged, [this](int value) {
        m_accuracy = value / 10.0;
        accuracySpinBox->blockSignals(true);
        accuracySpinBox->setValue(m_accuracy);
        accuracySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(accuracySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_accuracy = value;
        accuracySlider->blockSignals(true);
        accuracySlider->setValue(static_cast<int>(value * 10));
        accuracySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Window function
    connect(wfuncCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_wfunc = index;
        updateFFmpegFlags();
    });

    // Phase options
    connect(zeroPhaseCheck, &QCheckBox::toggled, [this](bool checked) {
        m_zeroPhase = checked;
        updateFFmpegFlags();
    });
    connect(minPhaseCheck, &QCheckBox::toggled, [this](bool checked) {
        m_minPhase = checked;
        updateFFmpegFlags();
    });
    connect(fixedCheck, &QCheckBox::toggled, [this](bool checked) {
        m_fixed = checked;
        updateFFmpegFlags();
    });
    connect(multiCheck, &QCheckBox::toggled, [this](bool checked) {
        m_multi = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFFirequalizer::applyPreset(int presetIndex) {
    QString entry;
    
    switch (presetIndex) {
        case 0: // Flat
            entry = "entry(0,0)";
            break;
        case 1: // Bass boost
            entry = "entry(0,6); entry(200,6); entry(400,0); entry(20000,0)";
            break;
        case 2: // Treble boost
            entry = "entry(0,0); entry(2000,0); entry(4000,6); entry(20000,6)";
            break;
        case 3: // Bass cut
            entry = "entry(0,-6); entry(200,-6); entry(400,0); entry(20000,0)";
            break;
        case 4: // Loudness curve
            entry = "entry(0,6); entry(100,4); entry(400,0); entry(2000,0); entry(4000,2); entry(10000,4); entry(20000,3)";
            break;
        case 5: // Vocal presence
            entry = "entry(0,0); entry(1500,0); entry(2000,3); entry(4000,3); entry(5000,0); entry(20000,0)";
            break;
        case 6: // Hi-Fi smile
            entry = "entry(0,3); entry(100,2); entry(300,0); entry(3000,0); entry(8000,2); entry(20000,3)";
            break;
        default:
            return;
    }
    
    m_gainEntry = entry;
    gainEntryEdit->blockSignals(true);
    gainEntryEdit->setText(entry);
    gainEntryEdit->blockSignals(false);
    
    updateFFmpegFlags();
}

void FFFirequalizer::updateFFmpegFlags() {
    QStringList wfuncNames = {"rectangular", "hann", "hamming", "blackman", "nuttall3", "mnuttall3", "nuttall", "bnuttall", "bharris", "tukey"};
    QStringList scaleNames = {"linlog", "linlin", "loglin", "loglog"};
    
    QString wfuncName = (m_wfunc >= 0 && m_wfunc < wfuncNames.size()) ? wfuncNames[m_wfunc] : "hann";
    QString scaleName = (m_scale >= 0 && m_scale < scaleNames.size()) ? scaleNames[m_scale] : "linlog";
    
    QStringList params;
    
    if (!m_gainEntry.isEmpty()) {
        params << QString("gain_entry='%1'").arg(m_gainEntry);
    }
    
    params << QString("delay=%1").arg(m_delay, 0, 'f', 3);
    params << QString("accuracy=%1").arg(m_accuracy, 0, 'f', 1);
    params << QString("wfunc=%1").arg(wfuncName);
    params << QString("scale=%1").arg(scaleName);
    
    if (m_fixed) params << "fixed=true";
    if (m_multi) params << "multi=true";
    if (m_zeroPhase) params << "zero_phase=true";
    if (m_minPhase) params << "min_phase=true";
    
    ffmpegFlags = QString("firequalizer=%1").arg(params.join(":"));
}

QString FFFirequalizer::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFFirequalizer::toJSON(QJsonObject& json) const {
    json["type"] = "ff-firequalizer";
    json["gain_entry"] = m_gainEntry;
    json["delay"] = m_delay;
    json["accuracy"] = m_accuracy;
    json["wfunc"] = m_wfunc;
    json["scale"] = m_scale;
    json["fixed"] = m_fixed;
    json["multi"] = m_multi;
    json["zero_phase"] = m_zeroPhase;
    json["min_phase"] = m_minPhase;
}

void FFFirequalizer::fromJSON(const QJsonObject& json) {
    m_gainEntry = json["gain_entry"].toString("");
    m_delay = json["delay"].toDouble(0.01);
    m_accuracy = json["accuracy"].toDouble(5.0);
    m_wfunc = json["wfunc"].toInt(0);
    m_scale = json["scale"].toInt(0);
    m_fixed = json["fixed"].toBool(false);
    m_multi = json["multi"].toBool(false);
    m_zeroPhase = json["zero_phase"].toBool(false);
    m_minPhase = json["min_phase"].toBool(false);
    updateFFmpegFlags();
}
