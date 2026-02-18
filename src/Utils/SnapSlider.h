#pragma once
#include <QSlider>
#include <QMouseEvent>

class SnapSlider : public QSlider {
    Q_OBJECT
public:
    explicit SnapSlider(Qt::Orientation orientation, QWidget* parent = nullptr)
        : QSlider(orientation, parent) {}
    
    void setSnapValue(int value) { m_snapValue = value; }
    void setSnapThreshold(int threshold) { m_snapThreshold = threshold; }

protected:
    void mouseReleaseEvent(QMouseEvent* event) override {
        QSlider::mouseReleaseEvent(event);
        // Snap on release
        if (qAbs(value() - m_snapValue) <= m_snapThreshold) {
            setValue(m_snapValue);
        }
    }
    
    void mouseMoveEvent(QMouseEvent* event) override {
        // Calculate what value the mouse position represents
        int val;
        if (orientation() == Qt::Vertical) {
            double ratio = 1.0 - (double(event->position().y()) / height());
            val = minimum() + ratio * (maximum() - minimum());
        } else {
            double ratio = double(event->position().x()) / width();
            val = minimum() + ratio * (maximum() - minimum());
        }
        
        // If within snap zone, lock to snap value
        if (qAbs(val - m_snapValue) <= m_snapThreshold) {
            setValue(m_snapValue);
        } else {
            QSlider::mouseMoveEvent(event);
        }
    }

private:
    int m_snapValue = 0;
    int m_snapThreshold = 4;
};