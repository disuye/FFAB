#pragma once

#include "BaseFilter.h"
#include <QList>
#include <QColor>

class ChannelEqWidget;

struct EqNode {
    double freq = 1000.0;      // Hz (20-20000)
    double gain = 0.0;         // dB (-30 to +30)
    double bandwidth = 100.0;  // Hz
    int filterType = 0;        // 0=Butterworth, 1=Cheby I, 2=Cheby II, 3=Elliptic
    
    bool isSelected = false;
};

struct ChannelEqData {
    QList<EqNode> nodes;
    QColor color;
    bool selected = true;  // Bright/editable when true, dimmed when false
};

class ChannelEqFilter : public BaseFilter {
    Q_OBJECT
public:
    ChannelEqFilter();
    ~ChannelEqFilter() override = default;

    QString displayName() const override { return "Channel EQ"; }
    QString filterType() const override { return "channel-eq"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    QString getDefaultCustomCommandTemplate() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    // Channel management
    int channelCount() const { return m_channels.size(); }
    ChannelEqData& channel(int index) { return m_channels[index]; }
    const ChannelEqData& channel(int index) const { return m_channels[index]; }
    
    // Node operations (called by widget)
    void addNode(int channelIndex, double freq, double gain);
    void removeNode(int channelIndex, int nodeIndex);
    void updateNode(int channelIndex, int nodeIndex, const EqNode& node);
    
    bool isLinked() const { return m_linked; }
    void setLinked(bool linked) { m_linked = linked; }

    static constexpr int MAX_NODES_PER_CHANNEL = 8;

signals:
    void parametersChanged();

private:
    void updateFFmpegFlags();
    void ensureStereoChannels();

    QList<ChannelEqData> m_channels;
    bool m_linked = true;  // Start linked for convenience
    
    QWidget* parametersWidget = nullptr;
    ChannelEqWidget* eqWidget = nullptr;
    QString ffmpegFlags;
};
