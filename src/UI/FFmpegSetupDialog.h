#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QDialogButtonBox;

// ============================================================================
// FFmpegInstallDialog — shows platform download links with auto-detection
//
// Accessed from:
//   1. The "Install FFmpeg..." button in FFmpegMissingDialog
//   2. Help > Install FFmpeg... menu action
// ============================================================================
class FFmpegInstallDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FFmpegInstallDialog(QWidget *parent = nullptr);

    /// Returns the path where FFmpeg was installed, or empty if cancelled
    QString installedPath() const { return m_installedPath; }

signals:
    /// Emitted after a successful download+extract so the caller can re-detect
    void ffmpegInstalled(const QString &ffmpegPath);

private slots:
    void onInstallNow();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();
    void onMetadataFinished();

private:
    void buildUI();

    struct PlatformLink {
        QString label;       // e.g. "macOS (Apple Silicon / Intel)"
        QString url;         // direct download URL
        QString archiveType; // "zip", "tar.xz"
        bool    recommended; // true = checkmark shown
    };
    QList<PlatformLink> allPlatformLinks() const;

    // Download + verify helpers
    void startDownload(const QString &url, const QString &archiveType);
    bool extractArchive(const QString &archivePath, const QString &destDir);
    QString findFFmpegBinary(const QString &dir);
    bool validateBinary(const QString &path);
    static QString installDir();

    QNetworkAccessManager *m_nam;
    QNetworkReply *m_currentReply = nullptr;
    QString m_expectedSha256;
    QString m_downloadArchiveType;
    QString m_installedPath;

    // Companion downloads (ffprobe, ffplay on macOS)
    QStringList m_companionQueue;  // URLs still to download
    void downloadNextCompanion();
    void onCompanionFinished();

    // UI
    QLabel       *m_statusLabel;
    QLabel       *m_installPathLabel;
    QProgressBar *m_progressBar;
    QPushButton  *m_installBtn;
    QPushButton  *m_cancelBtn;
};


// ============================================================================
// FFmpegMissingDialog — shown when FFmpeg cannot be found at startup
//
// Three buttons:
//   [Cancel]            — dismiss, do nothing (user proceeds without FFmpeg)
//   [Install FFmpeg...] — opens FFmpegInstallDialog
//   [Add FFmpeg Path]   — opens Settings dialog to manually enter path
// ============================================================================
class FFmpegMissingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FFmpegMissingDialog(QWidget *parent = nullptr);

    enum Result {
        Cancelled,
        Installed,   // FFmpeg was downloaded — re-detect needed
        OpenSettings // User chose to set path manually
    };

    Result userChoice() const { return m_choice; }

    /// Path returned by the install dialog, if applicable
    QString installedPath() const { return m_installedPath; }

private slots:
    void onInstallFFmpeg();
    void onAddPath();

private:
    Result  m_choice = Cancelled;
    QString m_installedPath;
};
