#include "ff-amix.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QJsonObject>
#include <QGroupBox>
#include <QFont>

FFAmix::FFAmix() {
    position = Position::MIDDLE;
}

QWidget* FFAmix::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Title
    auto titleLabel = new QLabel("Audio Mix (amix)");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Input configuration
    auto inputGroup = new QGroupBox("Input Configuration");
    auto inputLayout = new QFormLayout(inputGroup);

    inputCountSpinBox = new QSpinBox();
    inputCountSpinBox->setRange(2, 32);
    inputCountSpinBox->setValue(inputCount);
    inputCountSpinBox->setToolTip("Number of audio streams to mix together");
    connect(inputCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        inputCount = value;
    });
    inputLayout->addRow("Input Count:", inputCountSpinBox);

    weightsEdit = new QLineEdit();
    weightsEdit->setPlaceholderText("(values 0.0~2.0)");
    weightsEdit->setText(weights);
    weightsEdit->setToolTip("Weight for each input, space-separated. Last value applies to remaining inputs.");
    connect(weightsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        weights = text;
    });
    inputLayout->addRow("Weights:", weightsEdit);

    mainLayout->addWidget(inputGroup);

    // Duration settings
    auto durationGroup = new QGroupBox("Duration");
    auto durationLayout = new QFormLayout(durationGroup);

    durationCombo = new QComboBox();
    durationCombo->addItem("Longest input", "longest");
    durationCombo->addItem("Shortest input", "shortest");
    durationCombo->addItem("First input", "first");
    durationCombo->setToolTip("How to determine output duration");
    connect(durationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        duration = durationCombo->currentData().toString();
    });
    durationLayout->addRow("Duration:", durationCombo);

    dropoutSpinBox = new QDoubleSpinBox();
    dropoutSpinBox->setRange(0.0, 9000.0);
    dropoutSpinBox->setSingleStep(0.5);
    dropoutSpinBox->setDecimals(1);
    dropoutSpinBox->setValue(dropoutTransition);
    dropoutSpinBox->setSuffix(" sec");
    dropoutSpinBox->setToolTip("Transition time when an input stream ends");
    connect(dropoutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        dropoutTransition = value;
    });
    durationLayout->addRow("Dropout Transition:", dropoutSpinBox);

    mainLayout->addWidget(durationGroup);

    // Output settings
    auto outputGroup = new QGroupBox("Output");
    auto outputLayout = new QFormLayout(outputGroup);

    normalizeCheckBox = new QCheckBox("Normalize output levels");
    normalizeCheckBox->setChecked(normalize);
    normalizeCheckBox->setToolTip("Scale inputs to prevent clipping (recommended)");
    connect(normalizeCheckBox, &QCheckBox::toggled, [this](bool checked) {
        normalize = checked;
    });
    outputLayout->addRow("", normalizeCheckBox);

    mainLayout->addWidget(outputGroup);

    // Collapsible help section
    auto helpSection = new CollapsibleHelpSection(
        "<b>⚠️  Multi-input filter:</b> Requires <i>Audio Input</i> filter(s) before this filter.\n "
        "For complex amix chains, <i>Custom FFmpeg</i> with '[✓]Manage output labels manually' is another option.<br><br>"

        "<b>How it works:</b><br>"
        "[0:a] = Primary input (from main file list)<br>"
        "[1:a], [2:a], etc. = Additional file inputs<br>"
        "All inputs are mixed together into a single output.<br><br>"
        
        "<b>Weights (0.0~2.0):</b><br>"
        "• Default is <b>1.0</b>, input gain = output gain<br>"
        "• For a 2-channel mix, <b>0.7 0.7</b> approximates equal power output<br>"
        "• <b>1.0 0.5</b> makes the second input half as loud<br>"
        "• <b>1.0 2.0</b> boosts the second input<br><br>"
        
        "<b>Duration modes:</b><br>"
        "• <b>Longest:</b> Output continues until all inputs end<br>"
        "• <b>Shortest:</b> Output ends when first input ends<br>"
        "• <b>First:</b> Output duration matches first input<br><br>"
        
    );
    mainLayout->addWidget(helpSection);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAmix::updateFFmpegFlags() {
    // No-op: flags generated dynamically in buildFFmpegFlags()
}

QString FFAmix::buildFFmpegFlags() const {
    // Build input labels: [0:a][1:a][2:a]...
    QStringList inputLabels;
    for (int i = 0; i < inputCount; ++i) {
        inputLabels << QString("[%1:a]").arg(i);
    }
    
    QStringList params;
    params << QString("inputs=%1").arg(inputCount);
    params << QString("duration=%1").arg(duration);
    
    if (dropoutTransition != 2.0) {
        params << QString("dropout_transition=%1").arg(dropoutTransition);
    }
    
    if (!weights.trimmed().isEmpty()) {
        params << QString("weights=%1").arg(weights.trimmed());
    }
    
    if (!normalize) {
        params << "normalize=0";
    }
    
    return QString("%1amix=%2")
        .arg(inputLabels.join(""))
        .arg(params.join(":"));
}

void FFAmix::toJSON(QJsonObject& json) const {
    json["type"] = "ff-amix";
    json["inputCount"] = inputCount;
    json["duration"] = duration;
    json["dropoutTransition"] = dropoutTransition;
    json["weights"] = weights;
    json["normalize"] = normalize;
}

void FFAmix::fromJSON(const QJsonObject& json) {
    inputCount = json["inputCount"].toInt(2);
    duration = json["duration"].toString("longest");
    dropoutTransition = json["dropoutTransition"].toDouble(2.0);
    weights = json["weights"].toString("");
    normalize = json["normalize"].toBool(true);
    
    // Update UI if it exists
    if (inputCountSpinBox) inputCountSpinBox->setValue(inputCount);
    if (durationCombo) {
        int idx = durationCombo->findData(duration);
        if (idx >= 0) durationCombo->setCurrentIndex(idx);
    }
    if (dropoutSpinBox) dropoutSpinBox->setValue(dropoutTransition);
    if (weightsEdit) weightsEdit->setText(weights);
    if (normalizeCheckBox) normalizeCheckBox->setChecked(normalize);
}
