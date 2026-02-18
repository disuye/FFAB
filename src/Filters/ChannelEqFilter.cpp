#include "ChannelEqFilter.h"
#include "ChannelEqWidget.h"
#include "Utils/UnicodeSymbols.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ChannelEqFilter::ChannelEqFilter() {
    position = Position::MIDDLE;
    ensureStereoChannels();
    updateFFmpegFlags();
}

void ChannelEqFilter::ensureStereoChannels() {
    if (m_channels.isEmpty()) {
        // Channel 0 = Left (Orange)
        ChannelEqData left;
        left.color = QColor(177, 190, 140);  // Army Green
        left.selected = true;
        left.nodes.append({1000.0, 0.0, 100.0, 0, false});
        m_channels.append(left);
        
        // Channel 1 = Right (Blue)  
        ChannelEqData right;
        right.color = QColor(151, 151, 190);  // Urban Purple
        right.selected = true;
        right.nodes.append({1000.0, 0.0, 100.0, 0, false});
        m_channels.append(right);
    }
}

QWidget* ChannelEqFilter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto* mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // Toolbar
    auto* toolbar = new QHBoxLayout();
    
    auto* linkCheck = new QCheckBox("Link Channels");
    linkCheck->setChecked(m_linked);
    connect(linkCheck, &QCheckBox::toggled, this, &ChannelEqFilter::setLinked);
    toolbar->addWidget(linkCheck);
    
    toolbar->addStretch();
    
    // Channel selection toggles (bright/dim, not show/hide)
    for (int ch = 0; ch < m_channels.size(); ++ch) {
        auto* chBtn = new QPushButton(ch == 0 ? "L" : "R");
        chBtn->setCheckable(true);
        chBtn->setChecked(m_channels[ch].selected);
        chBtn->setFixedSize(28, 28);
        chBtn->setToolTip(ch == 0 ? "Left channel (select to edit)" : "Right channel (select to edit)");
        
        // Style: checked = bright colored, unchecked = dimmed
        QString baseColor = m_channels[ch].color.name();
        chBtn->setStyleSheet(QString(
            "QPushButton { background: %1; color: white; border-radius: 4px; font-weight: bold; } "
            "QPushButton:checked { background: %1; border: 2px solid white; } "
            "QPushButton:!checked { background: %2; border: none; }"
        ).arg(baseColor).arg(QColor(m_channels[ch].color).darker(200).name()));
        
        connect(chBtn, &QPushButton::toggled, [this, ch](bool selected) {
            m_channels[ch].selected = selected;
            if (eqWidget) eqWidget->update();
        });
        toolbar->addWidget(chBtn);
    }
    
    mainLayout->addLayout(toolbar);

    // EQ curve widget
    eqWidget = new ChannelEqWidget(this);
    eqWidget->setMinimumHeight(200);
    mainLayout->addWidget(eqWidget, 1);

    // Help text with shape legend
    auto* helpLabel = new QLabel(
        "<small><b>Shapes:</b> " + Sym::Stop + " Butterworth | " + Sym::TriangleUp + " Chebyshev I | " + Sym::TriangleDown + " Chebyshev II | " + Sym::Circle + " Elliptic<br>"
        "<b>Controls:</b> Alt+Click: add/delete | Drag: freq/gain | "
        "Ctrl+Drag: bandwidth | Shift+Click: filter type | Scroll: bandwidth</small>");
    helpLabel->setStyleSheet("color: #808080;");
    helpLabel->setWordWrap(true);
    mainLayout->addWidget(helpLabel);

    return parametersWidget;
}

void ChannelEqFilter::addNode(int channelIndex, double freq, double gain) {
    if (channelIndex < 0 || channelIndex >= m_channels.size()) return;
    if (m_channels[channelIndex].nodes.size() >= MAX_NODES_PER_CHANNEL) return;
    
    EqNode node;
    node.freq = freq;
    node.gain = gain;
    node.bandwidth = 100.0;
    node.filterType = 0;
    node.isSelected = false;
    m_channels[channelIndex].nodes.append(node);
    
    updateFFmpegFlags();
    emit parametersChanged();
}

void ChannelEqFilter::removeNode(int channelIndex, int nodeIndex) {
    if (channelIndex < 0 || channelIndex >= m_channels.size()) return;
    if (nodeIndex < 0 || nodeIndex >= m_channels[channelIndex].nodes.size()) return;
    
    m_channels[channelIndex].nodes.removeAt(nodeIndex);
    
    updateFFmpegFlags();
    emit parametersChanged();
}

void ChannelEqFilter::updateNode(int channelIndex, int nodeIndex, const EqNode& node) {
    if (channelIndex < 0 || channelIndex >= m_channels.size()) return;
    if (nodeIndex < 0 || nodeIndex >= m_channels[channelIndex].nodes.size()) return;
    
    m_channels[channelIndex].nodes[nodeIndex] = node;
    
    updateFFmpegFlags();
    emit parametersChanged();
}

void ChannelEqFilter::updateFFmpegFlags() {
    QStringList bandParams;
    
    for (int ch = 0; ch < m_channels.size(); ++ch) {
        for (const EqNode& node : m_channels[ch].nodes) {
            QString param = QString("c%1 f=%2 w=%3 g=%4 t=%5")
                                .arg(ch)
                                .arg(node.freq, 0, 'f', 1)
                                .arg(node.bandwidth, 0, 'f', 1)
                                .arg(node.gain, 0, 'f', 1)
                                .arg(node.filterType);
            bandParams.append(param);
        }
    }
    
    if (bandParams.isEmpty()) {
        ffmpegFlags = "anull";  // Pass-through if no bands
    } else {
        ffmpegFlags = QString("anequalizer=params='%1'").arg(bandParams.join("|"));
    }
}

QString ChannelEqFilter::buildFFmpegFlags() const {
    return ffmpegFlags;
}

QString ChannelEqFilter::getDefaultCustomCommandTemplate() const {
    return "anequalizer=params='c0 f=1000 w=100 g=0 t=0|c1 f=1000 w=100 g=0 t=0'";
}

void ChannelEqFilter::toJSON(QJsonObject& json) const {
    json["type"] = "channel-eq";
    json["linked"] = m_linked;
    
    QJsonArray channelsArray;
    for (const ChannelEqData& ch : m_channels) {
        QJsonObject chObj;
        chObj["color"] = ch.color.name();
        chObj["selected"] = ch.selected;
        
        QJsonArray nodesArray;
        for (const EqNode& node : ch.nodes) {
            QJsonObject nodeObj;
            nodeObj["freq"] = node.freq;
            nodeObj["gain"] = node.gain;
            nodeObj["bandwidth"] = node.bandwidth;
            nodeObj["filterType"] = node.filterType;
            nodesArray.append(nodeObj);
        }
        chObj["nodes"] = nodesArray;
        channelsArray.append(chObj);
    }
    json["channels"] = channelsArray;
}

void ChannelEqFilter::fromJSON(const QJsonObject& json) {
    m_linked = json["linked"].toBool(true);
    m_channels.clear();
    
    QJsonArray channelsArray = json["channels"].toArray();
    for (const QJsonValue& chVal : channelsArray) {
        QJsonObject chObj = chVal.toObject();
        
        ChannelEqData ch;
        ch.color = QColor(chObj["color"].toString("#ff9632"));
        ch.selected = chObj["selected"].toBool(true);
        
        QJsonArray nodesArray = chObj["nodes"].toArray();
        for (const QJsonValue& nodeVal : nodesArray) {
            QJsonObject nodeObj = nodeVal.toObject();
            
            EqNode node;
            node.freq = nodeObj["freq"].toDouble(1000.0);
            node.gain = nodeObj["gain"].toDouble(0.0);
            node.bandwidth = nodeObj["bandwidth"].toDouble(100.0);
            node.filterType = nodeObj["filterType"].toInt(0);
            node.isSelected = false;
            ch.nodes.append(node);
        }
        m_channels.append(ch);
    }
    
    // Ensure we have at least stereo
    if (m_channels.isEmpty()) {
        ensureStereoChannels();
    }
    
    updateFFmpegFlags();
    
    // Update UI if widget exists
    if (eqWidget) {
        eqWidget->update();
    }
}
