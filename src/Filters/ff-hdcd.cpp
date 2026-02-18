#include "ff-hdcd.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFHdcd::FFHdcd() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFHdcd::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("HDCD Decoding");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Decodes High Definition Compatible Digital (HDCD) encoded audio. "
        "HDCD extends 16-bit audio to ~20-bit dynamic range using hidden "
        "control codes in the least significant bits."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    auto noteLabel = new QLabel("Note: Only affects audio that was HDCD-encoded. Non-HDCD audio passes through unchanged.");
    noteLabel->setStyleSheet("color: gray; font-size: 11px;");
    noteLabel->setWordWrap(true);
    infoLayout->addWidget(noteLabel);

    mainLayout->addWidget(infoGroup);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: DECODING ==========
    auto decodingTab = new QWidget();
    auto decodingLayout = new QVBoxLayout(decodingTab);

    // Bits per sample
    auto bitsGroup = new QGroupBox("Source Bit Depth");
    auto bitsLayout = new QVBoxLayout(bitsGroup);

    bitsPerSampleCombo = new QComboBox();
    bitsPerSampleCombo->addItem("16-bit", 16);
    bitsPerSampleCombo->addItem("20-bit", 20);
    bitsPerSampleCombo->addItem("24-bit", 24);
    bitsPerSampleCombo->setCurrentIndex(m_bitsPerSample == 16 ? 0 : (m_bitsPerSample == 20 ? 1 : 2));
    bitsLayout->addWidget(bitsPerSampleCombo);

    auto bitsHint = new QLabel("Location of the true LSB in the source audio.");
    bitsHint->setStyleSheet("color: gray; font-size: 11px;");
    bitsLayout->addWidget(bitsHint);

    decodingLayout->addWidget(bitsGroup);

    // Process stereo option
    auto stereoGroup = new QGroupBox("Stereo Processing");
    auto stereoLayout = new QVBoxLayout(stereoGroup);

    processStereoCheck = new QCheckBox("Process stereo channels together");
    processStereoCheck->setChecked(m_processStereo);
    stereoLayout->addWidget(processStereoCheck);

    auto stereoHint = new QLabel("Only apply target gain when both channels match. Recommended for stereo sources.");
    stereoHint->setStyleSheet("color: gray; font-size: 11px;");
    stereoHint->setWordWrap(true);
    stereoLayout->addWidget(stereoHint);

    decodingLayout->addWidget(stereoGroup);

    // Force peak extend option
    auto peGroup = new QGroupBox("Peak Extension");
    auto peLayout = new QVBoxLayout(peGroup);

    forcePeCheck = new QCheckBox("Force peak extension above -3 dBFS");
    forcePeCheck->setChecked(m_forcePe);
    peLayout->addWidget(forcePeCheck);

    auto peHint = new QLabel("Always extend peaks even when PE is not signaled in the stream. Use with caution.");
    peHint->setStyleSheet("color: gray; font-size: 11px;");
    peHint->setWordWrap(true);
    peLayout->addWidget(peHint);

    decodingLayout->addWidget(peGroup);

    decodingLayout->addStretch();
    tabWidget->addTab(decodingTab, "Decoding");

    // ========== TAB 2: ADVANCED ==========
    auto advancedTab = new QWidget();
    auto advancedLayout = new QVBoxLayout(advancedTab);

    // Code detect timer
    auto cdtGroup = new QGroupBox("Code Detect Timer (ms)");
    auto cdtLayout = new QVBoxLayout(cdtGroup);

    cdtMsSpinBox = new QSpinBox();
    cdtMsSpinBox->setMinimum(100);
    cdtMsSpinBox->setMaximum(60000);
    cdtMsSpinBox->setValue(m_cdtMs);
    cdtMsSpinBox->setSuffix(" ms");

    auto cdtControlLayout = new QHBoxLayout();
    cdtControlLayout->addWidget(cdtMsSpinBox);
    cdtControlLayout->addStretch();
    
    auto cdtMinLabel = new QLabel("100");
    cdtControlLayout->addWidget(cdtMinLabel);

    cdtMsSlider = new QSlider(Qt::Horizontal);
    cdtMsSlider->setMinimum(100);
    cdtMsSlider->setMaximum(60000);
    cdtMsSlider->setValue(m_cdtMs);
    cdtControlLayout->addWidget(cdtMsSlider, 1);

    auto cdtMaxLabel = new QLabel("60000");
    cdtControlLayout->addWidget(cdtMaxLabel);

    cdtLayout->addLayout(cdtControlLayout);

    auto cdtHint = new QLabel("Period for detecting HDCD control codes. Shorter values respond faster but may be less accurate.");
    cdtHint->setStyleSheet("color: gray; font-size: 11px;");
    cdtHint->setWordWrap(true);
    cdtLayout->addWidget(cdtHint);

    advancedLayout->addWidget(cdtGroup);

    // Disable autoconvert option
    auto autoconvertGroup = new QGroupBox("Format Conversion");
    auto autoconvertLayout = new QVBoxLayout(autoconvertGroup);

    disableAutoconvertCheck = new QCheckBox("Disable automatic format conversion");
    disableAutoconvertCheck->setChecked(m_disableAutoconvert);
    autoconvertLayout->addWidget(disableAutoconvertCheck);

    auto autoconvertHint = new QLabel("Prevents the filter graph from automatically resampling or converting the audio format.");
    autoconvertHint->setStyleSheet("color: gray; font-size: 11px;");
    autoconvertHint->setWordWrap(true);
    autoconvertLayout->addWidget(autoconvertHint);

    advancedLayout->addWidget(autoconvertGroup);

    // Analyze mode
    auto analyzeGroup = new QGroupBox("Analyze Mode");
    auto analyzeLayout = new QVBoxLayout(analyzeGroup);

    analyzeModeCombo = new QComboBox();
    analyzeModeCombo->addItem("Off (Normal Decoding)", 0);
    analyzeModeCombo->addItem("LLE (Low-Level Extension)", 1);
    analyzeModeCombo->addItem("PE (Peak Extension)", 2);
    analyzeModeCombo->addItem("CDT (Code Detect)", 3);
    analyzeModeCombo->addItem("TGM (Target Gain)", 4);
    analyzeModeCombo->setCurrentIndex(m_analyzeMode);
    analyzeLayout->addWidget(analyzeModeCombo);

    auto analyzeHint = new QLabel(
        "Debug mode: Replaces audio with a tone whose amplitude indicates "
        "the selected processing aspect. For analysis only."
    );
    analyzeHint->setStyleSheet("color: gray; font-size: 11px;");
    analyzeHint->setWordWrap(true);
    analyzeLayout->addWidget(analyzeHint);

    advancedLayout->addWidget(analyzeGroup);

    advancedLayout->addStretch();
    tabWidget->addTab(advancedTab, "Advanced");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Bits per sample
    connect(bitsPerSampleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_bitsPerSample = bitsPerSampleCombo->itemData(index).toInt();
        updateFFmpegFlags();
    });

    // Process stereo
    connect(processStereoCheck, &QCheckBox::toggled, [this](bool checked) {
        m_processStereo = checked;
        updateFFmpegFlags();
    });

    // Force PE
    connect(forcePeCheck, &QCheckBox::toggled, [this](bool checked) {
        m_forcePe = checked;
        updateFFmpegFlags();
    });

    // CDT timer
    connect(cdtMsSlider, &QSlider::valueChanged, [this](int value) {
        m_cdtMs = value;
        cdtMsSpinBox->blockSignals(true);
        cdtMsSpinBox->setValue(value);
        cdtMsSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(cdtMsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_cdtMs = value;
        cdtMsSlider->blockSignals(true);
        cdtMsSlider->setValue(value);
        cdtMsSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Disable autoconvert
    connect(disableAutoconvertCheck, &QCheckBox::toggled, [this](bool checked) {
        m_disableAutoconvert = checked;
        updateFFmpegFlags();
    });

    // Analyze mode
    connect(analyzeModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_analyzeMode = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFHdcd::updateFFmpegFlags() {
    QStringList analyzeModeNames = {"off", "lle", "pe", "cdt", "tgm"};
    QString analyzeModeName = (m_analyzeMode >= 0 && m_analyzeMode < analyzeModeNames.size()) 
                              ? analyzeModeNames[m_analyzeMode] : "off";
    
    ffmpegFlags = QString("hdcd=disable_autoconvert=%1:process_stereo=%2:cdt_ms=%3:force_pe=%4:analyze_mode=%5:bits_per_sample=%6")
                      .arg(m_disableAutoconvert ? "true" : "false")
                      .arg(m_processStereo ? "true" : "false")
                      .arg(m_cdtMs)
                      .arg(m_forcePe ? "true" : "false")
                      .arg(analyzeModeName)
                      .arg(m_bitsPerSample);
}

QString FFHdcd::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFHdcd::toJSON(QJsonObject& json) const {
    json["type"] = "ff-hdcd";
    json["disable_autoconvert"] = m_disableAutoconvert;
    json["process_stereo"] = m_processStereo;
    json["cdt_ms"] = m_cdtMs;
    json["force_pe"] = m_forcePe;
    json["analyze_mode"] = m_analyzeMode;
    json["bits_per_sample"] = m_bitsPerSample;
}

void FFHdcd::fromJSON(const QJsonObject& json) {
    m_disableAutoconvert = json["disable_autoconvert"].toBool(true);
    m_processStereo = json["process_stereo"].toBool(true);
    m_cdtMs = json["cdt_ms"].toInt(2000);
    m_forcePe = json["force_pe"].toBool(false);
    m_analyzeMode = json["analyze_mode"].toInt(0);
    m_bitsPerSample = json["bits_per_sample"].toInt(16);
    updateFFmpegFlags();
}
