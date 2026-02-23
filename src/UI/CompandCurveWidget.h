#pragma once

#include <QWidget>
#include <QList>

struct CompandPoint;
struct CompandBandData;

class CompandCurveWidget : public QWidget {
    Q_OBJECT
public:
    explicit CompandCurveWidget(CompandBandData* data, QWidget* parent = nullptr);

signals:
    void dataChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // Coordinate conversion (linear dB on both axes)
    QPointF dbToPixel(double inputDb, double outputDb) const;
    double pixelToInputDb(double x) const;
    double pixelToOutputDb(double y) const;

    // Hit testing
    int hitTestNode(const QPoint& pos) const;

    // Drawing helpers
    void drawGrid(QPainter& painter);
    void drawUnityLine(QPainter& painter);
    void drawCurve(QPainter& painter);
    void drawNodes(QPainter& painter);
    void drawKneeIndicator(QPainter& painter, int nodeIndex);

    // Transfer function evaluation with soft-knee smoothing
    double evaluateTransferFunction(double inputDb,
                                    const QList<CompandPoint>& points,
                                    double knee) const;

    // Interaction state
    enum class DragMode { None, MoveNode, AdjustKnee };
    DragMode m_dragMode = DragMode::None;
    int m_dragNode = -1;
    QPoint m_dragStart;
    double m_dragStartKnee = 0.0;

    // Visual constants
    static constexpr double DB_MIN = -90.0;
    static constexpr double DB_MAX = 6.0;
    static constexpr int NODE_RADIUS = 8;
    static constexpr int HIT_RADIUS = 12;

    CompandBandData* m_data;
};
