#include "CompandCurveWidget.h"
#include "CompandBandData.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <cmath>

CompandCurveWidget::CompandCurveWidget(CompandBandData* data, QWidget* parent)
    : QWidget(parent), m_data(data)
{
    setMinimumSize(300, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

// ==================== COORDINATE CONVERSION ====================

QPointF CompandCurveWidget::dbToPixel(double inputDb, double outputDb) const {
    double x = (inputDb - DB_MIN) / (DB_MAX - DB_MIN) * width();
    double y = (1.0 - (outputDb - DB_MIN) / (DB_MAX - DB_MIN)) * height();
    return QPointF(x, y);
}

double CompandCurveWidget::pixelToInputDb(double x) const {
    double ratio = qBound(0.0, x / width(), 1.0);
    return DB_MIN + ratio * (DB_MAX - DB_MIN);
}

double CompandCurveWidget::pixelToOutputDb(double y) const {
    double ratio = 1.0 - qBound(0.0, y / height(), 1.0);
    return DB_MIN + ratio * (DB_MAX - DB_MIN);
}

// ==================== TRANSFER FUNCTION EVALUATION ====================

double CompandCurveWidget::evaluateTransferFunction(
    double inputDb, const QList<CompandPoint>& points, double knee) const
{
    if (points.isEmpty()) return inputDb;
    if (points.size() == 1) return points[0].outputDb;

    // Check interior breakpoints for knee zones first.
    // At each breakpoint where slope s1 meets slope s2, the quadratic knee is:
    //   y = y_left(x) + (s2 - s1) * (x - kneeStart)^2 / (2 * kneeWidth)
    // This produces a single smooth parabola matching the incoming segment's
    // value+slope at kneeStart and the outgoing segment's value+slope at kneeEnd.
    if (knee > 0.1) {
        double halfKnee = knee / 2.0;

        for (int i = 1; i < points.size() - 1; ++i) {
            double x0 = points[i].inputDb;

            // Compute max symmetric knee extent (the formula requires symmetry).
            // Clamp to half the distance to each neighbor so knees don't overlap.
            double maxLeft = qMin(halfKnee, (x0 - points[i-1].inputDb) / 2.0);
            double maxRight = qMin(halfKnee, (points[i+1].inputDb - x0) / 2.0);
            double actualHalf = qMax(0.0, qMin(maxLeft, maxRight));

            double kneeLeft = x0 - actualHalf;
            double kneeRight = x0 + actualHalf;

            if (inputDb >= kneeLeft && inputDb <= kneeRight) {
                double actualKnee = 2.0 * actualHalf;
                if (actualKnee < 0.01) break;

                // Slopes of the two segments meeting at this breakpoint
                double s1 = (points[i].outputDb - points[i-1].outputDb)
                           / (points[i].inputDb - points[i-1].inputDb);
                double s2 = (points[i+1].outputDb - points[i].outputDb)
                           / (points[i+1].inputDb - points[i].inputDb);

                // Incoming segment extrapolated to inputDb
                double yLeft = points[i].outputDb + s1 * (inputDb - x0);

                // Quadratic transition from s1 to s2
                double dx = inputDb - kneeLeft;
                return yLeft + (s2 - s1) * dx * dx / (2.0 * actualKnee);
            }
        }
    }

    // Below first point: flat at first point's output (matches FFmpeg behavior)
    if (inputDb <= points.first().inputDb) {
        return points[0].outputDb;
    }

    // Above last point: extrapolate from last segment
    if (inputDb >= points.last().inputDb) {
        int n = points.size();
        if (n >= 2) {
            double slope = (points[n-1].outputDb - points[n-2].outputDb)
                         / (points[n-1].inputDb - points[n-2].inputDb);
            return points[n-1].outputDb + slope * (inputDb - points[n-1].inputDb);
        }
        return points.last().outputDb;
    }

    // Between points: piecewise linear interpolation
    for (int i = 0; i < points.size() - 1; ++i) {
        if (inputDb >= points[i].inputDb && inputDb <= points[i+1].inputDb) {
            double segLen = points[i+1].inputDb - points[i].inputDb;
            if (segLen < 1e-10) return points[i].outputDb;

            double t = (inputDb - points[i].inputDb) / segLen;
            return points[i].outputDb + t * (points[i+1].outputDb - points[i].outputDb);
        }
    }

    return inputDb; // fallback
}

// ==================== PAINTING ====================

void CompandCurveWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(30, 30, 35));

    drawGrid(painter);
    drawUnityLine(painter);
    drawCurve(painter);
    drawNodes(painter);
}

void CompandCurveWidget::drawGrid(QPainter& painter) {
    painter.setPen(QPen(QColor(60, 60, 70), 1));

    // Vertical lines (input dB) every 10dB
    for (int db = -90; db <= 0; db += 10) {
        QPointF p = dbToPixel(db, 0);
        painter.drawLine(QPointF(p.x(), 0), QPointF(p.x(), height()));

        painter.setPen(QColor(100, 100, 110));
        QFont font = painter.font();
        font.setPixelSize(9);
        painter.setFont(font);
        painter.drawText(QPointF(p.x() + 2, height() - 4), QString::number(db));
        painter.setPen(QPen(QColor(60, 60, 70), 1));
    }

    // Horizontal lines (output dB) every 10dB
    for (int db = -90; db <= 0; db += 10) {
        QPointF p = dbToPixel(0, db);

        if (db == 0) {
            painter.setPen(QPen(QColor(80, 80, 90), 1));
        } else {
            painter.setPen(QPen(QColor(60, 60, 70), 1));
        }
        painter.drawLine(QPointF(0, p.y()), QPointF(width(), p.y()));

        painter.setPen(QColor(100, 100, 110));
        QString label = (db > 0) ? QString("+%1").arg(db) : QString::number(db);
        painter.drawText(QPointF(4, p.y() - 2), label);
    }
}

void CompandCurveWidget::drawUnityLine(QPainter& painter) {
    painter.setPen(QPen(QColor(80, 80, 60), 1, Qt::DashLine));
    QPointF start = dbToPixel(DB_MIN, DB_MIN);
    QPointF end = dbToPixel(DB_MAX, DB_MAX);
    painter.drawLine(start, end);
}

void CompandCurveWidget::drawCurve(QPainter& painter) {
    const QList<CompandPoint>& points = m_data->points;
    if (points.size() < 2) return;

    double knee = m_data->softKnee;

    QPainterPath curvePath;
    QPainterPath fillPath;

    fillPath.moveTo(QPointF(0, height())); // bottom-left corner

    bool first = true;
    for (int px = 0; px <= width(); px += 2) {
        double inputDb = pixelToInputDb(px);
        double outputDb = evaluateTransferFunction(inputDb, points, knee);
        QPointF pt = dbToPixel(inputDb, outputDb);

        if (first) {
            curvePath.moveTo(pt);
            first = false;
        } else {
            curvePath.lineTo(pt);
        }
        fillPath.lineTo(pt);
    }

    // Close fill path along bottom edge
    fillPath.lineTo(QPointF(width(), height()));
    fillPath.lineTo(QPointF(0, height()));

    // Fill under curve
    QColor fillColor(100, 180, 255);
    fillColor.setAlphaF(0.10);
    painter.fillPath(fillPath, fillColor);

    // Stroke curve
    painter.strokePath(curvePath, QPen(QColor(100, 180, 255), 2));
}

void CompandCurveWidget::drawNodes(QPainter& painter) {
    const QList<CompandPoint>& points = m_data->points;

    for (int i = 0; i < points.size(); ++i) {
        QPointF center = dbToPixel(points[i].inputDb, points[i].outputDb);

        // Highlight dragged node
        if (m_dragMode == DragMode::MoveNode && m_dragNode == i) {
            painter.setPen(QPen(Qt::white, 2));
            painter.setBrush(QColor(100, 180, 255));
        } else {
            painter.setPen(QPen(QColor(200, 200, 210), 2));
            painter.setBrush(QColor(80, 150, 220));
        }

        painter.drawEllipse(center, NODE_RADIUS, NODE_RADIUS);

        // Knee indicator for interior points
        if (m_data->softKnee > 0.5) {
            drawKneeIndicator(painter, i);
        }
    }
}

void CompandCurveWidget::drawKneeIndicator(QPainter& painter, int nodeIndex) {
    const QList<CompandPoint>& points = m_data->points;
    // Only show on interior points (not first or last)
    if (nodeIndex <= 0 || nodeIndex >= points.size() - 1) return;

    double halfKnee = m_data->softKnee / 2.0;
    double x0 = points[nodeIndex].inputDb;

    // Match the symmetric clamping from evaluateTransferFunction
    double maxLeft = qMin(halfKnee, (x0 - points[nodeIndex-1].inputDb) / 2.0);
    double maxRight = qMin(halfKnee, (points[nodeIndex+1].inputDb - x0) / 2.0);
    double actualHalf = qMax(0.0, qMin(maxLeft, maxRight));

    QPointF leftPt = dbToPixel(x0 - actualHalf, points[nodeIndex].outputDb);
    QPointF rightPt = dbToPixel(x0 + actualHalf, points[nodeIndex].outputDb);

    painter.setPen(QPen(QColor(255, 200, 100, 100), 1, Qt::DashLine));
    painter.drawLine(QPointF(leftPt.x(), leftPt.y() - 8),
                     QPointF(leftPt.x(), leftPt.y() + 8));
    painter.drawLine(QPointF(rightPt.x(), rightPt.y() - 8),
                     QPointF(rightPt.x(), rightPt.y() + 8));
}

// ==================== HIT TESTING ====================

int CompandCurveWidget::hitTestNode(const QPoint& pos) const {
    const QList<CompandPoint>& points = m_data->points;

    for (int i = points.size() - 1; i >= 0; --i) {
        QPointF nodePos = dbToPixel(points[i].inputDb, points[i].outputDb);
        double dist = QLineF(QPointF(pos), nodePos).length();
        if (dist <= HIT_RADIUS) {
            return i;
        }
    }
    return -1;
}

// ==================== MOUSE INTERACTION ====================

void CompandCurveWidget::mousePressEvent(QMouseEvent* event) {
    int hitNode = hitTestNode(event->pos());
    Qt::KeyboardModifiers mods = event->modifiers();

    if (event->button() == Qt::LeftButton) {

        // Alt+Click on node = Delete node
        if ((mods & Qt::AltModifier) && hitNode >= 0) {
            m_data->removePoint(hitNode);
            emit dataChanged();
            update();
            return;
        }

        // Ctrl+Click on node = Start knee adjustment
        if ((mods & Qt::ControlModifier) && hitNode >= 0) {
            m_dragMode = DragMode::AdjustKnee;
            m_dragNode = hitNode;
            m_dragStart = event->pos();
            m_dragStartKnee = m_data->softKnee;
            return;
        }

        // Click on node = Start drag move
        if (hitNode >= 0) {
            m_dragMode = DragMode::MoveNode;
            m_dragNode = hitNode;
            m_dragStart = event->pos();
            return;
        }

        // Alt+Click on empty area = Add new point
        if (mods & Qt::AltModifier) {
            double inputDb = pixelToInputDb(event->pos().x());
            double outputDb = pixelToOutputDb(event->pos().y());
            m_data->addPoint(inputDb, outputDb);
            emit dataChanged();
            update();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}

void CompandCurveWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragMode == DragMode::MoveNode && m_dragNode >= 0) {
        double inputDb = qBound(DB_MIN, pixelToInputDb(event->pos().x()), DB_MAX);
        double outputDb = qBound(DB_MIN, pixelToOutputDb(event->pos().y()), DB_MAX);

        m_dragNode = m_data->updatePoint(m_dragNode, inputDb, outputDb);
        emit dataChanged();
        update();
    }
    else if (m_dragMode == DragMode::AdjustKnee && m_dragNode >= 0) {
        int deltaY = m_dragStart.y() - event->pos().y();
        double newKnee = qBound(0.01, m_dragStartKnee + deltaY * 0.2, 48.0);
        m_data->softKnee = newKnee;
        emit dataChanged();
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void CompandCurveWidget::mouseReleaseEvent(QMouseEvent* event) {
    m_dragMode = DragMode::None;
    m_dragNode = -1;
    QWidget::mouseReleaseEvent(event);
}
