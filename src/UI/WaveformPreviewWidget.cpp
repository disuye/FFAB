#include "WaveformPreviewWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QStyle>
#include "Utils/UnicodeSymbols.h"

WaveformPreviewWidget::WaveformPreviewWidget(QWidget* parent) 
    : QWidget(parent), duration(0) {
    
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);
    
    // ========== BUTTONS ROW ==========
    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(4);

    regionPreviewButton = new QPushButton();
    regionPreviewButton->setIcon(Sym::arrowNEIcon());
    regionPreviewButton->setIconSize(QSize(10, 10));
    connect(regionPreviewButton, &QPushButton::clicked, this, &WaveformPreviewWidget::regionPreviewRequested);
    regionPreviewButton->setMaximumWidth(24);
    regionPreviewButton->setToolTip("Open the Audio Preview window.");
    buttonsLayout->addWidget(regionPreviewButton);

    generateButton = new QPushButton("Generate Preview");
    connect(generateButton, &QPushButton::clicked, this, &WaveformPreviewWidget::generatePreviewRequested);
    generateButton->setToolTip("Use [←] key to Generate Preview\nwhile using ↑/↓ in the File List.\nPress [→] or [Space] to play/stop.");
    buttonsLayout->addWidget(generateButton);

    playButton = new QPushButton();
    playButton->setIcon(Sym::playIcon());
    playButton->setIconSize(QSize(10, 10));
    connect(playButton, &QPushButton::clicked, this, &WaveformPreviewWidget::onPlayClicked);
    playButton->setMaximumWidth(24);
    playButton->setEnabled(false);
    buttonsLayout->addWidget(playButton);
    
    timeLabel = new QLabel(" 00:00:000 /  00:00:000");
    timeLabel->setStyleSheet("color: #808080; font-size: 10px; padding-top: 3px; ");
    buttonsLayout->addWidget(timeLabel);
    
    buttonsLayout->addStretch();
    
    viewCommandButton = new QPushButton("View Command");
    connect(viewCommandButton, &QPushButton::clicked, this, &WaveformPreviewWidget::viewCommandRequested);
    viewCommandButton->setToolTip("Use the [ \\ ] key to toggle\nffmpeg command highlighting.");
    buttonsLayout->addWidget(viewCommandButton);
    
    mainLayout->addLayout(buttonsLayout);
    
    // ========== WAVEFORM CANVAS ==========
    waveformCanvas = new PlayheadLabel();
    waveformCanvas->setMinimumHeight(120);
    waveformCanvas->setMaximumHeight(150);
    waveformCanvas->setStyleSheet("background-color: #2A2A2A; border: 1px solid #404040; border-radius: 4px;");
    waveformCanvas->setAlignment(Qt::AlignCenter);
    waveformCanvas->setScaledContents(true);
    waveformCanvas->installEventFilter(this);
    waveformCanvas->setCursor(Qt::PointingHandCursor);
    
    // Placeholder waveform
    QPixmap placeholder(800, 120);
    placeholder.fill(QColor(42, 42, 42));
    waveformImage = placeholder;
    waveformCanvas->setPixmap(placeholder);
    
    mainLayout->addWidget(waveformCanvas);
    
    // ========== MEDIA PLAYER SETUP ==========
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(1.0);
    
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &WaveformPreviewWidget::onPositionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &WaveformPreviewWidget::onDurationChanged);
    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &WaveformPreviewWidget::onPlaybackStateChanged);
}

void WaveformPreviewWidget::setPreviewFile(const QString& audioFilePath, const QString& waveformImagePath) {
    // Load waveform image
    waveformImage.load(waveformImagePath);
    if (waveformImage.isNull()) {
        qWarning() << "Failed to load waveform image:" << waveformImagePath;
        return;
    }

    waveformCanvas->setPixmap(waveformImage);
    waveformCanvas->playheadProgress = -1.0;  // No playhead yet

    // FORCE RELOAD: suppress loop reaction to the stop, then reload source
    m_suppressLoop = true;
    mediaPlayer->stop();
    mediaPlayer->setSource(QUrl());  // Clear old source
    mediaPlayer->setSource(QUrl::fromLocalFile(audioFilePath));  // Load new
    m_suppressLoop = false;

    // Enable playback controls
    playButton->setEnabled(true);
    playButton->setIcon(Sym::playIcon());

    qDebug() << "Preview loaded:" << audioFilePath;
}

void WaveformPreviewWidget::clearPreview() {
    m_suppressLoop = true;
    mediaPlayer->stop();
    mediaPlayer->setSource(QUrl());
    
    // Reset to placeholder
    QPixmap placeholder(800, 120);
    placeholder.fill(QColor(42, 42, 42));
    waveformImage = placeholder;
    waveformCanvas->setPixmap(placeholder);
    
    playButton->setEnabled(true);
    playButton->setIcon(Sym::playIcon());
    timeLabel->setText(" 00:00:000 /  00:00:000");
    
    duration = 0;
    waveformCanvas->playheadProgress = -1.0;
}

void WaveformPreviewWidget::setCanvasVisible(bool visible) {
    waveformCanvas->setVisible(visible);
    generateButton->setVisible(visible);
    playButton->setVisible(visible);
    timeLabel->setVisible(visible);
}

void WaveformPreviewWidget::onPlayClicked() {
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        // User-initiated stop — suppress loop restart
        m_suppressLoop = true;
        mediaPlayer->stop();
        mediaPlayer->setPosition(0);
        playButton->setIcon(Sym::playIcon());
        waveformCanvas->playheadProgress = -1.0;
        waveformCanvas->update();  // Repaint without playhead
    } else {
        // Play
        emit playbackStarted();
        mediaPlayer->play();
        playButton->setIcon(Sym::stopIcon());
    }
}

void WaveformPreviewWidget::stop() {
    // Always stop playback (not a toggle like onPlayClicked)
    if (mediaPlayer->playbackState() != QMediaPlayer::StoppedState) {
        m_suppressLoop = true;
        mediaPlayer->stop();
        mediaPlayer->setPosition(0);
        playButton->setIcon(Sym::playIcon());
        waveformCanvas->playheadProgress = -1.0;
        waveformCanvas->update();  // Repaint without playhead
    }
}


void WaveformPreviewWidget::onPositionChanged(qint64 position) {
    if (duration > 0) {
        waveformCanvas->playheadProgress = static_cast<double>(position) / duration;
        waveformCanvas->update();
    }
    
    int curMins = position / 60000;
    int curSecs = (position % 60000) / 1000;
    int curMs   = position % 1000;
    
    int totMins = duration / 60000;
    int totSecs = (duration % 60000) / 1000;
    int totMs   = duration % 1000;
    
    QString currentTime = QString("%1:%2:%3")
        .arg(curMins, 2, 10, QChar('0'))
        .arg(curSecs, 2, 10, QChar('0'))
        .arg(curMs, 3, 10, QChar('0'));
    
    QString totalTime = QString("%1:%2:%3")
        .arg(totMins, 2, 10, QChar('0'))
        .arg(totSecs, 2, 10, QChar('0'))
        .arg(totMs, 3, 10, QChar('0'));
    
    timeLabel->setText(QString(" %1 /  %2").arg(currentTime).arg(totalTime));
}

void WaveformPreviewWidget::onDurationChanged(qint64 newDuration) {
    duration = newDuration;
    qDebug() << "Duration changed to:" << duration << "ms =" << (duration / 1000.0) << "seconds";
    
    int mins = duration / 60000;
    int secs = (duration % 60000) / 1000;
    int ms = duration % 1000;
    QString totalTime = QString("%1:%2:%3")
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'))
        .arg(ms, 3, 10, QChar('0'));
    timeLabel->setText(QString("00:00:000 / %1").arg(totalTime));
}

bool WaveformPreviewWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == waveformCanvas && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        onWaveformClicked(mouseEvent);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void WaveformPreviewWidget::onWaveformClicked(QMouseEvent* event) {
    if (duration == 0 || waveformImage.isNull()) return;
    
    // Calculate position from click
    int clickX = event->pos().x();
    int labelWidth = waveformCanvas->width();
    double progress = static_cast<double>(clickX) / labelWidth;
    qint64 newPosition = static_cast<qint64>(progress * duration);
    
    // Seek to position and start playing
    emit playbackStarted();
    mediaPlayer->setPosition(newPosition);
    mediaPlayer->play();
    playButton->setIcon(Sym::stopIcon());
}

// Reset [stop] icon back to [play] once audio playback is complete, or loop if active
void WaveformPreviewWidget::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    if (state == QMediaPlayer::StoppedState) {
        if (m_looping && !m_suppressLoop && playButton->isEnabled()) {
            // Natural end of file — loop from beginning
            emit playbackStarted();
            mediaPlayer->setPosition(0);
            mediaPlayer->play();
            // playButton stays as stopIcon
        } else {
            // Programmatic stop or loop not active
            m_suppressLoop = false;
            playButton->setIcon(Sym::playIcon());
            waveformCanvas->playheadProgress = -1.0;
            waveformCanvas->update();
        }
    }
}
