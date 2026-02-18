#include "ff-amerge.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QFont>

FFAmerge::FFAmerge() {
    position = Position::MIDDLE;
    // No need to call updateFFmpegFlags() - flags are generated dynamically
}

QWidget* FFAmerge::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Title
    auto titleLabel = new QLabel("Channel Merge (amerge)");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Preset selection
    auto presetGroup = new QGroupBox("Common Presets");
    auto presetLayout = new QVBoxLayout(presetGroup);

    presetCombo = new QComboBox();
    presetCombo->addItem("Custom", 0);
    presetCombo->addItem("Stereo (2 mono → stereo)", 2);
    presetCombo->addItem("2.1 (L + R + LFE)", 3);
    presetCombo->addItem("Quad (4 channels)", 4);
    presetCombo->addItem("5.0 Surround", 5);
    presetCombo->addItem("5.1 Surround", 6);
    presetCombo->addItem("7.1 Surround", 8);
    presetCombo->setCurrentIndex(1); // Default to Stereo
    
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        int presetValue = presetCombo->currentData().toInt();
        if (presetValue > 0 && inputCountSpinBox) {
            inputCountSpinBox->setValue(presetValue);
        }
    });
    presetLayout->addWidget(presetCombo);
    mainLayout->addWidget(presetGroup);

    // Manual input count
    auto inputGroup = new QGroupBox("Input Configuration");
    auto inputLayout = new QFormLayout(inputGroup);

    inputCountSpinBox = new QSpinBox();
    inputCountSpinBox->setRange(2, 64);
    inputCountSpinBox->setValue(inputCount);
    inputCountSpinBox->setToolTip("Number of input streams to merge (requires matching AudioInput filters)");
    connect(inputCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        inputCount = value;
        updateFFmpegFlags();
        // Reset preset combo to "Custom" when manually changing
        if (presetCombo) {
            presetCombo->blockSignals(true);
            presetCombo->setCurrentIndex(0);
            presetCombo->blockSignals(false);
        }
    });
    inputLayout->addRow("Input Count:", inputCountSpinBox);

    mainLayout->addWidget(inputGroup);

    // Collapsible help section at bottom
    auto helpSection = new CollapsibleHelpSection(
        "<b>⚠️  Multi-input filter:</b> Requires Audio Input filter(s) before this filter. "
        "Each Audio Input provides one channel to merge into the output.<br><br>"
        
        "<b>How it works:</b><br>"
        "[0:a] = Primary input (from main file list)<br>"
        "[1:a], [2:a], etc. = Additional inputs (from Audio Input filters)<br>"
        "All inputs are merged into a <i>single</i> multi-channel output file.<br><br>"
        
        "<b>Common use cases:</b><br>"
        "• <b>2 inputs:</b> Mono-L + Mono-R → Stereo<br>"
        "• <b>6 inputs:</b> L + R + C + LFE + SL + SR → 5.1 surround<br>"
        "• <b>8 inputs:</b> Full 7.1 surround sound<br><br>"
        
        "<b>⚠️  Important:</b><br>"
        "• Input order matters! First input = first channel, second = second channel, etc.<br>"
        "• You need <b>" + QString::number(inputCount - 1) + " AudioInput filter(s)</b> before this filter<br>"
        "• All inputs should have the same sample rate for best results"
    );
    mainLayout->addWidget(helpSection);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAmerge::updateFFmpegFlags() {
    // Flags are now generated dynamically in buildFFmpegFlags()
    // This method kept for UI update triggers but is now a no-op
}

void FFAmerge::updateInfoLabel() {
    if (!channelLayoutLabel) return;
    
    QString layoutInfo;
    switch (inputCount) {
        case 2:
            layoutInfo = "ℹ️  <b>Output:</b> Stereo (L, R)";
            break;
        case 3:
            layoutInfo = "ℹ️  <b>Output:</b> 2.1 (L, R, LFE)";
            break;
        case 4:
            layoutInfo = "ℹ️  <b>Output:</b> Quad (FL, FR, BL, BR)";
            break;
        case 5:
            layoutInfo = "ℹ️  <b>Output:</b> 5.0 (L, R, C, SL, SR)";
            break;
        case 6:
            layoutInfo = "ℹ️  <b>Output:</b> 5.1 (L, R, C, LFE, SL, SR)";
            break;
        case 8:
            layoutInfo = "ℹ️  <b>Output:</b> 7.1 (L, R, C, LFE, BL, BR, SL, SR)";
            break;
        default:
            layoutInfo = QString("ℹ️  <b>Output:</b> %1 channels (custom layout)").arg(inputCount);
            break;
    }
    
    channelLayoutLabel->setText(layoutInfo);
}

QString FFAmerge::buildFFmpegFlags() const {
    // Generate flags dynamically to ensure inputCount is always current
    // Build input labels: [0:a][1:a][2:a]...
    QStringList inputLabels;
    for (int i = 0; i < inputCount; ++i) {
        inputLabels << QString("[%1:a]").arg(i);
    }
    
    // Build the full filter string
    return QString("%1amerge=inputs=%2")
                    .arg(inputLabels.join(""))
                    .arg(inputCount);
}

void FFAmerge::toJSON(QJsonObject& json) const {
    json["type"] = "ff-amerge";
    json["inputCount"] = inputCount;
}

void FFAmerge::fromJSON(const QJsonObject& json) {
    inputCount = json["inputCount"].toInt(2);
    // Note: flags are generated dynamically in buildFFmpegFlags()
    
    // Update UI if it exists
    if (inputCountSpinBox) {
        inputCountSpinBox->setValue(inputCount);
    }
    
    // Try to match preset
    if (presetCombo) {
        int presetIndex = -1;
        switch (inputCount) {
            case 2: presetIndex = 1; break;
            case 3: presetIndex = 2; break;
            case 4: presetIndex = 3; break;
            case 5: presetIndex = 4; break;
            case 6: presetIndex = 5; break;
            case 8: presetIndex = 6; break;
            default: presetIndex = 0; break; // Custom
        }
        presetCombo->setCurrentIndex(presetIndex);
    }
}
