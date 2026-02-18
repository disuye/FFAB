#ifndef ROTATEDLABEL_H
#define ROTATEDLABEL_H

#include <QLabel>
#include <QPainter>

class RotatedLabel : public QLabel {
public:
    explicit RotatedLabel(const QString& text, QWidget* parent = nullptr)
        : QLabel(text, parent) {}
    
    void setBackgroundColor(const QColor& color) { m_bgColor = color; update(); }
    void setTextColor(const QColor& color) { m_textColor = color; update(); }
    void setRotation(int degrees) { m_rotation = degrees; updateGeometry(); update(); }
    void setCornerRadius(int radius) { m_radius = radius; update(); }
    void setFontSize(int size) { 
        QFont f = font();
        f.setPointSize(size);
        setFont(f);
        updateGeometry();
        update();
    }
    
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        if (m_rotation != 0) {
            painter.translate(width() / 2, height() / 2);
            painter.rotate(m_rotation);
            painter.translate(-effectiveWidth() / 2, -effectiveHeight() / 2);
        }
        
        // Draw rounded background
        painter.setBrush(m_bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(0, 0, effectiveWidth(), effectiveHeight(), m_radius, m_radius);
        
        // Draw text
        painter.setPen(m_textColor);
        painter.setFont(font());
        painter.drawText(QRect(0, 0, effectiveWidth(), effectiveHeight()), Qt::AlignCenter, text());
    }
    
    QSize sizeHint() const override {
        QFontMetrics fm(font());
        int textWidth = fm.horizontalAdvance(text()) + 40;
        int textHeight = fm.height() + 1;
        
        // Swap dimensions if rotated 90/-90
        if (m_rotation == 90 || m_rotation == -90) {
            return QSize(textHeight, textWidth);
        }
        return QSize(textWidth, textHeight);
    }
    
    QSize minimumSizeHint() const override { return sizeHint(); }
    
private:
    int effectiveWidth() const {
        if (m_rotation == 90 || m_rotation == -90) return height();
        return width();
    }
    
    int effectiveHeight() const {
        if (m_rotation == 90 || m_rotation == -90) return width();
        return height();
    }
    
    QColor m_bgColor = Qt::transparent;
    QColor m_textColor = Qt::white;
    int m_rotation = -90;  // Default: vertical
    int m_radius = 2;      // Default corner radius
};

#endif // ROTATEDLABEL_H