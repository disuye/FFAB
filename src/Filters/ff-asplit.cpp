#include "ff-asplit.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QJsonArray>
#include <cmath>

AsplitFilter::AsplitFilter() {
    position = Position::MIDDLE;
}

QWidget* AsplitFilter::getParametersWidget() {
    auto* widget = new QWidget();
    widget = new QWidget();
    // Target for global styling main.cpp
    // parametersWidget->setObjectName("filterParamsPaneTarget");
    widget->setObjectName("ffAsplitTarget");
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(8, 8, 8, 8);
    
    // Number of splits parameter
    auto* splitsLayout = new QHBoxLayout();
    auto* label = new QLabel("Number of splits: THRU + ");
    spinBox = new QSpinBox();
    spinBox->setRange(1, 7);  // Min 1, Max 7 splits + THRU
    spinBox->setValue(m_numSplits);
    spinBox->setToolTip("Split audio into N parallel streams");
    
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &AsplitFilter::setNumSplits);
    
    splitsLayout->addWidget(label);
    splitsLayout->addWidget(spinBox);
    splitsLayout->addStretch();
    layout->addLayout(splitsLayout);
    
    // Equal power checkbox
    equalPowerCheckbox = new QCheckBox("Equal power summing");
    equalPowerCheckbox->setChecked(m_useEqualPower);
    equalPowerCheckbox->setToolTip(
        "Equal power: weight = 1/√N (louder, console style summing)\n"
        "Linear: weight = 1/N (default, mathematical average)"
    );
    
    connect(equalPowerCheckbox, &QCheckBox::toggled, 
            this, &AsplitFilter::setUseEqualPower);
    
    layout->addWidget(equalPowerCheckbox);
    
    // Auto amix checkbox
    autoAmixCheckbox = new QCheckBox("Automatic Return to Main Chain");
    autoAmixCheckbox->setChecked(m_useAutoAmix);
    autoAmixCheckbox->setToolTip(
        "ON: Streams are automatically mixed back together (default)\n"
        "OFF: Use Custom FFmpeg or amix filters to manually combine streams\n"
        "     Main chain continues from thru stream; split outputs available as labels"
    );
    
    connect(autoAmixCheckbox, &QCheckBox::toggled, 
            this, &AsplitFilter::setUseAutoAmix);
    
    layout->addWidget(autoAmixCheckbox);
    
    // Collapsible help section at bottom
    auto helpSection = new CollapsibleHelpSection(
        "<b>What is this?</b><br>"
        "Based on ffmpeg Asplit, this filter duplicates the main chain out to 7 additional streams.<br>"
        "<br>"
        "<b>Stream routing:</b><br>"
        "• Thru (stream 0): Dry signal passes through<br>"
        "• Streams 1...7: Split streams for parallel processing<br>"
        "<br>"
        "<b>Usage:</b><br>"
        "• Click stream buttons [1]-[7] to edit each stream's filter stack<br>"
        "• ALT+click stream button to mute/unmute individual streams<br>"
        "<br>"
        "<b>Summing:</b><br>"
        "• Auto recombination mixes all streams back together<br>"
        "• Disable for manual routing via Custom FFmpeg filter"

    );
    layout->addWidget(helpSection);

    layout->addStretch();
    
    return widget;
}


void AsplitFilter::setNumSplits(int n) {
    if (n >= 1 && n <= 7 && n != m_numSplits) {
        m_numSplits = n;
        if (spinBox) {
            spinBox->blockSignals(true);
            spinBox->setValue(n);
            spinBox->blockSignals(false);
        }
        
        // Remove mutes for streams that no longer exist
        QSet<int> toRemove;
        for (int s : m_mutedStreams) {
            if (s > n) {
                toRemove.insert(s);
            }
        }
        for (int s : toRemove) {
            m_mutedStreams.remove(s);
        }
        
        emit numSplitsChanged(n);
    }
}

bool AsplitFilter::isStreamMuted(int streamIndex) const {
    return m_mutedStreams.contains(streamIndex);
}

void AsplitFilter::setStreamMuted(int streamIndex, bool muted) {
    // Validate stream index (0 = thru, 1-numSplits = parallel streams)
    if (streamIndex < 0 || streamIndex > m_numSplits) {
        return;
    }
    
    bool changed = false;
    if (muted && !m_mutedStreams.contains(streamIndex)) {
        m_mutedStreams.insert(streamIndex);
        changed = true;
    } else if (!muted && m_mutedStreams.contains(streamIndex)) {
        m_mutedStreams.remove(streamIndex);
        changed = true;
    }
    
    if (changed) {
        emit streamMuteChanged(streamIndex, muted);
    }
}

void AsplitFilter::toggleStreamMute(int streamIndex) {
    setStreamMuted(streamIndex, !isStreamMuted(streamIndex));
}

int AsplitFilter::getActiveStreamCount() const {
    // Total streams = thru (1) + numSplits
    int totalStreams = 1 + m_numSplits;
    
    // Count muted streams that are actually in valid range
    int mutedCount = 0;
    for (int s : m_mutedStreams) {
        if (s >= 0 && s <= m_numSplits) {
            mutedCount++;
        }
    }
    
    return totalStreams - mutedCount;
}

void AsplitFilter::setUseEqualPower(bool enabled) {
    if (m_useEqualPower != enabled) {
        m_useEqualPower = enabled;
        if (equalPowerCheckbox) {
            equalPowerCheckbox->blockSignals(true);
            equalPowerCheckbox->setChecked(enabled);
            equalPowerCheckbox->blockSignals(false);
        }
        emit summingModeChanged(enabled);
    }
}

void AsplitFilter::setUseAutoAmix(bool enabled) {
    if (m_useAutoAmix != enabled) {
        m_useAutoAmix = enabled;
        if (autoAmixCheckbox) {
            autoAmixCheckbox->blockSignals(true);
            autoAmixCheckbox->setChecked(enabled);
            autoAmixCheckbox->blockSignals(false);
        }
        emit autoAmixChanged(enabled);
    }
}

double AsplitFilter::getStreamWeight(int streamIndex) const {
    // Muted streams have zero weight
    if (isStreamMuted(streamIndex)) {
        return 0.0;
    }
    
    int activeCount = getActiveStreamCount();
    if (activeCount <= 0) {
        return 0.0;  // All muted - shouldn't happen but handle gracefully
    }
    
    if (m_useEqualPower) {
        // Equal power: 1/sqrt(N)
        return 1.0 / std::sqrt(static_cast<double>(activeCount));
    } else {
        // Linear: 1/N
        return 1.0 / static_cast<double>(activeCount);
    }
}

QString AsplitFilter::buildFFmpegFlags() const {
    // This filter doesn't generate FFmpeg flags directly in the simple case
    // It's handled specially in FFmpegCommandBuilder for stream routing
    // When we have sub-chains, the command builder will generate the full routing
    return "";
}

void AsplitFilter::toJSON(QJsonObject& json) const {
    json["type"] = "asplit";
    json["numSplits"] = m_numSplits;
    json["filterId"] = m_filterId;
    json["customOutputStream"] = m_useCustomOutputStream;
    json["useEqualPower"] = m_useEqualPower;
    json["useAutoAmix"] = m_useAutoAmix;
    
    // Save muted streams as array
    QJsonArray mutedArray;
    for (int s : m_mutedStreams) {
        mutedArray.append(s);
    }
    json["mutedStreams"] = mutedArray;
    
    // Serialize sub-chains (from MultiOutputFilter)
    subChainsToJSON(json);
}

void AsplitFilter::fromJSON(const QJsonObject& json) {
    if (json.contains("numSplits")) {
        setNumSplits(json["numSplits"].toInt());
    }
    if (json.contains("filterId")) {
        m_filterId = json["filterId"].toInt();
    }
    if (json.contains("customOutputStream")) {
        m_useCustomOutputStream = json["customOutputStream"].toBool();
    }
    if (json.contains("useEqualPower")) {
        m_useEqualPower = json["useEqualPower"].toBool();
    }
    if (json.contains("useAutoAmix")) {
        m_useAutoAmix = json["useAutoAmix"].toBool();
    }
    
    // Load muted streams
    m_mutedStreams.clear();
    if (json.contains("mutedStreams")) {
        QJsonArray mutedArray = json["mutedStreams"].toArray();
        for (const QJsonValue& val : mutedArray) {
            m_mutedStreams.insert(val.toInt());
        }
    }
    
    // Note: Sub-chain deserialization is handled by FilterChain::fromJSON
    // because it needs access to createFilterByType()
}
