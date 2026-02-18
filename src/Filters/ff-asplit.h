#pragma once

#include "MultiOutputFilter.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QSet>

/**
 * AsplitFilter - Audio Stream Splitter
 * 
 * Splits one audio stream into multiple output streams for parallel processing.
 * Each output stream can have its own filter chain (sub-chain) for independent processing.
 * 
 * FFmpeg filter: asplit
 * Example: [0:a]asplit=3[s1][s2][s3]
 * 
 * Inherits from MultiOutputFilter:
 * - Owns sub-chains internally (no more position-based ownership)
 * - Streams 1-7 can each have filters assigned
 * - Main chain (stream 0 / "thru") continues in the FilterChain
 * 
 * Stream Muting:
 * - Individual streams can be muted (ALT+click on stream button)
 * - Muted streams contribute weight=0 to the final amix
 * - Remaining streams have weights recalculated to maintain level
 * 
 * Summing Modes:
 * - Equal Power (default): weight = 1/sqrt(activeStreams) - maintains constant power
 * - Linear: weight = 1/activeStreams - simple average
 * 
 * UI: Custom row widget with inline stream selector buttons [1][2][3][4][5][6][7]
 */
class AsplitFilter : public MultiOutputFilter {
    Q_OBJECT
    
public:
    AsplitFilter();
    
    QString displayName() const override { return "Audio Split"; }
    QString filterType() const override { return "asplit"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;
    
    // MultiOutputFilter interface
    int getNumOutputStreams() const override { return m_numSplits; }
    
    // Asplit-specific
    int numSplits() const { return m_numSplits; }
    void setNumSplits(int n);
    
    // Stream muting (0 = thru, 1-7 = split streams)
    bool isStreamMuted(int streamIndex) const;
    void setStreamMuted(int streamIndex, bool muted);
    void toggleStreamMute(int streamIndex);
    QSet<int> getMutedStreams() const { return m_mutedStreams; }
    int getActiveStreamCount() const;  // (numSplits + 1) - mutedStreams that are in range
    
    // Summing mode
    bool useEqualPower() const { return m_useEqualPower; }
    void setUseEqualPower(bool enabled);
    
    // Auto amix toggle
    bool useAutoAmix() const { return m_useAutoAmix; }
    void setUseAutoAmix(bool enabled);
    
    // Calculate weight for a stream based on mute state and summing mode
    double getStreamWeight(int streamIndex) const;
    
signals:
    void numSplitsChanged(int newCount);
    void streamMuteChanged(int streamIndex, bool muted);
    void summingModeChanged(bool useEqualPower);
    void autoAmixChanged(bool useAutoAmix);
    
private:
    int m_numSplits = 1;              // Default: 1 split streams + THRU
    QSet<int> m_mutedStreams;         // Which streams are muted (0 = thru, 1-7 = splits)
    bool m_useEqualPower = false;      // Equal power vs linear summing
    bool m_useAutoAmix = true;        // Auto-generate amix to recombine streams
    
    // UI elements
    QSpinBox* spinBox = nullptr;
    QCheckBox* equalPowerCheckbox = nullptr;
    QCheckBox* autoAmixCheckbox = nullptr;
};
