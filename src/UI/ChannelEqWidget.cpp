#include "ChannelEqWidget.h"
#include "ChannelEqFilter.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QtMath>
#include <cmath>

ChannelEqWidget::ChannelEqWidget(ChannelEqFilter* filter, QWidget* parent)
    : QWidget(parent), m_filter(filter)
{
    setMinimumSize(400, 250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

// ==================== COORDINATE CONVERSION ====================

QPointF ChannelEqWidget::freqGainToPixel(double freq, double gain) const {
    double logMin = std::log10(FREQ_MIN);
    double logMax = std::log10(FREQ_MAX);
    double logFreq = std::log10(qBound(FREQ_MIN, freq, FREQ_MAX));
    
    double x = (logFreq - logMin) / (logMax - logMin) * width();
    double y = (1.0 - (gain - GAIN_MIN) / (GAIN_MAX - GAIN_MIN)) * height();
    
    return QPointF(x, y);
}

QPointF ChannelEqWidget::nodeToPixel(const EqNode& node) const {
    return freqGainToPixel(node.freq, node.gain);
}

double ChannelEqWidget::pixelToFreq(double x) const {
    double logMin = std::log10(FREQ_MIN);
    double logMax = std::log10(FREQ_MAX);
    double ratio = qBound(0.0, x / width(), 1.0);
    return std::pow(10.0, logMin + ratio * (logMax - logMin));
}

double ChannelEqWidget::pixelToGain(double y) const {
    double ratio = 1.0 - qBound(0.0, y / height(), 1.0);
    return GAIN_MIN + ratio * (GAIN_MAX - GAIN_MIN);
}

// ==================== ACCURATE BIQUAD RESPONSE ====================

void ChannelEqWidget::calculateBiquadCoefficients(const EqNode& node, double sampleRate,
                                                   double& b0, double& b1, double& b2,
                                                   double& a0, double& a1, double& a2) const {
    // Peaking EQ filter coefficients based on Audio EQ Cookbook
    // Modified for different filter types
    
    double A = std::pow(10.0, node.gain / 40.0);  // sqrt of linear gain
    double w0 = 2.0 * M_PI * node.freq / sampleRate;
    double cosw0 = std::cos(w0);
    double sinw0 = std::sin(w0);
    
    // Q from bandwidth (approximation)
    double Q = node.freq / qMax(node.bandwidth, 1.0);
    Q = qBound(0.1, Q, 100.0);
    
    double alpha = sinw0 / (2.0 * Q);
    
    switch (node.filterType) {
        case 0:  // Butterworth - maximally flat
        {
            // Standard peaking EQ (Butterworth-derived)
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cosw0;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha / A;
            break;
        }
        case 1:  // Chebyshev Type I - ripple in passband, steeper rolloff
        {
            // Modify alpha for sharper response
            double ripple = 0.5;  // dB ripple
            double epsilon = std::sqrt(std::pow(10.0, ripple / 10.0) - 1.0);
            double alphaCheb = alpha * (1.0 + epsilon);
            
            b0 = 1.0 + alphaCheb * A;
            b1 = -2.0 * cosw0;
            b2 = 1.0 - alphaCheb * A;
            a0 = 1.0 + alphaCheb / A;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alphaCheb / A;
            break;
        }
        case 2:  // Chebyshev Type II - ripple in stopband
        {
            // Inverse Chebyshev characteristic
            double ripple = 0.5;
            double epsilon = 1.0 / std::sqrt(std::pow(10.0, ripple / 10.0) - 1.0);
            double alphaCheb = alpha * epsilon;
            
            b0 = 1.0 + alphaCheb * A;
            b1 = -2.0 * cosw0;
            b2 = 1.0 - alphaCheb * A;
            a0 = 1.0 + alphaCheb / A;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alphaCheb / A;
            break;
        }
        case 3:  // Elliptic - steepest rolloff, ripple in both bands
        {
            // Narrower bandwidth for sharper response (elliptic-like character)
            double alphaEllip = alpha * 0.7;  // Tighter Q
            
            b0 = 1.0 + alphaEllip * A;
            b1 = -2.0 * cosw0;
            b2 = 1.0 - alphaEllip * A;
            a0 = 1.0 + alphaEllip / A;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alphaEllip / A;
            break;
        }
        default:
            // Fallback to Butterworth
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cosw0;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cosw0;
            a2 = 1.0 - alpha / A;
            break;
    }
}

double ChannelEqWidget::evaluateBiquadResponse(double freq, double sampleRate,
                                                double b0, double b1, double b2,
                                                double a0, double a1, double a2) const {
    // Evaluate |H(e^jw)| for the biquad filter
    double w = 2.0 * M_PI * freq / sampleRate;
    double cosw = std::cos(w);
    double cos2w = std::cos(2.0 * w);
    double sinw = std::sin(w);
    double sin2w = std::sin(2.0 * w);
    
    // Numerator: B(e^jw) = b0 + b1*e^-jw + b2*e^-2jw
    double numReal = b0 + b1 * cosw + b2 * cos2w;
    double numImag = -b1 * sinw - b2 * sin2w;
    double numMag = std::sqrt(numReal * numReal + numImag * numImag);
    
    // Denominator: A(e^jw) = a0 + a1*e^-jw + a2*e^-2jw
    double denReal = a0 + a1 * cosw + a2 * cos2w;
    double denImag = -a1 * sinw - a2 * sin2w;
    double denMag = std::sqrt(denReal * denReal + denImag * denImag);
    
    if (denMag < 1e-10) denMag = 1e-10;  // Prevent division by zero
    
    double magnitude = numMag / denMag;
    
    // Convert to dB
    if (magnitude < 1e-10) magnitude = 1e-10;
    return 20.0 * std::log10(magnitude);
}

double ChannelEqWidget::calculateBandResponse(const EqNode& node, double freq) const {
    double b0, b1, b2, a0, a1, a2;
    calculateBiquadCoefficients(node, SAMPLE_RATE, b0, b1, b2, a0, a1, a2);
    return evaluateBiquadResponse(freq, SAMPLE_RATE, b0, b1, b2, a0, a1, a2);
}

double ChannelEqWidget::calculateResponse(int channelIndex, double freq) const {
    const ChannelEqData& ch = m_filter->channel(channelIndex);
    double totalGainDb = 0.0;
    
    // Sum contributions from all bands (in dB domain for cascaded filters)
    for (const EqNode& node : ch.nodes) {
        totalGainDb += calculateBandResponse(node, freq);
    }
    
    return qBound(GAIN_MIN, totalGainDb, GAIN_MAX);
}

// ==================== PAINTING ====================

void ChannelEqWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Background
    painter.fillRect(rect(), QColor(30, 30, 35));
    
    drawGrid(painter);
    
    // Draw curves and nodes for each channel
    for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
        drawCurve(painter, ch);
        drawNodes(painter, ch);
    }
}

void ChannelEqWidget::drawGrid(QPainter& painter) {
    painter.setPen(QPen(QColor(60, 60, 70), 1));
    
    // Frequency lines (logarithmic)
    QList<double> freqLines = {50, 100, 200, 500, 1000, 2000, 5000, 10000};
    for (double f : freqLines) {
        QPointF p = freqGainToPixel(f, 0);
        painter.drawLine(QPointF(p.x(), 0), QPointF(p.x(), height()));
        
        QString label = (f >= 1000) ? QString("%1k").arg(f/1000) : QString::number(static_cast<int>(f));
        painter.setPen(QColor(100, 100, 110));
        painter.drawText(QPointF(p.x() + 2, height() - 4), label);
        painter.setPen(QPen(QColor(60, 60, 70), 1));
    }
    
    // Gain lines
    for (int g = -20; g <= 20; g += 10) {
        QPointF p = freqGainToPixel(1000, g);
        
        if (g == 0) {
            painter.setPen(QPen(QColor(80, 80, 90), 1));
        } else {
            painter.setPen(QPen(QColor(60, 60, 70), 1));
        }
        painter.drawLine(QPointF(0, p.y()), QPointF(width(), p.y()));
        
        painter.setPen(QColor(100, 100, 110));
        QString label = (g > 0) ? QString("+%1").arg(g) : QString::number(g);
        painter.drawText(QPointF(4, p.y() - 2), label);
    }
}

void ChannelEqWidget::drawCurve(QPainter& painter, int channelIndex) {
    const ChannelEqData& ch = m_filter->channel(channelIndex);
    double opacity = ch.selected ? OPACITY_SELECTED : OPACITY_DIMMED;
    
    QPointF zeroLeft = freqGainToPixel(FREQ_MIN, 0);
    QPointF zeroRight = freqGainToPixel(FREQ_MAX, 0);
    
    // Build curve path and fill path simultaneously
    QPainterPath curvePath;
    QPainterPath fillPath;
    
    // Fill starts at 0dB left edge
    fillPath.moveTo(zeroLeft);
    
    bool first = true;
    for (int px = 0; px <= width(); px += 2) {
        double freq = pixelToFreq(px);
        double gain = calculateResponse(channelIndex, freq);
        QPointF pt = freqGainToPixel(freq, gain);
        
        if (first) {
            curvePath.moveTo(pt);
            first = false;
        } else {
            curvePath.lineTo(pt);
        }
        fillPath.lineTo(pt);
    }
    
    // Fill closes along 0dB line back to start
    fillPath.lineTo(zeroRight);
    fillPath.lineTo(zeroLeft);
    
    // Draw fill
    QColor fillColor = ch.color;
    fillColor.setAlphaF(opacity * 0.12);
    painter.fillPath(fillPath, fillColor);
    
    // Draw curve line
    QColor lineColor = ch.color;
    lineColor.setAlphaF(opacity);
    painter.strokePath(curvePath, QPen(lineColor, 2));
}

void ChannelEqWidget::drawNodes(QPainter& painter, int channelIndex) {
    const ChannelEqData& ch = m_filter->channel(channelIndex);
    
    // Determine opacity based on selected state
    double opacity = ch.selected ? OPACITY_SELECTED : OPACITY_DIMMED;
    
    for (int i = 0; i < ch.nodes.size(); ++i) {
        const EqNode& node = ch.nodes[i];
        QPointF center = freqGainToPixel(node.freq, node.gain);
        
        // Bandwidth indicator
        QColor bwColor = ch.color;
        bwColor.setAlphaF(opacity * 0.5);
        drawBandwidthIndicator(painter, node, bwColor);
        
        // Node colors with opacity
        QColor fillColor = ch.color;
        fillColor.setAlphaF(opacity);
        QColor borderColor = Qt::white;
        borderColor.setAlphaF(opacity);
        
        // Draw node shape based on filter type, with node ID inside
        drawNodeShape(painter, center, node.filterType, fillColor, borderColor, i + 1);
    }
}

void ChannelEqWidget::drawNodeShape(QPainter& painter, const QPointF& center, int filterType,
                                     const QColor& fillColor, const QColor& borderColor, int nodeId) {
    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(fillColor);
    
    QPainterPath shape;
    
    switch (filterType) {
        case 0:  // Butterworth - Square
        {
            double halfSize = NODE_RADIUS * 0.85;
            shape.addRect(center.x() - halfSize, center.y() - halfSize, 
                         halfSize * 2, halfSize * 2);
            break;
        }
        case 1:  // Chebyshev I - Triangle pointing UP
        {
            double size = NODE_RADIUS * 1.1;
            shape.moveTo(center.x(), center.y() - size);                    // Top
            shape.lineTo(center.x() - size, center.y() + size * 0.7);       // Bottom left
            shape.lineTo(center.x() + size, center.y() + size * 0.7);       // Bottom right
            shape.closeSubpath();
            break;
        }
        case 2:  // Chebyshev II - Triangle pointing DOWN
        {
            double size = NODE_RADIUS * 1.1;
            shape.moveTo(center.x(), center.y() + size);                    // Bottom
            shape.lineTo(center.x() - size, center.y() - size * 0.7);       // Top left
            shape.lineTo(center.x() + size, center.y() - size * 0.7);       // Top right
            shape.closeSubpath();
            break;
        }
        case 3:  // Elliptic - Circle
        {
            shape.addEllipse(center, NODE_RADIUS, NODE_RADIUS);
            break;
        }
        default:
            shape.addEllipse(center, NODE_RADIUS, NODE_RADIUS);
            break;
    }
    
    painter.drawPath(shape);
    
    // Draw node ID number inside
    QColor textColor = Qt::white;
    textColor.setAlphaF(borderColor.alphaF());
    painter.setPen(textColor);
    
    QFont font = painter.font();
    font.setPixelSize(10);
    font.setBold(true);
    painter.setFont(font);
    
    QString idText = QString::number(nodeId);
    QRectF textRect(center.x() - NODE_RADIUS, center.y() - NODE_RADIUS,
                    NODE_RADIUS * 2, NODE_RADIUS * 2);
    painter.drawText(textRect, Qt::AlignCenter, idText);
}

void ChannelEqWidget::drawBandwidthIndicator(QPainter& painter, const EqNode& node, const QColor& color) {
    double halfBW = node.bandwidth / 2.0;
    double freqLow = node.freq - halfBW;
    double freqHigh = node.freq + halfBW;
    
    QPointF center = freqGainToPixel(node.freq, node.gain);
    QPointF left = freqGainToPixel(qMax(FREQ_MIN, freqLow), node.gain);
    QPointF right = freqGainToPixel(qMin(FREQ_MAX, freqHigh), node.gain);
    
    painter.setPen(QPen(color, 1, Qt::DashLine));
    painter.drawLine(QPointF(left.x(), center.y() - 15), QPointF(left.x(), center.y() + 15));
    painter.drawLine(QPointF(right.x(), center.y() - 15), QPointF(right.x(), center.y() + 15));
    painter.drawLine(left, right);
}

// ==================== HIT TESTING ====================

int ChannelEqWidget::hitTestNode(const QPoint& pos, int* outChannel) const {
    // Only hit test on SELECTED channels
    // Check in reverse order (topmost drawn last)
    for (int ch = m_filter->channelCount() - 1; ch >= 0; --ch) {
        const ChannelEqData& chData = m_filter->channel(ch);
        
        // Skip unselected channels - they cannot be interacted with
        if (!chData.selected) continue;
        
        for (int i = 0; i < chData.nodes.size(); ++i) {
            QPointF nodePos = freqGainToPixel(chData.nodes[i].freq, chData.nodes[i].gain);
            double dist = QLineF(pos, nodePos).length();
            if (dist <= HIT_RADIUS) {
                if (outChannel) *outChannel = ch;
                return i;
            }
        }
    }
    return -1;
}

// ==================== MOUSE INTERACTION ====================

void ChannelEqWidget::mousePressEvent(QMouseEvent* event) {
    int hitChannel = -1;
    int hitNode = hitTestNode(event->pos(), &hitChannel);
    
    Qt::KeyboardModifiers mods = event->modifiers();
    
    if (event->button() == Qt::LeftButton) {
        
        // Alt+Click on node = Delete node
        if ((mods & Qt::AltModifier) && hitNode >= 0) {
            if (m_filter->isLinked()) {
                for (int ch = m_filter->channelCount() - 1; ch >= 0; --ch) {
                    if (hitNode < m_filter->channel(ch).nodes.size()) {
                        m_filter->removeNode(ch, hitNode);
                    }
                }
            } else {
                m_filter->removeNode(hitChannel, hitNode);
            }
            update();
            return;
        }
        
        // Ctrl+Click = Start bandwidth adjustment
        if ((mods & Qt::ControlModifier) && hitNode >= 0) {
            m_dragMode = DragMode::AdjustBandwidth;
            m_dragChannel = hitChannel;
            m_dragNode = hitNode;
            m_dragStart = event->pos();
            m_dragStartBandwidth = m_filter->channel(hitChannel).nodes[hitNode].bandwidth;
            return;
        }
        
        // Shift+Click = Cycle filter type
        if ((mods & Qt::ShiftModifier) && hitNode >= 0) {
            if (m_filter->isLinked()) {
                for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
                    if (hitNode < m_filter->channel(ch).nodes.size()) {
                        EqNode node = m_filter->channel(ch).nodes[hitNode];
                        node.filterType = (node.filterType + 1) % 4;
                        m_filter->updateNode(ch, hitNode, node);
                    }
                }
            } else {
                EqNode node = m_filter->channel(hitChannel).nodes[hitNode];
                node.filterType = (node.filterType + 1) % 4;
                m_filter->updateNode(hitChannel, hitNode, node);
            }
            update();
            return;
        }
        
        // Click on node = select and start drag
        if (hitNode >= 0) {
            m_dragMode = DragMode::MoveNode;
            m_dragChannel = hitChannel;
            m_dragNode = hitNode;
            m_dragStart = event->pos();
            return;
        }
        
        // Alt+Click on empty area = Add new node
        if (mods & Qt::AltModifier) {
            double freq = pixelToFreq(event->pos().x());
            double gain = pixelToGain(event->pos().y());
            
            if (m_filter->isLinked()) {
                // Add to all SELECTED channels
                for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
                    if (m_filter->channel(ch).selected &&
                        m_filter->channel(ch).nodes.size() < ChannelEqFilter::MAX_NODES_PER_CHANNEL) {
                        m_filter->addNode(ch, freq, gain);
                    }
                }
            } else {
                // Add to first selected channel only
                for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
                    if (m_filter->channel(ch).selected &&
                        m_filter->channel(ch).nodes.size() < ChannelEqFilter::MAX_NODES_PER_CHANNEL) {
                        m_filter->addNode(ch, freq, gain);
                        break;
                    }
                }
            }
            update();
            return;
        }
    }
    
    QWidget::mousePressEvent(event);
}

void ChannelEqWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragMode == DragMode::MoveNode && m_dragNode >= 0) {
        double freq = pixelToFreq(event->pos().x());
        double gain = pixelToGain(event->pos().y());
        
        freq = qBound(FREQ_MIN, freq, FREQ_MAX);
        gain = qBound(GAIN_MIN, gain, GAIN_MAX);
        
        if (m_filter->isLinked()) {
            for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
                if (m_filter->channel(ch).selected && 
                    m_dragNode < m_filter->channel(ch).nodes.size()) {
                    EqNode node = m_filter->channel(ch).nodes[m_dragNode];
                    node.freq = freq;
                    node.gain = gain;
                    m_filter->updateNode(ch, m_dragNode, node);
                }
            }
        } else {
            EqNode node = m_filter->channel(m_dragChannel).nodes[m_dragNode];
            node.freq = freq;
            node.gain = gain;
            m_filter->updateNode(m_dragChannel, m_dragNode, node);
        }
        update();
    }
    else if (m_dragMode == DragMode::AdjustBandwidth && m_dragNode >= 0) {
        int deltaY = m_dragStart.y() - event->pos().y();
        double scale = 1.0 + deltaY * 0.01;
        double newBW = qBound(10.0, m_dragStartBandwidth * scale, 5000.0);
        
        if (m_filter->isLinked()) {
            for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
                if (m_filter->channel(ch).selected &&
                    m_dragNode < m_filter->channel(ch).nodes.size()) {
                    EqNode node = m_filter->channel(ch).nodes[m_dragNode];
                    node.bandwidth = newBW;
                    m_filter->updateNode(ch, m_dragNode, node);
                }
            }
        } else {
            EqNode node = m_filter->channel(m_dragChannel).nodes[m_dragNode];
            node.bandwidth = newBW;
            m_filter->updateNode(m_dragChannel, m_dragNode, node);
        }
        update();
    }
    
    QWidget::mouseMoveEvent(event);
}

void ChannelEqWidget::mouseReleaseEvent(QMouseEvent* event) {
    m_dragMode = DragMode::None;
    m_dragChannel = -1;
    m_dragNode = -1;
    QWidget::mouseReleaseEvent(event);
}

void ChannelEqWidget::wheelEvent(QWheelEvent* event) {
    int hitChannel = -1;
    int hitNode = hitTestNode(event->position().toPoint(), &hitChannel);
    
    if (hitNode >= 0) {
        double delta = event->angleDelta().y() > 0 ? 1.1 : 0.9;
        
        if (m_filter->isLinked()) {
            for (int ch = 0; ch < m_filter->channelCount(); ++ch) {
                if (m_filter->channel(ch).selected &&
                    hitNode < m_filter->channel(ch).nodes.size()) {
                    EqNode node = m_filter->channel(ch).nodes[hitNode];
                    node.bandwidth = qBound(10.0, node.bandwidth * delta, 5000.0);
                    m_filter->updateNode(ch, hitNode, node);
                }
            }
        } else {
            EqNode node = m_filter->channel(hitChannel).nodes[hitNode];
            node.bandwidth = qBound(10.0, node.bandwidth * delta, 5000.0);
            m_filter->updateNode(hitChannel, hitNode, node);
        }
        update();
        event->accept();
        return;
    }
    
    QWidget::wheelEvent(event);
}
