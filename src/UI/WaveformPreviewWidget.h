#ifndef WAVEFORMPREVIEWWIDGET_H
#define WAVEFORMPREVIEWWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>

/**
 * PlayheadLabel - QLabel subclass that paints a playhead overlay
 * 
 * The waveform PNG may be much larger than the on-screen label
 * (e.g. 2800×600 scaled to ~700×120 via setScaledContents).
 * If we drew the playhead onto the pixmap, a 2px line would scale
 * down to sub-pixel. Instead, this subclass lets QLabel paint the
 * scaled waveform normally, then draws the 2px playhead on top
 * at widget (screen) coordinates — always crisp regardless of
 * source image resolution.
 */
class PlayheadLabel : public QLabel {
public:
    using QLabel::QLabel;
    double playheadProgress = -1.0;  // -1 = hidden, 0.0–1.0 = position
    
protected:
    void paintEvent(QPaintEvent* event) override {
        QLabel::paintEvent(event);  // Paint scaled waveform + stylesheet

        if (playheadProgress >= 0.0 && playheadProgress <= 1.0) {
            int playheadX = static_cast<int>(playheadProgress * width());
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            // Clip playhead to rounded corners (matches stylesheet border-radius)
            QPainterPath path;
            path.addRoundedRect(rect(), 4, 4);
            painter.setClipPath(path);

            painter.setPen(QPen(QColor(255, 85, 0, 200), 2));
            painter.drawLine(playheadX, 0, playheadX, height());
        }
    }
};

class WaveformPreviewWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit WaveformPreviewWidget(QWidget* parent = nullptr);
    ~WaveformPreviewWidget() override = default;
    
    void setPreviewFile(const QString& audioFilePath, const QString& waveformImagePath);
    void clearPreview();
    void setCanvasVisible(bool visible);
    
    // Public method to trigger playback (for keyboard shortcuts)
    void play() { onPlayClicked(); }
    void stop();

    void setLooping(bool loop) { m_looping = loop; mediaPlayer->setLoops(loop ? QMediaPlayer::Infinite : 1); }
    
signals:
    void generatePreviewRequested();
    void viewCommandRequested();
    void regionPreviewRequested();
    void playbackStarted();
    
private slots:
    void onPlayClicked();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onWaveformClicked(QMouseEvent* event);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    
private:
    QPushButton* generateButton;
    QPushButton* viewCommandButton;
    QPushButton* playButton;
    PlayheadLabel* waveformCanvas;
    QLabel* timeLabel;
    QPushButton* regionPreviewButton;

    QMediaPlayer* mediaPlayer;
    QAudioOutput* audioOutput;
    
    QPixmap waveformImage;
    qint64 duration;

    bool m_looping = false;

    bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif // WAVEFORMPREVIEWWIDGET_H
