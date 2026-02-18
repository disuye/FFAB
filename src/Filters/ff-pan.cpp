#include "ff-pan.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QPushButton>

FFPan::FFPan() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFPan::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Channel Remix / Pan");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Remix input channels to output channels with custom gain coefficients. "
        "Supports downmixing, upmixing, channel swapping, and arbitrary mixing matrices."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Output layout
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

    mainLayout->addWidget(layoutGroup);

    // Presets
    auto presetGroup = new QGroupBox("Presets");
    auto presetLayout = new QVBoxLayout(presetGroup);

    presetCombo = new QComboBox();
    presetCombo->addItem("(Custom)", -1);
    presetCombo->addItem("Pass-through (stereo)", 0);
    presetCombo->addItem("Swap L/R", 1);
    presetCombo->addItem("Mono from Left", 2);
    presetCombo->addItem("Mono from Right", 3);
    presetCombo->addItem("Mono (sum L+R)", 4);
    presetCombo->addItem("Stereo to Mono (centered)", 5);
    presetCombo->addItem("5.1 to Stereo (downmix)", 6);
    presetCombo->addItem("Center channel extract", 7);
    presetCombo->addItem("Side channels only (L-R)", 8);
    presetCombo->addItem("Mid channel only (L+R)", 9);
    presetLayout->addWidget(presetCombo);

    mainLayout->addWidget(presetGroup);

    // Channel mapping
    auto mapGroup = new QGroupBox("Channel Mapping");
    auto mapLayout = new QVBoxLayout(mapGroup);

    channelMapEdit = new QTextEdit();
    channelMapEdit->setPlaceholderText("c0=c0|c1=c1");
    channelMapEdit->setText(m_channelMap);
    channelMapEdit->setMaximumHeight(80);
    mapLayout->addWidget(channelMapEdit);

    auto mapHint = new QLabel(
        "Format: c<out>=<gain>*<in>+<gain>*<in>|c<out>=...\n"
        "Examples:\n"
        "  c0=0.5*c0+0.5*c1  (mix L+R to output 0)\n"
        "  c0=c1|c1=c0  (swap channels)\n"
        "  c0=FL|c1=FR  (use channel names)"
    );
    mapHint->setStyleSheet("color: gray; font-size: 11px;");
    mapHint->setWordWrap(true);
    mapLayout->addWidget(mapHint);

    mainLayout->addWidget(mapGroup);

    // Channel names reference
    auto refGroup = new QGroupBox("Channel Names Reference");
    auto refLayout = new QVBoxLayout(refGroup);

    auto refLabel = new QLabel(
        "FL=Front Left, FR=Front Right, FC=Front Center\n"
        "LFE=Subwoofer, BL=Back Left, BR=Back Right\n"
        "SL=Side Left, SR=Side Right, BC=Back Center\n"
        "c0, c1, c2... = channels by index"
    );
    refLabel->setStyleSheet("font-size: 11px;");
    refLayout->addWidget(refLabel);

    mainLayout->addWidget(refGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Layout
    connect(layoutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_outputLayout = layoutCombo->itemData(index).toString();
        updateFFmpegFlags();
    });

    // Presets
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        if (index > 0) {
            applyPreset(presetCombo->itemData(index).toInt());
        }
    });

    // Channel map
    connect(channelMapEdit, &QTextEdit::textChanged, [this]() {
        m_channelMap = channelMapEdit->toPlainText().simplified();
        presetCombo->blockSignals(true);
        presetCombo->setCurrentIndex(0); // Custom
        presetCombo->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFPan::applyPreset(int presetIndex) {
    QString layout;
    QString map;
    
    switch (presetIndex) {
        case 0: // Pass-through stereo
            layout = "stereo";
            map = "c0=c0|c1=c1";
            break;
        case 1: // Swap L/R
            layout = "stereo";
            map = "c0=c1|c1=c0";
            break;
        case 2: // Mono from Left
            layout = "mono";
            map = "c0=c0";
            break;
        case 3: // Mono from Right
            layout = "mono";
            map = "c0=c1";
            break;
        case 4: // Mono sum
            layout = "mono";
            map = "c0=0.5*c0+0.5*c1";
            break;
        case 5: // Stereo to mono centered
            layout = "stereo";
            map = "c0=0.5*c0+0.5*c1|c1=0.5*c0+0.5*c1";
            break;
        case 6: // 5.1 to stereo downmix
            layout = "stereo";
            map = "c0=0.5*FL+0.35*FC+0.35*BL|c1=0.5*FR+0.35*FC+0.35*BR";
            break;
        case 7: // Center extract (from stereo - what's common to both)
            layout = "mono";
            map = "c0=0.5*c0+0.5*c1";
            break;
        case 8: // Side only (L-R difference)
            layout = "stereo";
            map = "c0=c0-c1|c1=c1-c0";
            break;
        case 9: // Mid only (L+R sum)
            layout = "stereo";
            map = "c0=0.5*c0+0.5*c1|c1=0.5*c0+0.5*c1";
            break;
        default:
            return;
    }
    
    m_outputLayout = layout;
    m_channelMap = map;
    
    // Update UI
    int layoutIndex = layoutCombo->findData(layout);
    if (layoutIndex >= 0) {
        layoutCombo->blockSignals(true);
        layoutCombo->setCurrentIndex(layoutIndex);
        layoutCombo->blockSignals(false);
    }
    
    channelMapEdit->blockSignals(true);
    channelMapEdit->setText(map);
    channelMapEdit->blockSignals(false);
    
    updateFFmpegFlags();
}

void FFPan::updateFFmpegFlags() {
    // Format: pan=<layout>|<mapping>
    ffmpegFlags = QString("pan=%1|%2")
                      .arg(m_outputLayout)
                      .arg(m_channelMap);
}

QString FFPan::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFPan::toJSON(QJsonObject& json) const {
    json["type"] = "ff-pan";
    json["output_layout"] = m_outputLayout;
    json["channel_map"] = m_channelMap;
}

void FFPan::fromJSON(const QJsonObject& json) {
    m_outputLayout = json["output_layout"].toString("stereo");
    m_channelMap = json["channel_map"].toString("c0=c0|c1=c1");
    updateFFmpegFlags();
}
