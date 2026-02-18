#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPixmap>

class QCloseEvent;
class QMouseEvent;
class RegionPreviewWindow;

/**
 * WaveformCanvas - Custom painted widget for waveform display
 * 
 * Proper QWidget subclass with paintEvent — avoids stylesheet/eventFilter
 * conflicts that prevent painting on plain QWidgets.
 */
class WaveformCanvas : public QWidget {
    Q_OBJECT
    
public:
    explicit WaveformCanvas(RegionPreviewWindow* owner, QWidget* parent = nullptr);
    
    void setDisplayPixmap(const QPixmap& pm) { m_displayPixmap = pm; update(); }
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private:
    RegionPreviewWindow* m_owner = nullptr;
    QPixmap m_displayPixmap;
};

/**
 * RegionPreviewWindow - Expanded waveform view with region selection
 * 
 * Opens as a modeless window (like CommandViewWindow).
 * Displays the same waveform PNG at a larger size with a custom-painted
 * canvas for pixel-accurate region selection and playhead rendering.
 * 
 * Region selection controls playback bounds only (no FFmpeg involvement):
 *   - ALT+click+drag:   Select a region (or replace existing)
 *   - ALT+click (no drag): Clear region
 *   - SHIFT+drag inside: Move region (clamped to file bounds)
 *   - Plain click:       Seek playhead + play
 * 
 * When a region is active, playback is clamped to the region bounds.
 */
class RegionPreviewWindow : public QWidget {
    Q_OBJECT
    
public:
    explicit RegionPreviewWindow(QWidget* parent = nullptr);
    ~RegionPreviewWindow() override = default;
    
    // Feed the same preview data as WaveformPreviewWidget
    void setPreviewFile(const QString& audioFilePath, const QString& waveformImagePath);
    void clearPreview();
    
    // Region query (for PreviewGenerator)
    bool hasRegion() const;
    qint64 regionStartMs() const;
    qint64 regionEndMs() const;
    void clearRegion();
    
    // Called by WaveformCanvas — public so the inner class can access
    void handleCanvasMousePress(QMouseEvent* event);
    void handleCanvasMouseMove(QMouseEvent* event);
    void handleCanvasMouseRelease(QMouseEvent* event);
    void handleCanvasResize();
    
    // Playback control (for cross-stop and key commands)
    void stopPlayback();
    void playFromRegionOrStart();
    
signals:
    void generatePreviewRequested();
    void playbackStarted();
    void windowClosed();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    
private slots:
    void onPlayClicked();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 newDuration);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    
private:
    // ========== COORDINATE MAPPING ==========
    qint64 widgetXToMs(int x) const;
    int msToCanvasX(qint64 ms) const;
    
    // ========== DISPLAY ==========
    void updateDisplay();
    QString formatTime(qint64 ms) const;
    void updateTimeLabels();
    
    // ========== WIDGETS ==========
    QPushButton* generateButton = nullptr;
    QPushButton* playButton = nullptr;
    QLabel* timeLabel = nullptr;
    QLabel* selStartLabel = nullptr;
    QLabel* selEndLabel = nullptr;
    
    WaveformCanvas* waveformCanvas = nullptr;
    
    // ========== MEDIA PLAYER ==========
    QMediaPlayer* mediaPlayer = nullptr;
    QAudioOutput* audioOutput = nullptr;
    
    // ========== STATE ==========
    QPixmap waveformImage;
    qint64 m_duration = 0;
    
    qint64 m_regionStartMs = -1;
    qint64 m_regionEndMs = -1;
    
    bool m_isDragging = false;
    bool m_isMoving = false;
    qint64 m_dragAnchorMs = -1;
    qint64 m_moveOffsetMs = 0;
    qint64 m_moveOrigStartMs = 0;
    qint64 m_moveOrigEndMs = 0;
    
    qint64 m_playheadMs = 0;

    // percentage region location
    double m_regionStartRatio = -1.0;  // 0.0–1.0, -1 = no region
    double m_regionEndRatio = -1.0;
};
