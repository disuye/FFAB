#pragma once

#include <QWidget>
#include <QList>

class ChannelEqFilter;
struct EqNode;

class ChannelEqWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChannelEqWidget(ChannelEqFilter* filter, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Coordinate conversion (log frequency scale)
    QPointF nodeToPixel(const EqNode& node) const;
    QPointF freqGainToPixel(double freq, double gain) const;
    double pixelToFreq(double x) const;
    double pixelToGain(double y) const;
    
    // Hit testing
    int hitTestNode(const QPoint& pos, int* outChannel = nullptr) const;
    
    // Drawing helpers
    void drawGrid(QPainter& painter);
    void drawCurve(QPainter& painter, int channelIndex);
    void drawNodes(QPainter& painter, int channelIndex);
    void drawNodeShape(QPainter& painter, const QPointF& center, int filterType, 
                       const QColor& fillColor, const QColor& borderColor, int nodeId);
    void drawBandwidthIndicator(QPainter& painter, const EqNode& node, const QColor& color);
    
    // Accurate frequency response calculation
    double calculateResponse(int channelIndex, double freq) const;
    double calculateBandResponse(const EqNode& node, double freq) const;
    void calculateBiquadCoefficients(const EqNode& node, double sampleRate,
                                     double& b0, double& b1, double& b2,
                                     double& a0, double& a1, double& a2) const;
    double evaluateBiquadResponse(double freq, double sampleRate,
                                  double b0, double b1, double b2,
                                  double a0, double a1, double a2) const;
    
    // Interaction state
    enum class DragMode { None, MoveNode, AdjustBandwidth };
    DragMode m_dragMode = DragMode::None;
    int m_dragChannel = -1;
    int m_dragNode = -1;
    QPoint m_dragStart;
    double m_dragStartBandwidth = 0;
    
    // Visual constants
    static constexpr double FREQ_MIN = 20.0;
    static constexpr double FREQ_MAX = 20000.0;
    static constexpr double GAIN_MIN = -30.0;
    static constexpr double GAIN_MAX = 30.0;
    static constexpr int NODE_RADIUS = 10;
    static constexpr int HIT_RADIUS = 14;
    static constexpr double SAMPLE_RATE = 48000.0;  // For response calculation
    
    // Opacity levels
    static constexpr double OPACITY_SELECTED = 0.85;
    static constexpr double OPACITY_DIMMED = 0.40;
    
    ChannelEqFilter* m_filter;
};
