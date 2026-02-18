#include "ff-join.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFJoin::FFJoin() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFJoin::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Multi-input warning
    auto warningGroup = new QGroupBox("⚠️ Multi-Input Filter");
    auto warningLayout = new QVBoxLayout(warningGroup);
    warningGroup->setStyleSheet("QGroupBox { color: #ff9944; font-weight: bold; }");

    auto warningLabel = new QLabel(
        "This filter requires MULTIPLE audio inputs.\n"
        "Connect the required number of audio sources to combine them "
        "into a single multi-channel output."
    );
    warningLabel->setWordWrap(true);
    warningLayout->addWidget(warningLabel);

    mainLayout->addWidget(warningGroup);

    // Info group
    auto infoGroup = new QGroupBox("Join Audio Streams");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Combines multiple separate audio streams into a single multi-channel "
        "stream. For example, join 6 mono inputs into a 5.1 surround output, "
        "or 2 mono inputs into stereo."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Number of inputs
    auto inputsGroup = new QGroupBox("Number of Inputs");
    auto inputsLayout = new QVBoxLayout(inputsGroup);

    inputsSpinBox = new QSpinBox();
    inputsSpinBox->setMinimum(1);
    inputsSpinBox->setMaximum(64);
    inputsSpinBox->setValue(m_inputs);
    inputsLayout->addWidget(inputsSpinBox);

    auto inputsHint = new QLabel("Number of input streams to join together.");
    inputsHint->setStyleSheet("color: gray; font-size: 11px;");
    inputsLayout->addWidget(inputsHint);

    mainLayout->addWidget(inputsGroup);

    // Output channel layout
    auto layoutGroup = new QGroupBox("Output Channel Layout");
    auto layoutLayout = new QVBoxLayout(layoutGroup);

    layoutCombo = new QComboBox();
    layoutCombo->addItem("Mono", "mono");
    layoutCombo->addItem("Stereo", "stereo");
    layoutCombo->addItem("2.1", "2.1");
    layoutCombo->addItem("3.0", "3.0");
    layoutCombo->addItem("4.0 (Quad)", "quad");
    layoutCombo->addItem("5.0", "5.0");
    layoutCombo->addItem("5.1", "5.1");
    layoutCombo->addItem("6.1", "6.1");
    layoutCombo->addItem("7.1", "7.1");
    layoutCombo->setCurrentIndex(1); // stereo
    layoutLayout->addWidget(layoutCombo);

    auto layoutHint = new QLabel(
        "The channel layout of the output stream. "
        "Number of output channels should match number of inputs × input channels."
    );
    layoutHint->setStyleSheet("color: gray; font-size: 11px;");
    layoutHint->setWordWrap(true);
    layoutLayout->addWidget(layoutHint);

    mainLayout->addWidget(layoutGroup);

    // Channel mapping (optional)
    auto mapGroup = new QGroupBox("Channel Mapping (Optional)");
    auto mapLayout = new QVBoxLayout(mapGroup);

    mapEdit = new QLineEdit();
    mapEdit->setPlaceholderText("e.g., 0.0-FL|1.0-FR|2.0-FC");
    mapEdit->setText(m_map);
    mapLayout->addWidget(mapEdit);

    auto mapHint = new QLabel(
        "Format: input_idx.channel_idx-output_channel\n"
        "Examples:\n"
        "  0.0-FL|1.0-FR  (input 0 ch 0 → FL, input 1 ch 0 → FR)\n"
        "Leave empty for automatic sequential mapping."
    );
    mapHint->setStyleSheet("color: gray; font-size: 11px;");
    mapHint->setWordWrap(true);
    mapLayout->addWidget(mapHint);

    mainLayout->addWidget(mapGroup);

    // Common configurations
    auto examplesGroup = new QGroupBox("Common Configurations");
    auto examplesLayout = new QVBoxLayout(examplesGroup);

    auto examplesLabel = new QLabel(
        "• 2 mono → stereo: inputs=2, layout=stereo\n"
        "• 6 mono → 5.1: inputs=6, layout=5.1\n"
        "• 8 mono → 7.1: inputs=8, layout=7.1\n"
        "• 3 stereo → 5.1: inputs=3, layout=5.1"
    );
    examplesLabel->setStyleSheet("font-size: 11px;");
    examplesLayout->addWidget(examplesLabel);

    mainLayout->addWidget(examplesGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Inputs
    connect(inputsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_inputs = value;
        updateFFmpegFlags();
    });

    // Layout
    connect(layoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_channelLayout = layoutCombo->itemData(index).toString();
        updateFFmpegFlags();
    });

    // Map
    connect(mapEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_map = text.trimmed();
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFJoin::updateFFmpegFlags() {
    QString inputLabels;
    for (int i = 0; i < m_inputs; i++) {
        inputLabels += QString("[%1:a]").arg(i);
    }

    ffmpegFlags = inputLabels + QString("join=inputs=%1:channel_layout=%2")
                      .arg(m_inputs)
                      .arg(m_channelLayout);

    if (!m_map.isEmpty()) {
        ffmpegFlags += QString(":map=%1").arg(m_map);
    }
}

QString FFJoin::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFJoin::toJSON(QJsonObject& json) const {
    json["type"] = "ff-join";
    json["inputs"] = m_inputs;
    json["channel_layout"] = m_channelLayout;
    json["map"] = m_map;
}

void FFJoin::fromJSON(const QJsonObject& json) {
    m_inputs = json["inputs"].toInt(2);
    m_channelLayout = json["channel_layout"].toString("stereo");
    m_map = json["map"].toString("");
    updateFFmpegFlags();
}
