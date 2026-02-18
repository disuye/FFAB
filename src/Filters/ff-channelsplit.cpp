#include "ff-channelsplit.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFChannelsplit::FFChannelsplit() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFChannelsplit::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Warning about multiple outputs
    auto warningLabel = new QLabel(
        "⚠️ This filter produces multiple output streams (one per channel).\n"
        "Use with amerge or select specific outputs in complex filter graphs."
    );
    warningLabel->setStyleSheet("color: #c90; font-size: 11px; padding: 8px; background: #fff8e0; border-radius: 4px;");
    warningLabel->setWordWrap(true);
    mainLayout->addWidget(warningLabel);

    // Input channel layout
    auto layoutGroup = new QGroupBox("Input Channel Layout");
    auto layoutLayout = new QVBoxLayout(layoutGroup);

    layoutCombo = new QComboBox();
    layoutCombo->addItem("Mono", "mono");
    layoutCombo->addItem("Stereo", "stereo");
    layoutCombo->addItem("2.1", "2.1");
    layoutCombo->addItem("3.0", "3.0");
    layoutCombo->addItem("Quad (4.0)", "quad");
    layoutCombo->addItem("5.0", "5.0");
    layoutCombo->addItem("5.1", "5.1");
    layoutCombo->addItem("6.1", "6.1");
    layoutCombo->addItem("7.1", "7.1");

    int layoutIndex = layoutCombo->findData(m_channelLayout);
    if (layoutIndex >= 0) layoutCombo->setCurrentIndex(layoutIndex);
    else layoutCombo->setCurrentIndex(1); // default stereo

    auto layoutHint = new QLabel("Specify the channel layout of the input audio");
    layoutHint->setStyleSheet("color: gray; font-size: 11px;");
    layoutLayout->addWidget(layoutCombo);
    layoutLayout->addWidget(layoutHint);
    mainLayout->addWidget(layoutGroup);

    // Channels to extract
    auto channelsGroup = new QGroupBox("Channels to Extract");
    auto channelsLayout = new QVBoxLayout(channelsGroup);

    channelsEdit = new QLineEdit();
    channelsEdit->setText(m_channels);
    channelsEdit->setPlaceholderText("all, or e.g., FL+FR or 0+1");

    auto channelsHint = new QLabel(
        "Specify which channels to extract:\n"
        "  • all — extract all channels (default)\n"
        "  • FL+FR — extract front left and front right only\n"
        "  • FC+LFE — extract center and subwoofer\n\n"
        "Channel names: FL, FR, FC, LFE, BL, BR, SL, SR"
    );
    channelsHint->setStyleSheet("color: gray; font-size: 11px;");
    channelsLayout->addWidget(channelsEdit);
    channelsLayout->addWidget(channelsHint);
    mainLayout->addWidget(channelsGroup);

    // Output info
    auto outputGroup = new QGroupBox("Output Streams");
    auto outputLayout = new QVBoxLayout(outputGroup);

    auto outputLabel = new QLabel(
        "Each extracted channel becomes a separate mono output stream.\n"
        "For stereo input with 'all': outputs [0]=Left, [1]=Right\n"
        "For 5.1 input with 'all': outputs [0]=FL, [1]=FR, [2]=FC, [3]=LFE, [4]=BL, [5]=BR"
    );
    outputLabel->setStyleSheet("color: #666; font-size: 11px;");
    outputLayout->addWidget(outputLabel);
    mainLayout->addWidget(outputGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    connect(layoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_channelLayout = layoutCombo->itemData(index).toString();
        updateFFmpegFlags();
    });

    connect(channelsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_channels = text.isEmpty() ? "all" : text;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFChannelsplit::updateFFmpegFlags() {
    if (m_channels == "all") {
        ffmpegFlags = QString("channelsplit=channel_layout=%1")
                          .arg(m_channelLayout);
    } else {
        ffmpegFlags = QString("channelsplit=channel_layout=%1:channels=%2")
                          .arg(m_channelLayout)
                          .arg(m_channels);
    }
}

QString FFChannelsplit::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFChannelsplit::toJSON(QJsonObject& json) const {
    json["type"] = "ff-channelsplit";
    json["channelLayout"] = m_channelLayout;
    json["channels"] = m_channels;
}

void FFChannelsplit::fromJSON(const QJsonObject& json) {
    m_channelLayout = json["channelLayout"].toString("stereo");
    m_channels = json["channels"].toString("all");
    updateFFmpegFlags();
}
