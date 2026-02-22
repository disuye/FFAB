#include "RegionPreviewWindow.h"
#include "Utils/UnicodeSymbols.h"
#include "Core/Preferences.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>

// ============================================================================
// WAVEFORM CANVAS (proper QWidget subclass)
// ============================================================================

WaveformCanvas::WaveformCanvas(RegionPreviewWindow* owner, QWidget* parent)
    : QWidget(parent), m_owner(owner) {
}

void WaveformCanvas::paintEvent(QPaintEvent* event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Clip to rounded rect
    QPainterPath path;
    path.addRoundedRect(rect(), 4, 4);
    p.setClipPath(path);

    // Fill background
    p.fillRect(rect(), QColor(0x2A, 0x2A, 0x2A));

    // Draw waveform
    if (!m_displayPixmap.isNull()) {
        p.drawPixmap(0, 0, m_displayPixmap);
    }
}

void WaveformCanvas::mousePressEvent(QMouseEvent* event) {
    if (m_owner) m_owner->handleCanvasMousePress(event);
}

void WaveformCanvas::mouseMoveEvent(QMouseEvent* event) {
    if (m_owner) m_owner->handleCanvasMouseMove(event);
}

void WaveformCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (m_owner) m_owner->handleCanvasMouseRelease(event);
}

void WaveformCanvas::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_owner) m_owner->handleCanvasResize();
}

// ============================================================================
// CONSTRUCTION
// ============================================================================

RegionPreviewWindow::RegionPreviewWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Audio Preview");
    setMinimumWidth(600);
    resize(900, 300);
    
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(0);
    
    // ========== TOP BAR (matches CommandViewWindow info label area) ==========
    auto topBar = new QHBoxLayout();
    topBar->setSpacing(4);
    
    generateButton = new QPushButton("Generate Preview");
    connect(generateButton, &QPushButton::clicked, this, &RegionPreviewWindow::generatePreviewRequested);
    generateButton->setToolTip("Use [←] key to Generate Preview\nwhile using ↑/↓ in the File List.\nPress [→] or [Space] to play/stop.");
    topBar->addWidget(generateButton);
    
    playButton = new QPushButton();
    playButton->setIcon(Sym::playIcon());
    playButton->setIconSize(QSize(10, 10));
    playButton->setEnabled(false);
    playButton->setMaximumWidth(24);
    connect(playButton, &QPushButton::clicked, this, &RegionPreviewWindow::onPlayClicked);
    topBar->addWidget(playButton);
    
    // Playhead / Total timer
    timeLabel = new QLabel("00:00.000 / 00:00.000");
    timeLabel->setStyleSheet("color: #808080; font-size: 10px; padding-top: 3px;");
    topBar->addWidget(timeLabel);
    
    topBar->addStretch();
    
    // Selection counters (right-aligned)
    auto selStyle = QStringLiteral("color: #808080; font-size: 10px; padding-top: 3px;");
    
    selStartLabel = new QLabel("Start: —");
    selStartLabel->setStyleSheet(selStyle);
    topBar->addWidget(selStartLabel);
    
    selEndLabel = new QLabel("End: —");
    selEndLabel->setStyleSheet(selStyle);
    topBar->addWidget(selEndLabel);

    topBar->addStretch();

    auto regionTips = new QLabel("ALT+Click+Select region to preview | ALT+Click to clear | SHIFT+Click+Drag to move region");
    regionTips->setStyleSheet("color: #808080; font-size: 10px; padding-top: 3px;");
    topBar->addWidget(regionTips);

    layout->addLayout(topBar);
    layout->addSpacing(4);
    
    // ========== WAVEFORM CANVAS ==========
    waveformCanvas = new WaveformCanvas(this);
    waveformCanvas->setMinimumHeight(200);
    // Background and border-radius handled in WaveformCanvas::paintEvent()
    waveformCanvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    waveformCanvas->setMouseTracking(true);
    waveformCanvas->setCursor(Qt::CrossCursor);
    
    layout->addWidget(waveformCanvas);
    
    // ========== MEDIA PLAYER ==========
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(1.0);
    
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &RegionPreviewWindow::onPositionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &RegionPreviewWindow::onDurationChanged);
    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &RegionPreviewWindow::onPlaybackStateChanged);
    
    // Placeholder waveform
    QPixmap placeholder(800, 200);
    placeholder.fill(QColor(42, 42, 42));
    waveformImage = placeholder;
    
    // Restore geometry from preferences
    QByteArray geometry = Preferences::instance().regionWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

void RegionPreviewWindow::setPreviewFile(const QString& audioFilePath, const QString& waveformImagePath) {
    // Load waveform image
    waveformImage.load(waveformImagePath);
    if (waveformImage.isNull()) {
        qWarning() << "RegionPreviewWindow: Failed to load waveform image:" << waveformImagePath;
        return;
    }
    
    // Force reload media — suppress loop reaction to stop during reload
    m_suppressLoop = true;
    mediaPlayer->stop();
    mediaPlayer->setSource(QUrl());
    mediaPlayer->setSource(QUrl::fromLocalFile(audioFilePath));
    m_suppressLoop = false;

    playButton->setEnabled(true);
    m_playheadMs = 0;
    playButton->setIcon(Sym::playIcon());

    updateDisplay();

    qDebug() << "RegionPreviewWindow: Preview loaded:" << audioFilePath;
}

void RegionPreviewWindow::clearPreview() {
    m_suppressLoop = true;
    mediaPlayer->stop();
    mediaPlayer->setSource(QUrl());
    
    QPixmap placeholder(800, 200);
    placeholder.fill(QColor(42, 42, 42));
    waveformImage = placeholder;
    
    playButton->setEnabled(false);
    playButton->setIcon(Sym::playIcon());
    timeLabel->setText("00:00.000 / 00:00.000");
    
    m_duration = 0;
    m_playheadMs = 0;
    clearRegion();
    
    updateDisplay();
}

bool RegionPreviewWindow::hasRegion() const {
    return m_regionStartMs >= 0 && m_regionEndMs >= 0 && m_regionStartMs < m_regionEndMs;
}

qint64 RegionPreviewWindow::regionStartMs() const {
    return m_regionStartMs;
}

qint64 RegionPreviewWindow::regionEndMs() const {
    return m_regionEndMs;
}

void RegionPreviewWindow::clearRegion() {
    // If looping was clamped to this region, stop playback — no valid loop point anymore
    if (m_looping && hasRegion() && mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_suppressLoop = true;
        mediaPlayer->stop();
        mediaPlayer->setPosition(0);
        m_playheadMs = 0;
        playButton->setIcon(Sym::playIcon());
    }
    m_regionStartMs = -1;
    m_regionEndMs = -1;
    m_isDragging = false;
    m_isMoving = false;
    updateTimeLabels();
    updateDisplay();
    m_regionStartRatio = -1.0;
    m_regionEndRatio = -1.0;
}

void RegionPreviewWindow::handleCanvasResize() {
    updateDisplay();
}

// ============================================================================
// COORDINATE MAPPING
// ============================================================================

qint64 RegionPreviewWindow::widgetXToMs(int x) const {
    if (m_duration == 0 || !waveformCanvas) return 0;
    int w = waveformCanvas->width();
    if (w <= 0) return 0;
    double ratio = qBound(0.0, static_cast<double>(x) / w, 1.0);
    return static_cast<qint64>(ratio * m_duration);
}

int RegionPreviewWindow::msToCanvasX(qint64 ms) const {
    if (m_duration == 0 || !waveformCanvas) return 0;
    double ratio = static_cast<double>(ms) / m_duration;
    return static_cast<int>(ratio * waveformCanvas->width());
}

// ============================================================================
// DISPLAY
// ============================================================================

void RegionPreviewWindow::updateDisplay() {
    if (!waveformCanvas) return;
    
    int canvasW = waveformCanvas->width();
    int canvasH = waveformCanvas->height();
    if (canvasW <= 0 || canvasH <= 0) return;
    
    QPixmap display(canvasW, canvasH);
    display.fill(QColor(42, 42, 42));
    
    QPainter painter(&display);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 1. Draw base waveform (scaled to canvas)
    if (!waveformImage.isNull()) {
        painter.drawPixmap(0, 0, canvasW, canvasH, waveformImage);
    }
    
    // 2. Draw region overlay (dim excluded areas)
    if (hasRegion()) {
        int startX = msToCanvasX(m_regionStartMs);
        int endX = msToCanvasX(m_regionEndMs);
        
        // Dim left of region
        QColor dimColor(0, 0, 0, 140);
        if (startX > 0) {
            painter.fillRect(0, 0, startX, canvasH, dimColor);
        }
        // Dim right of region
        if (endX < canvasW) {
            painter.fillRect(endX, 0, canvasW - endX, canvasH, dimColor);
        }
        
        // Region edge lines
        QPen edgePen(QColor(255, 255, 255, 120), 1);
        painter.setPen(edgePen);
        painter.drawLine(startX, 0, startX, canvasH);
        painter.drawLine(endX, 0, endX, canvasH);
    }
    
    // 3. Draw playhead
    if (m_duration > 0 && m_playheadMs > 0) {
        int playheadX = msToCanvasX(m_playheadMs);
        painter.setPen(QPen(QColor(255, 85, 0, 200), 2));  // Identical orange playhead
        painter.drawLine(playheadX, 0, playheadX, canvasH);
    }
    
    painter.end();
    
    // Hand the composited pixmap to the canvas for painting
    waveformCanvas->setDisplayPixmap(display);
}

QString RegionPreviewWindow::formatTime(qint64 ms) const {
    if (ms < 0) return "—";
    int totalSec = ms / 1000;
    int millis = ms % 1000;
    return QString("%1:%2.%3")
        .arg(totalSec / 60, 2, 10, QChar('0'))
        .arg(totalSec % 60, 2, 10, QChar('0'))
        .arg(millis, 3, 10, QChar('0'));
}

void RegionPreviewWindow::updateTimeLabels() {
    // Playhead / Total
    timeLabel->setText(QString("%1 / %2")
        .arg(formatTime(m_playheadMs))
        .arg(formatTime(m_duration)));
    
    // Selection counters
    if (hasRegion()) {
        selStartLabel->setText(QString("Start: %1").arg(formatTime(m_regionStartMs)));
        selEndLabel->setText(QString("End: %1").arg(formatTime(m_regionEndMs)));
    } else {
        selStartLabel->setText("Start: —");
        selEndLabel->setText("End: —");
    }
}

// ============================================================================
// MOUSE HANDLING
// ============================================================================

void RegionPreviewWindow::handleCanvasMousePress(QMouseEvent* event) {
    if (m_duration == 0) return;
    
    qint64 clickMs = widgetXToMs(event->pos().x());
    
    if (event->button() == Qt::LeftButton && event->modifiers() & Qt::AltModifier) {
        // ===== ALT+CLICK: Start new region or clear =====
        m_isDragging = true;
        m_isMoving = false;
        m_dragAnchorMs = clickMs;
        m_regionStartMs = clickMs;
        m_regionEndMs = clickMs;
        updateTimeLabels();
        updateDisplay();
        
    } else if (event->button() == Qt::LeftButton && event->modifiers() & Qt::ShiftModifier) {
        // ===== SHIFT+CLICK: Move existing region =====
        if (hasRegion() && clickMs >= m_regionStartMs && clickMs <= m_regionEndMs) {
            m_isMoving = true;
            m_isDragging = false;
            m_moveOffsetMs = clickMs - m_regionStartMs;
            m_moveOrigStartMs = m_regionStartMs;
            m_moveOrigEndMs = m_regionEndMs;
            waveformCanvas->setCursor(Qt::ClosedHandCursor);
        }
        
    } else if (event->button() == Qt::LeftButton) {
        // ===== PLAIN CLICK: Seek + play =====
        emit playbackStarted();
        mediaPlayer->setPosition(clickMs);
        mediaPlayer->play();
        playButton->setIcon(Sym::stopIcon());
    }
}

void RegionPreviewWindow::handleCanvasMouseMove(QMouseEvent* event) {
    if (m_duration == 0) return;

    if (m_duration > 0) {
        m_regionStartRatio = static_cast<double>(m_regionStartMs) / m_duration;
        m_regionEndRatio = static_cast<double>(m_regionEndMs) / m_duration;
    }
    
    qint64 currentMs = widgetXToMs(event->pos().x());
    
    if (m_isDragging) {
        // Update region from anchor to current position
        m_regionStartMs = qMin(m_dragAnchorMs, currentMs);
        m_regionEndMs = qMax(m_dragAnchorMs, currentMs);
        
        // Clamp to file bounds
        m_regionStartMs = qMax(m_regionStartMs, qint64(0));
        m_regionEndMs = qMin(m_regionEndMs, m_duration);
        
        updateTimeLabels();
        updateDisplay();
        
    } else if (m_isMoving) {
        // Move region, keeping duration constant
        qint64 regionDuration = m_moveOrigEndMs - m_moveOrigStartMs;
        qint64 newStart = currentMs - m_moveOffsetMs;
        
        // Clamp to file bounds
        newStart = qMax(newStart, qint64(0));
        if (newStart + regionDuration > m_duration) {
            newStart = m_duration - regionDuration;
        }
        
        m_regionStartMs = newStart;
        m_regionEndMs = newStart + regionDuration;
        
        updateTimeLabels();
        updateDisplay();
        
    } else {
        // Update cursor based on hover position and modifiers
        bool altHeld = event->modifiers() & Qt::AltModifier;
        bool shiftHeld = event->modifiers() & Qt::ShiftModifier;
        
        if (altHeld) {
            waveformCanvas->setCursor(Qt::CrossCursor);
        } else if (shiftHeld && hasRegion()) {
            qint64 hoverMs = widgetXToMs(event->pos().x());
            if (hoverMs >= m_regionStartMs && hoverMs <= m_regionEndMs) {
                waveformCanvas->setCursor(Qt::OpenHandCursor);
            } else {
                waveformCanvas->setCursor(Qt::CrossCursor);
            }
        } else {
            waveformCanvas->setCursor(Qt::PointingHandCursor);
        }
    }
}

void RegionPreviewWindow::handleCanvasMouseRelease(QMouseEvent* event) {
    if (m_duration > 0) {
        m_regionStartRatio = static_cast<double>(m_regionStartMs) / m_duration;
        m_regionEndRatio = static_cast<double>(m_regionEndMs) / m_duration;
    }

    if (m_isDragging) {
        m_isDragging = false;
        
        // If start == end (no drag distance), this was just an ALT+click — clear region
        if (m_regionStartMs == m_regionEndMs) {
            clearRegion();
        }
        
        updateDisplay();
    }
    
    if (m_isMoving) {
        m_isMoving = false;
        waveformCanvas->setCursor(Qt::CrossCursor);
        updateDisplay();
    }
}

// ============================================================================
// MEDIA PLAYER SLOTS
// ============================================================================

void RegionPreviewWindow::onPlayClicked() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        // User-initiated stop — suppress loop restart
        m_suppressLoop = true;
        mediaPlayer->stop();
        mediaPlayer->setPosition(0);
        playButton->setIcon(Sym::playIcon());
        m_playheadMs = 0;
        updateDisplay();
    } else {
        // Play — if region active, start from region start
        emit playbackStarted();
        if (hasRegion()) {
            mediaPlayer->setPosition(m_regionStartMs);
        }
        mediaPlayer->play();
        playButton->setIcon(Sym::stopIcon());
    }
}

void RegionPreviewWindow::stopPlayback() {
    if (mediaPlayer->playbackState() != QMediaPlayer::StoppedState) {
        m_suppressLoop = true;
        mediaPlayer->stop();
        mediaPlayer->setPosition(0);
        playButton->setIcon(Sym::playIcon());
        m_playheadMs = 0;
        updateDisplay();
    }
}

void RegionPreviewWindow::playFromRegionOrStart() { 
    onPlayClicked(); 
}

// void RegionPreviewWindow::playFromRegionOrStart() {
//     if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
//         // Stop and reset
//         mediaPlayer->stop();
//         mediaPlayer->setPosition(0);
//         playButton->setText("▶");
//         m_playheadMs = 0;
//         updateDisplay();
//     } else {
//         // Play — if region active, start from region start
//         emit playbackStarted();
//         if (hasRegion()) {
//             mediaPlayer->setPosition(m_regionStartMs);
//         }
//         mediaPlayer->play();
//         playButton->setText("■");
//     }
// }

void RegionPreviewWindow::onPositionChanged(qint64 position) {
    m_playheadMs = position;

    // Region boundary handling
    if (hasRegion() && position >= m_regionEndMs) {
        if (m_looping) {
            // Loop: seek back to region start (keep playing — don't stop)
            mediaPlayer->setPosition(m_regionStartMs);
            m_playheadMs = m_regionStartMs;
        } else {
            // No loop: stop at region end
            m_suppressLoop = true;
            mediaPlayer->stop();
            m_playheadMs = m_regionEndMs;
            playButton->setIcon(Sym::playIcon());
        }
    }

    updateTimeLabels();
    updateDisplay();
}

void RegionPreviewWindow::onDurationChanged(qint64 newDuration) {
    m_duration = newDuration;
    qDebug() << "RegionPreviewWindow: Duration:" << m_duration << "ms";
    if (m_regionStartRatio >= 0 && m_regionEndRatio > m_regionStartRatio) {
        m_regionStartMs = static_cast<qint64>(m_regionStartRatio * m_duration);
        m_regionEndMs = static_cast<qint64>(m_regionEndRatio * m_duration);
        updateTimeLabels();
    }
    updateTimeLabels();
}

void RegionPreviewWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::StoppedState) {
        if (m_looping && !m_suppressLoop && playButton->isEnabled()) {
            // Natural end of file (no region, or region handling didn't catch it) — loop from start
            emit playbackStarted();
            mediaPlayer->setPosition(0);
            mediaPlayer->play();
            // playButton stays as stopIcon
        } else {
            m_suppressLoop = false;
            playButton->setIcon(Sym::playIcon());
            updateDisplay();
        }
    }
}

// ============================================================================
// WINDOW EVENTS
// ============================================================================

void RegionPreviewWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // Canvas now has real dimensions — recomposite
    updateDisplay();
    updateTimeLabels();
}

void RegionPreviewWindow::closeEvent(QCloseEvent* event) {
    stopPlayback();
    Preferences::instance().setRegionWindowGeometry(saveGeometry());
    emit windowClosed();
    QWidget::closeEvent(event);
}
