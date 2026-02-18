#include "SmartAuxReturn.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QFont>

SmartAuxReturn::SmartAuxReturn() {
    position = Position::MIDDLE;
}    

QWidget* SmartAuxReturn::getParametersWidget() {
    if (parametersWidget) return parametersWidget;
    
    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("smartAuxReturnTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);
    
    // Title
    auto titleLabel = new QLabel("Smart Aux Return");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Info label
    infoLabel = new QLabel(
        "This filter captures <u>all</u> loose audio streams positioned above it; mixes back into the main [0:a] audio chain."
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #808080; padding: 8px;");
    mainLayout->addWidget(infoLabel);
    
    // Mix level section
    auto mixGroup = new QGroupBox("Mix Level");
    auto mixLayout = new QFormLayout(mixGroup);

    mixSpinBox = new QDoubleSpinBox();
    mixSpinBox->setRange(0.0, 2.0);
    mixSpinBox->setSingleStep(0.1);
    mixSpinBox->setDecimals(2);
    mixSpinBox->setValue(mixLevel);
    mixSpinBox->setToolTip("Insert chain blend level\n0.0 = silent\n0.5 = balanced (default)\n1.0 = equal with main\n2.0 = double insert level");
    connect(mixSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        mixLevel = value;
        updateFFmpegFlags();
    });    
    mixLayout->addRow("Insert Mix:", mixSpinBox);

    mainLayout->addWidget(mixGroup);

    // Duration & Behavior section
    auto behaviorGroup = new QGroupBox("Duration & Behavior");
    auto behaviorLayout = new QFormLayout(behaviorGroup);

    // Duration mode
    durationCombo = new QComboBox();
    durationCombo->addItem("Longest Input", static_cast<int>(DurationMode::Longest));
    durationCombo->addItem("Shortest Input", static_cast<int>(DurationMode::Shortest));
    durationCombo->addItem("First Input", static_cast<int>(DurationMode::First));
    durationCombo->setCurrentIndex(static_cast<int>(durationMode));
    durationCombo->setToolTip("How to determine output duration:\n"
                              "Longest: Use longest input (default)\n"
                              "Shortest: Use shortest input\n"
                              "First: Use first input (main chain)");
    connect(durationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {                      
        durationMode = static_cast<DurationMode>(durationCombo->currentData().toInt());
        updateFFmpegFlags();
    });    
    behaviorLayout->addRow("Duration:", durationCombo);

    // Dropout transition
    dropoutSpinBox = new QDoubleSpinBox();
    dropoutSpinBox->setRange(0.0, 10.0);
    dropoutSpinBox->setSingleStep(0.1);
    dropoutSpinBox->setDecimals(1);
    dropoutSpinBox->setSuffix(" sec");
    dropoutSpinBox->setValue(dropoutTransition);
    dropoutSpinBox->setToolTip("Volume renormalization transition when an input ends\n"
                               "Default: 2.0 seconds");
    connect(dropoutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {                       
        dropoutTransition = value;
        updateFFmpegFlags();
    });    
    behaviorLayout->addRow("Dropout Transition:", dropoutSpinBox);

    // Normalize checkbox
    normalizeCheckBox = new QCheckBox("Enable normalization");
    normalizeCheckBox->setChecked(normalize);
    normalizeCheckBox->setToolTip("Scale inputs instead of simple summation\n"
                                  "Prevents clipping but may reduce perceived volume\n"
                                  "Enabled by default (recommended)");
    connect(normalizeCheckBox, &QCheckBox::toggled, [this](bool checked) {                          
        normalize = checked;
        updateFFmpegFlags();
    });    
    behaviorLayout->addRow("Normalize:", normalizeCheckBox);

    mainLayout->addWidget(behaviorGroup);

    // Collapsible help section
    auto helpSection = new CollapsibleHelpSection(
        "<b>Smart Aux Return:</b><br>"
        "Automatically detects and mixes processed insert chains back into the main signal.<br><br>"
        
        "<b>Example 1: Multitrack Audio Bus</b><br>"
        "Audio Input [1:a] → Volume → EQ → IR Reverb → <b>Smart Aux Return</b><br>"
        "Result: Volume+EQ+Reverb applied to [1:a]...[N:a], all mixed back into main [0:a] chain.<br><br>"
        
        "<b>Example 2: Parallel Effects</b><br>"
        "(⊙෴☉) asplit CURRENTLY NOT IMPLEMENTED<br>"
        "[0:a] → Asplit=3 → Compress / Reverb / EQ<br> → <b>Smart Aux Return</b><br>"
        "Result: Both processed sidechains are mixed back into main [0:a] chain.<br><br>"
        
        "<b>Mix Level:</b><br>"
        "0.0 = Only the main chain [0:a] is heard.<br>"
        "0.5 = Blend (main & aux signals all reduced, based on number of inputs) <br>"
        "1.0 = Equal weight (main and aux signals all max)<br>"
        "2.0 = Aux signal(s) double the volume of the main chain.<br><br>"
        
        "<b>Duration Modes:</b><br>"
        "Longest: Output continues until longest input ends<br>"
        "Shortest: Output stops when shortest input ends<br>"
        "First: Output duration matches main chain<br><br>"
        
        "<b>Normalize / Dropout:</b><br>"
        "The speed at which Normalization to 0dBFS occurs = Dropout Transition time."
    );
    mainLayout->addWidget(helpSection);

    mainLayout->addStretch();

    return parametersWidget;
}

void SmartAuxReturn::updateFFmpegFlags() {
    // Flags are generated dynamically in buildFFmpegFlags()
}

QString SmartAuxReturn::buildFFmpegFlags() const {
    // This is a placeholder - the actual mixing is handled by FilterChain::buildFilterFlags()
    // which detects this filter type and generates the appropriate amix command
    
    // Return a marker so FilterChain knows to handle this specially
    return "SMART_AUX_RETURN";
}

void SmartAuxReturn::toJSON(QJsonObject& json) const {
    json["type"] = "SmartAuxReturn";
    json["mixLevel"] = mixLevel;
    json["durationMode"] = static_cast<int>(durationMode);
    json["dropoutTransition"] = dropoutTransition;
    json["normalize"] = normalize;
}

void SmartAuxReturn::fromJSON(const QJsonObject& json) {
    mixLevel = json["mixLevel"].toDouble(0.5);
    durationMode = static_cast<DurationMode>(json["durationMode"].toInt(0)); // 0 = Longest
    dropoutTransition = json["dropoutTransition"].toDouble(2.0);
    normalize = json["normalize"].toBool(true);
    
    // Update UI if it exists
    if (mixSpinBox) mixSpinBox->setValue(mixLevel);
    if (durationCombo) durationCombo->setCurrentIndex(static_cast<int>(durationMode));
    if (dropoutSpinBox) dropoutSpinBox->setValue(dropoutTransition);
    if (normalizeCheckBox) normalizeCheckBox->setChecked(normalize);
}
