#pragma once
#include <QString>
#include <QIcon>
#include <QIconEngine>
#include <QPixmap>
#include <QPainter>
#include <QPalette>
#include <QApplication>
#include <QSvgRenderer>
#include <functional>

// macOS Tahoe seems to have a unicode display issue
// hence this complex approach to icons and characters

namespace Sym {

// =====================================================================
// TEXT CONSTANTS — for labels, tooltips, help text, combo items
// =====================================================================

// Transport controls
inline const QString Play         = QString(QChar(0x25B6)); // ▶ right-pointing triangle
inline const QString Stop         = QString(QChar(0x25A0)); // ■ black square

// Geometric shapes (algorithm diagrams)
inline const QString TriangleUp   = QString(QChar(0x25B2)); // ▲ up-pointing triangle
inline const QString TriangleDown = QString(QChar(0x25BC)); // ▼ down-pointing triangle
inline const QString Circle       = QString(QChar(0x25CF)); // ● black circle
inline const QString LargeCircle  = QString(QChar(0x2B24)); // ⬤ black large circle
inline const QString TriangleLeft = QString(QChar(0x25C0)); // ◀ left-pointing triangle

// Arrows
inline const QString ArrowUp     = QString(QChar(0x2191)); // ↑
inline const QString ArrowDown   = QString(QChar(0x2193)); // ↓
inline const QString ArrowLeft   = QString(QChar(0x2190)); // ←
inline const QString ArrowRight  = QString(QChar(0x2192)); // →
inline const QString ArrowNE     = QString(QChar(0x2197)); // ↗ north-east

// Action indicators
inline const QString Cross       = QString(QChar(0x2715)); // ✕ delete / close
inline const QString Check       = QString(QChar(0x2713)); // ✓ success
inline const QString BallotX     = QString(QChar(0x2717)); // ✗ failure

// Miscellaneous
inline const QString Skull       = QString(QChar(0x2620)); // ☠ chaos mode
inline const QString ForAll      = QString(QChar(0x2200)); // ∀ wildcard
inline const QString CirclePlus  = QString(QChar(0x2295)); // ⊕ zip operation
inline const QString CircleTimes = QString(QChar(0x2297)); // ⊗ broadcast operation

// =====================================================================
// SVG ICONS — adaptive to light/dark mode
// These are pure vector geometry, no font dependency.
// Color is read from QPalette at paint time, so icons update
// immediately when the OS switches between light and dark mode.
// =====================================================================

inline QByteArray makeSvg(const QByteArray& body) {
    return "<svg xmlns='http://www.w3.org/2000/svg' width='16' height='16' viewBox='0 0 16 16'>" + body + "</svg>";
}

// Icon engine that re-renders SVG with current palette colors on every paint
class AdaptiveIconEngine : public QIconEngine {
public:
    // Builder receives (foreground, background) color hex strings
    using Builder = std::function<QByteArray(const QByteArray& fg, const QByteArray& bg)>;

    explicit AdaptiveIconEngine(Builder builder) : m_builder(std::move(builder)) {}

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State) override {
        auto group = (mode == QIcon::Disabled) ? QPalette::Disabled : QPalette::Normal;
        QByteArray fg = qApp->palette().color(group, QPalette::ButtonText).name().toUtf8();
        QByteArray bg = qApp->palette().color(group, QPalette::Button).name().toUtf8();

        QSvgRenderer renderer(m_builder(fg, bg));
        painter->setRenderHint(QPainter::Antialiasing);
        renderer.render(painter, QRectF(rect));
    }

    QIconEngine *clone() const override {
        return new AdaptiveIconEngine(m_builder);
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override {
        qreal dpr = qApp->devicePixelRatio();
        QPixmap pm(size * dpr);
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        paint(&p, QRect(QPoint(0, 0), size), mode, state);
        return pm;
    }

private:
    Builder m_builder;
};

// ▶ Play — right-pointing triangle
inline QIcon playIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<polygon points='5,3 5,13 12,8' fill='" + fg + "'/>");
    }));
}

// ■ Stop — filled square
inline QIcon stopIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<rect x='4' y='4' width='8' height='8' fill='" + fg + "'/>");
    }));
}

// ↗ ArrowNE — diagonal arrow with arrowhead
inline QIcon arrowNEIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg(
            "<line x1='4' y1='12' x2='11' y2='5' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
            "<polyline points='7,5 11,5 11,9' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
        );
    }));
}

// ▲ TriangleUp — up-pointing triangle
inline QIcon triangleUpIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<polygon points='8,3 3,13 13,13' fill='" + fg + "'/>");
    }));
}

// ▼ TriangleDown — down-pointing triangle
inline QIcon triangleDownIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<polygon points='3,3 13,3 8,13' fill='" + fg + "'/>");
    }));
}

// ● Circle — filled circle
inline QIcon circleIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<circle cx='8' cy='8' r='5' fill='" + fg + "'/>");
    }));
}

// ⬤ LargeCircle — larger filled circle
inline QIcon largeCircleIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<circle cx='8' cy='8' r='7' fill='" + fg + "'/>");
    }));
}

// ◀ TriangleLeft — left-pointing triangle
inline QIcon triangleLeftIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg("<polygon points='11,3 11,13 4,8' fill='" + fg + "'/>");
    }));
}

// ↑ ArrowUp
inline QIcon arrowUpIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg(
            "<line x1='8' y1='13' x2='8' y2='4' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
            "<polyline points='4,7 8,3 12,7' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
        );
    }));
}

// ↓ ArrowDown
inline QIcon arrowDownIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg(
            "<line x1='8' y1='3' x2='8' y2='12' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
            "<polyline points='4,9 8,13 12,9' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
        );
    }));
}

// ← ArrowLeft
inline QIcon arrowLeftIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg(
            "<line x1='13' y1='8' x2='4' y2='8' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
            "<polyline points='7,4 3,8 7,12' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
        );
    }));
}

// → ArrowRight
inline QIcon arrowRightIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg(
            "<line x1='3' y1='8' x2='12' y2='8' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
            "<polyline points='9,4 13,8 9,12' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
        );
    }));
}

// ✕ Cross — delete / close
inline QIcon crossIcon() {
    return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
        return makeSvg(
            "<line x1='4' y1='4' x2='12' y2='12' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
            "<line x1='12' y1='4' x2='4' y2='12' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
        );
    }));
}

// ∀ ForAll — inverted A / wildcard
// inline QIcon forAllIcon() {
//     return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
//         return makeSvg(
//             "<polyline points='3,3 8,13 13,3' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
//             "<line x1='5' y1='7' x2='11' y2='7' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
//         );
//     }));
// }

// ✓ Check — success
// inline QIcon checkIcon() {
//     return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
//         return makeSvg(
//             "<polyline points='3,8 6,12 13,4' fill='none' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round' stroke-linejoin='round'/>"
//         );
//     }));
// }

// ✗ BallotX — failure
// inline QIcon ballotXIcon() {
//     return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
//         return makeSvg(
//             "<line x1='4' y1='4' x2='12' y2='12' stroke='" + fg + "' stroke-width='2' stroke-linecap='round'/>"
//             "<line x1='12' y1='4' x2='4' y2='12' stroke='" + fg + "' stroke-width='2' stroke-linecap='round'/>"
//         );
//     }));
// }

// ⊕ CirclePlus — zip operation
// inline QIcon circlePlusIcon() {
//     return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
//         return makeSvg(
//             "<circle cx='8' cy='8' r='6' fill='none' stroke='" + fg + "' stroke-width='1.5'/>"
//             "<line x1='8' y1='4' x2='8' y2='12' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
//             "<line x1='4' y1='8' x2='12' y2='8' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
//         );
//     }));
// }

// ⊗ CircleTimes — broadcast operation
// inline QIcon circleTimesIcon() {
//     return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray&) {
//         return makeSvg(
//             "<circle cx='8' cy='8' r='6' fill='none' stroke='" + fg + "' stroke-width='1.5'/>"
//             "<line x1='4.8' y1='4.8' x2='11.2' y2='11.2' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
//             "<line x1='11.2' y1='4.8' x2='4.8' y2='11.2' stroke='" + fg + "' stroke-width='1.5' stroke-linecap='round'/>"
//         );
//     }));
// }


// ☠ Skull — chaos mode (skull and crossbones)
// Cutouts use palette Button color so they match the button background in any theme
// inline QIcon skullIcon() {
//     return QIcon(new AdaptiveIconEngine([](const QByteArray& fg, const QByteArray& bg) {
//         return makeSvg(
//             "<ellipse cx='8' cy='4.5' rx='4.5' ry='3.8' fill='" + fg + "'/>"
//             "<rect x='6' y='7.5' width='4' height='1.8' rx='0.8' fill='" + fg + "'/>"
//             "<circle cx='6.3' cy='4.2' r='1.0' fill='" + bg + "'/>"
//             "<circle cx='9.7' cy='4.2' r='1.0' fill='" + bg + "'/>"
//             "<ellipse cx='8' cy='6.5' rx='0.7' ry='0.8' fill='" + bg + "'/>"
//             "<line x1='3' y1='10.5' x2='13' y2='14.5' stroke='" + fg + "' stroke-width='1.2' stroke-linecap='round'/>"
//             "<line x1='13' y1='10.5' x2='3' y2='14.5' stroke='" + fg + "' stroke-width='1.2' stroke-linecap='round'/>"
//         );
//     }));
// }

} // namespace Sym
