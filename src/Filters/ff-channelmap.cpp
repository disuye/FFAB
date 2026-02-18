#include "ff-channelmap.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFChannelmap::FFChannelmap() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFChannelmap::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "Remap, reorder, duplicate, or drop audio channels.\n"
        "Use channel indices (0=first) or names (FL, FR, FC, etc.)"
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Presets
    auto presetGroup = new QGroupBox("Presets");
    auto presetLayout = new QVBoxLayout(presetGroup);

    presetCombo = new QComboBox();
    presetCombo->addItem("Custom...", -1);
    presetCombo->addItem("Swap L/R", 0);
    presetCombo->addItem("Mono from Left", 1);
    presetCombo->addItem("Mono from Right", 2);
    presetCombo->addItem("Mono (L+R mix) to Stereo", 3);
    presetCombo->addItem("Left to Both", 4);
    presetCombo->addItem("Right to Both", 5);
    presetCombo->addItem("Stereo to 5.1 (front)", 6);
    presetCombo->addItem("5.1 to Stereo (downmix)", 7);

    presetLayout->addWidget(presetCombo);
    mainLayout->addWidget(presetGroup);

    // Channel map string
    auto mapGroup = new QGroupBox("Channel Mapping");
    auto mapLayout = new QVBoxLayout(mapGroup);

    mapEdit = new QLineEdit();
    mapEdit->setText(m_map);
    mapEdit->setPlaceholderText("e.g., 1|0 or FR|FL or 0|0");

    auto mapHint = new QLabel(
        "Format: out0|out1|out2... where each is input channel index or name.\n"
        "Examples:\n"
        "  • 1|0 — swap left and right\n"
        "  • 0|0 — duplicate left to both channels\n"
        "  • FL|FR|FL|FR|FC|LFE — stereo to 5.1"
    );
    mapHint->setStyleSheet("color: gray; font-size: 11px;");
    mapLayout->addWidget(mapEdit);
    mapLayout->addWidget(mapHint);
    mainLayout->addWidget(mapGroup);

    // Output channel layout
    auto layoutGroup = new QGroupBox("Output Channel Layout");
    auto layoutLayout = new QVBoxLayout(layoutGroup);

    layoutCombo = new QComboBox();
    layoutCombo->addItem("Mono", "mono");
    layoutCombo->addItem("Stereo", "stereo");
    layoutCombo->addItem("2.1", "2.1");
    layoutCombo->addItem("3.0", "3.0");
    layoutCombo->addItem("4.0", "quad");
    layoutCombo->addItem("5.0", "5.0");
    layoutCombo->addItem("5.1", "5.1");
    layoutCombo->addItem("6.1", "6.1");
    layoutCombo->addItem("7.1", "7.1");
    
    // Set current index based on m_channelLayout
    int layoutIndex = layoutCombo->findData(m_channelLayout);
    if (layoutIndex >= 0) layoutCombo->setCurrentIndex(layoutIndex);
    else layoutCombo->setCurrentIndex(1); // default stereo

    auto layoutHint = new QLabel("Must match the number of channels in the mapping");
    layoutHint->setStyleSheet("color: gray; font-size: 11px;");
    layoutLayout->addWidget(layoutCombo);
    layoutLayout->addWidget(layoutHint);
    mainLayout->addWidget(layoutGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        int presetId = presetCombo->itemData(index).toInt();
        if (presetId >= 0) {
            applyPreset(presetId);
        }
    });

    connect(mapEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_map = text;
        presetCombo->blockSignals(true);
        presetCombo->setCurrentIndex(0); // Custom
        presetCombo->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(layoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_channelLayout = layoutCombo->itemData(index).toString();
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFChannelmap::applyPreset(int index) {
    switch (index) {
        case 0: // Swap L/R
            m_map = "1|0";
            m_channelLayout = "stereo";
            break;
        case 1: // Mono from Left
            m_map = "0";
            m_channelLayout = "mono";
            break;
        case 2: // Mono from Right
            m_map = "1";
            m_channelLayout = "mono";
            break;
        case 3: // Mono mix to Stereo (needs amerge, so just duplicate)
            m_map = "0|0";
            m_channelLayout = "stereo";
            break;
        case 4: // Left to Both
            m_map = "0|0";
            m_channelLayout = "stereo";
            break;
        case 5: // Right to Both
            m_map = "1|1";
            m_channelLayout = "stereo";
            break;
        case 6: // Stereo to 5.1
            m_map = "FL|FR|FL|FR|FC|LFE";
            m_channelLayout = "5.1";
            break;
        case 7: // 5.1 to Stereo
            m_map = "FL|FR";
            m_channelLayout = "stereo";
            break;
        default:
            return;
    }

    mapEdit->blockSignals(true);
    mapEdit->setText(m_map);
    mapEdit->blockSignals(false);

    int layoutIndex = layoutCombo->findData(m_channelLayout);
    if (layoutIndex >= 0) {
        layoutCombo->blockSignals(true);
        layoutCombo->setCurrentIndex(layoutIndex);
        layoutCombo->blockSignals(false);
    }

    updateFFmpegFlags();
}

void FFChannelmap::updateFFmpegFlags() {
    ffmpegFlags = QString("channelmap=map=%1:channel_layout=%2")
                      .arg(m_map)
                      .arg(m_channelLayout);
}

QString FFChannelmap::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFChannelmap::toJSON(QJsonObject& json) const {
    json["type"] = "ff-channelmap";
    json["map"] = m_map;
    json["channelLayout"] = m_channelLayout;
}

void FFChannelmap::fromJSON(const QJsonObject& json) {
    m_map = json["map"].toString("0|1");
    m_channelLayout = json["channelLayout"].toString("stereo");
    updateFFmpegFlags();
}
