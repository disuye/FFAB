#include "FFmpegSetupDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QSettings>
#include <QDebug>


// ============================================================================
//  FFmpegInstallDialog
// ============================================================================

QString FFmpegInstallDialog::installDir()
{
    // ~/Library/Application Support/FFAB/ffmpeg  (macOS)
    // ~/.local/share/FFAB/ffmpeg                 (Linux)
    // %APPDATA%/FFAB/ffmpeg                      (Windows)
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString dir = base + "/ffmpeg";
    QDir().mkpath(dir);
    return dir;
}

QList<FFmpegInstallDialog::PlatformLink> FFmpegInstallDialog::allPlatformLinks() const
{
    QList<PlatformLink> links;

    // ----- macOS -----
    bool isMac = false;
    bool isLinux = false;
    bool isWindows = false;
    bool isArm = false;

#ifdef Q_OS_MACOS
    isMac = true;
#endif
#ifdef Q_OS_LINUX
    isLinux = true;
#endif
#ifdef Q_OS_WIN
    isWindows = true;
#endif
#if defined(__aarch64__) || defined(_M_ARM64)
    isArm = true;
#endif

    links.append({
        "macOS — Static build (Universal)",
        "https://evermeet.cx/ffmpeg/getrelease/zip",
        "zip",
        isMac
    });

    links.append({
        "Linux — Static build (amd64)",
        "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz",
        "tar.xz",
        isLinux && !isArm
    });

    links.append({
        "Linux — Static build (arm64)",
        "https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-arm64-static.tar.xz",
        "tar.xz",
        isLinux && isArm
    });

    links.append({
        "Windows — Static build (gyan.dev)",
        "https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip",
        "zip",
        isWindows
    });

    return links;
}


FFmpegInstallDialog::FFmpegInstallDialog(QWidget *parent)
    : QDialog(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    setWindowTitle("Install FFmpeg");
    setMinimumSize(560,560);
    setMaximumSize(560,560);
    setModal(true);
    buildUI();
}

void FFmpegInstallDialog::buildUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(20, 20, 20, 20);

    // Header
    auto *header = new QLabel(
        "<span style='font-size:14px;'>Download FFmpeg</span><br><br>"
        "FFAB will download, verify, and extract the following FFmpeg build plus companion apps FFprobe and FFplay. Or click the link to download and install manually.");
    header->setWordWrap(true);

    layout->addWidget(header);

    // ----- Platform links -----
    auto links = allPlatformLinks();
    for (const auto &link : links) {
        auto *row = new QHBoxLayout();
        row->setSpacing(0);

        // Checkmark for recommended platform
        QString indicator = link.recommended
            ? QString::fromUtf8(" →  ")   // Unicode check mark + space
            : QString("      ");            // Spacing to align
        auto *checkLabel = new QLabel(indicator);
        if (link.recommended)
            checkLabel->setStyleSheet("color: #FF5500; font-size: 12px;");
        checkLabel->setFixedWidth(32);
        row->addWidget(checkLabel);

        // Clickable link label
        auto *linkLabel = new QLabel(
            QString("<a style='color: #808080' href='%1'>%2</a>").arg(link.url, link.label));
        linkLabel->setOpenExternalLinks(true);
        linkLabel->setToolTip(link.url);
        row->addWidget(linkLabel, 1);

        layout->addLayout(row);
    }

    layout->setSpacing(24);
    
    // Install path info
    m_installPathLabel = new QLabel(
    QString("<small>FFmpeg will be installed to: <b>%1</b></small>")
    .arg(installDir()));
    m_installPathLabel->setWordWrap(true);
    m_installPathLabel->setStyleSheet("color: #808080;");

    layout->addWidget(m_installPathLabel);    
    layout->addStretch();

    // Status label
    m_statusLabel = new QLabel("");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);

    layout->addWidget(m_statusLabel);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedHeight(4);
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "    border: none;"
        "    background-color: #404040;"
        "    border-radius: 2px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: rgba(255, 85, 0, 0.80);"
        "    border-radius: 2px;"
        "}"
    );

    layout->addWidget(m_progressBar);
    layout->addStretch();

    // Buttons
    auto *btnLayout = new QHBoxLayout();

    m_cancelBtn = new QPushButton("Cancel");
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_cancelBtn);

    btnLayout->addStretch();

    m_installBtn = new QPushButton("Install Now");
    m_installBtn->setDefault(true);
    connect(m_installBtn, &QPushButton::clicked, this, &FFmpegInstallDialog::onInstallNow);
    btnLayout->addWidget(m_installBtn);

    layout->addLayout(btnLayout);

    // Source attribution
    auto *sourceLabel = new QLabel(
        "<small>Sources: "
        "<a style='color: #808080;' href='https://evermeet.cx/ffmpeg/'>evermeet.cx</a> | "
        "<a style='color: #808080;' href='https://johnvansickle.com/ffmpeg/'>johnvansickle.com</a> | "
        "<a style='color: #808080;' href='https://www.gyan.dev/ffmpeg/builds/'>gyan.dev</a>"
        "</small>");
    sourceLabel->setOpenExternalLinks(true);
    sourceLabel->setStyleSheet("color: #808080;");
    layout->addWidget(sourceLabel);
}


void FFmpegInstallDialog::onInstallNow()
{
    // Find the recommended platform link
    auto links = allPlatformLinks();
    for (const auto &link : links) {
        if (link.recommended) {
            m_installBtn->setEnabled(false);
            m_downloadArchiveType = link.archiveType;

#ifdef Q_OS_MACOS
            // Fetch metadata first (includes SHA-256)
            m_statusLabel->setText("Fetching release info...");
            m_statusLabel->setVisible(true);
            m_progressBar->setVisible(true);
            m_progressBar->setMaximum(0); // indeterminate

            QNetworkRequest req(QUrl("https://evermeet.cx/ffmpeg/info/ffmpeg/release"));
            req.setHeader(QNetworkRequest::UserAgentHeader, "FFAB/1.0");
            m_currentReply = m_nam->get(req);
            connect(m_currentReply, &QNetworkReply::finished,
                    this, &FFmpegInstallDialog::onMetadataFinished);
            return;
#else
            // Other platforms: download directly (no SHA-256 metadata endpoint)
            startDownload(link.url, link.archiveType);
            return;
#endif
        }
    }

    // No recommended link found (shouldn't happen)
    m_statusLabel->setText("No suitable download found for this platform.");
    m_statusLabel->setStyleSheet("color: #d42424;");
    m_statusLabel->setVisible(true);
}

void FFmpegInstallDialog::onMetadataFinished()
{
    auto *reply = m_currentReply;
    m_currentReply = nullptr;
    reply->deleteLater();

    QString downloadUrl;

    // Find recommended link URL
    auto links = allPlatformLinks();
    for (const auto &link : links) {
        if (link.recommended) {
            downloadUrl = link.url;
            break;
        }
    }

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            m_expectedSha256 = obj.value("sha256").toString();
            QString version = obj.value("version").toString();
            m_statusLabel->setText(QString("Found FFmpeg %1 — downloading...").arg(version));
        }
    } else {
        m_statusLabel->setText("Could not fetch checksums — downloading without verification...");
        m_expectedSha256.clear();
    }

    startDownload(downloadUrl, m_downloadArchiveType);
}

void FFmpegInstallDialog::startDownload(const QString &url, const QString &archiveType)
{
    m_downloadArchiveType = archiveType;
    m_statusLabel->setText("Downloading ffmpeg...");
    m_statusLabel->setVisible(true);
    m_progressBar->setVisible(true);
    m_progressBar->setMaximum(0); // indeterminate until we get Content-Length

    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "FFAB/1.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    m_currentReply = m_nam->get(req);
    connect(m_currentReply, &QNetworkReply::downloadProgress,
            this, &FFmpegInstallDialog::onDownloadProgress);
    connect(m_currentReply, &QNetworkReply::finished,
            this, &FFmpegInstallDialog::onDownloadFinished);
}

void FFmpegInstallDialog::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        m_progressBar->setMaximum(static_cast<int>(total / 1024));
        m_progressBar->setValue(static_cast<int>(received / 1024));
        double mb = received / (1024.0 * 1024.0);
        double totalMb = total / (1024.0 * 1024.0);
        m_progressBar->setFormat(QString("%1 / %2 MB")
            .arg(mb, 0, 'f', 1)
            .arg(totalMb, 0, 'f', 1));
    }
}

void FFmpegInstallDialog::onDownloadFinished()
{
    auto *reply = m_currentReply;
    m_currentReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        m_statusLabel->setText(QString("Download failed: %1").arg(reply->errorString()));
        m_statusLabel->setStyleSheet("color: #d42424;");
        m_progressBar->setVisible(false);
        m_installBtn->setEnabled(true);
        m_installBtn->setText("Retry");
        return;
    }

    // Write archive to disk
    QString dest = installDir();
    QString ext = m_downloadArchiveType.split(".").last();
    QString archivePath = dest + "/ffmpeg_download." + ext;

    QFile file(archivePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_statusLabel->setText("Cannot write to " + archivePath);
        m_statusLabel->setStyleSheet("color: #d42424;");
        m_progressBar->setVisible(false);
        m_installBtn->setEnabled(true);
        return;
    }

    QByteArray archiveData = reply->readAll();
    file.write(archiveData);
    file.close();

    // ---- SHA-256 verification ----
    if (!m_expectedSha256.isEmpty()) {
        m_statusLabel->setText("Verifying SHA-256 checksum...");

        QFile f(archivePath);
        if (f.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            while (!f.atEnd())
                hash.addData(f.read(1024 * 1024));
            f.close();

            QString computed = hash.result().toHex().toLower();
            if (computed != m_expectedSha256.toLower()) {
                QFile::remove(archivePath);
                m_statusLabel->setText(
                    QString("✗ Checksum verification FAILED.\n\n"
                            "Expected: %1\nGot: %2\n\n"
                            "The download may be corrupted. Please try again.")
                        .arg(m_expectedSha256, computed));
                m_statusLabel->setStyleSheet("color: #d42424;");
                m_progressBar->setVisible(false);
                m_installBtn->setEnabled(true);
                m_installBtn->setText("Retry");
                return;
            }
            m_statusLabel->setText("✓ SHA-256 verified — extracting...");
        }
    } else {
        m_statusLabel->setText("Extracting...");
    }

    // ---- Extract ----
    if (!extractArchive(archivePath, dest)) {
        m_statusLabel->setText("Failed to extract archive.");
        m_statusLabel->setStyleSheet("color: #d42424;");
        m_progressBar->setVisible(false);
        m_installBtn->setEnabled(true);
        return;
    }

    QFile::remove(archivePath);

    // ---- Strip macOS quarantine/provenance attributes ----
    //  Downloaded files get com.apple.quarantine / com.apple.provenance
    //  which prevents QFile::exists() from seeing them on subsequent launches.
#ifdef Q_OS_MACOS
    QProcess::execute("xattr", {"-cr", dest});
#endif

    // ---- Find binary ----
    QString ffmpegBin = findFFmpegBinary(dest);
    if (ffmpegBin.isEmpty() || !validateBinary(ffmpegBin)) {
        m_statusLabel->setText("Could not find a valid ffmpeg binary in extracted files.");
        m_statusLabel->setStyleSheet("color: #d42424;");
        m_progressBar->setVisible(false);
        m_installBtn->setEnabled(true);
        return;
    }

    // ---- Set permissions (Unix) ----
#ifndef Q_OS_WIN
    QFile::setPermissions(ffmpegBin,
        QFileDevice::ReadOwner  | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
        QFileDevice::ReadGroup  | QFileDevice::ExeGroup   |
        QFileDevice::ReadOther  | QFileDevice::ExeOther);

    // Also ffprobe and ffplay if present
    QFileInfo fi(ffmpegBin);
    for (const QString &sibling : {"ffprobe", "ffplay"}) {
        QString siblingBin = fi.absolutePath() + "/" + sibling;
        if (QFile::exists(siblingBin)) {
            QFile::setPermissions(siblingBin,
                QFileDevice::ReadOwner  | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                QFileDevice::ReadGroup  | QFileDevice::ExeGroup   |
                QFileDevice::ReadOther  | QFileDevice::ExeOther);
        }
    }
#endif

    // ---- Save path to QSettings ----
    QSettings settings;
    settings.setValue("processing/ffmpegPath", ffmpegBin);
    settings.sync();

    m_installedPath = ffmpegBin;

    // ---- Download companion tools (ffprobe, ffplay) on macOS ----
#ifdef Q_OS_MACOS
    QFileInfo fi2(ffmpegBin);
    QString dir = fi2.absolutePath();
    m_companionQueue.clear();
    if (!QFile::exists(dir + "/ffprobe"))
        m_companionQueue << "https://evermeet.cx/ffmpeg/getrelease/ffprobe/zip";
    if (!QFile::exists(dir + "/ffplay"))
        m_companionQueue << "https://evermeet.cx/ffmpeg/getrelease/ffplay/zip";

    if (!m_companionQueue.isEmpty()) {
        m_statusLabel->setText("✓ FFmpeg installed — downloading ffprobe & ffplay...");
        downloadNextCompanion();
        return;  // will finish in onCompanionFinished
    }
#endif

    // ---- Show version (no companions needed or already present) ----
    QProcess proc;
    proc.setProgram(ffmpegBin);
    proc.setArguments({"-version"});
    proc.start();
    QString versionStr = "unknown";
    if (proc.waitForFinished(5000)) {
        versionStr = proc.readAllStandardOutput().split('\n').first().trimmed();
    }

    m_progressBar->setVisible(false);
    m_statusLabel->setText(
        QString("✓ FFmpeg installed successfully\n\n%1\n\nPath: %2")
            .arg(versionStr, ffmpegBin));
        adjustSize();
    m_installBtn->setVisible(false);
    m_cancelBtn->setText("Done");
    disconnect(m_cancelBtn, nullptr, nullptr, nullptr);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::accept);

    emit ffmpegInstalled(ffmpegBin);
}


bool FFmpegInstallDialog::extractArchive(const QString &archivePath, const QString &destDir)
{
    QProcess proc;
    proc.setWorkingDirectory(destDir);

#ifdef Q_OS_WIN
    proc.setProgram("powershell");
    proc.setArguments({
        "-NoProfile", "-Command",
        QString("Expand-Archive -Path '%1' -DestinationPath '%2' -Force")
            .arg(archivePath, destDir)
    });
#elif defined(Q_OS_MACOS)
    proc.setProgram("ditto");
    proc.setArguments({"-xk", archivePath, destDir});
#else
    proc.setProgram("tar");
    proc.setArguments({"xf", archivePath, "-C", destDir});
#endif

    proc.start();
    bool ok = proc.waitForFinished(120000) && proc.exitCode() == 0;
    if (!ok) {
        qWarning() << "Archive extraction failed:" << proc.readAllStandardError();
    }
    return ok;
}


QString FFmpegInstallDialog::findFFmpegBinary(const QString &dir)
{
    QString name = "ffmpeg";
#ifdef Q_OS_WIN
    name = "ffmpeg.exe";
#endif

    // Top level
    if (QFile::exists(dir + "/" + name))
        return dir + "/" + name;

    // One level deep (archives often wrap in a directory)
    QDir d(dir);
    for (const QFileInfo &entry : d.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString candidate = entry.absoluteFilePath() + "/" + name;
        if (QFile::exists(candidate))
            return candidate;

        candidate = entry.absoluteFilePath() + "/bin/" + name;
        if (QFile::exists(candidate))
            return candidate;
    }

    return {};
}


bool FFmpegInstallDialog::validateBinary(const QString &path)
{
    if (!QFile::exists(path))
        return false;

    QProcess proc;
    proc.setProgram(path);
    proc.setArguments({"-version"});
    proc.start();
    if (!proc.waitForFinished(5000))
        return false;

    return proc.readAllStandardOutput().contains("ffmpeg version");
}


void FFmpegInstallDialog::downloadNextCompanion()
{
    if (m_companionQueue.isEmpty()) {
        // All companions done — show final success
        QProcess proc;
        proc.setProgram(m_installedPath);
        proc.setArguments({"-version"});
        proc.start();
        QString versionStr = "unknown";
        if (proc.waitForFinished(5000)) {
            versionStr = proc.readAllStandardOutput().split('\n').first().trimmed();
        }

        // Check what we ended up with
        QFileInfo fi(m_installedPath);
        QString dir = fi.absolutePath();
        QStringList installed;
        installed << "ffmpeg";
        if (QFile::exists(dir + "/ffprobe")) installed << "ffprobe";
        if (QFile::exists(dir + "/ffplay"))  installed << "ffplay";

        m_progressBar->setVisible(false);
        m_statusLabel->setText(
            QString("✓ Installed: %1\n\n%2\n\nPath: %3")
                .arg(installed.join(", "), versionStr, m_installedPath));
            adjustSize();
        m_installBtn->setVisible(false);
        m_cancelBtn->setText("Done");
        disconnect(m_cancelBtn, nullptr, nullptr, nullptr);
        connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::accept);

        emit ffmpegInstalled(m_installedPath);
        return;
    }

    QString url = m_companionQueue.takeFirst();
    // Extract tool name from URL for status display
    QString toolName = url.contains("ffprobe") ? "ffprobe" : "ffplay";
    m_statusLabel->setText(QString("Downloading %1...").arg(toolName));

    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "FFAB/1.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    m_currentReply = m_nam->get(req);
    connect(m_currentReply, &QNetworkReply::downloadProgress,
            this, &FFmpegInstallDialog::onDownloadProgress);
    connect(m_currentReply, &QNetworkReply::finished,
            this, &FFmpegInstallDialog::onCompanionFinished);
}


void FFmpegInstallDialog::onCompanionFinished()
{
    auto *reply = m_currentReply;
    m_currentReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        // Companion failure is non-fatal — just log and continue
        qWarning() << "Companion download failed:" << reply->errorString();
        downloadNextCompanion();
        return;
    }

    QString dest = installDir();
    QString archivePath = dest + "/companion_download.zip";

    QFile file(archivePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();

        // Extract
        extractArchive(archivePath, dest);
        QFile::remove(archivePath);

        // Strip macOS quarantine/provenance attributes
#ifdef Q_OS_MACOS
        QProcess::execute("xattr", {"-cr", dest});
#endif

        // Set permissions on any new binaries
#ifndef Q_OS_WIN
        for (const QString &name : {"ffprobe", "ffplay"}) {
            QString binPath = dest + "/" + name;
            if (QFile::exists(binPath)) {
                QFile::setPermissions(binPath,
                    QFileDevice::ReadOwner  | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                    QFileDevice::ReadGroup  | QFileDevice::ExeGroup   |
                    QFileDevice::ReadOther  | QFileDevice::ExeOther);
            }
        }
#endif
    }

    downloadNextCompanion();
}


// ============================================================================
//  FFmpegMissingDialog
// ============================================================================

FFmpegMissingDialog::FFmpegMissingDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("FFAB");
    setMinimumWidth(560);
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(24, 20, 24, 20);

    // Icon + message
    auto *msgLabel = new QLabel(
        "<span style='font-size:14px;'>FFmpeg needs to be installed, or cannot be found</span><br><br>"
        "<small>FFAB requires FFmpeg to process audio: FFAB can install it automatically right now, or you can manually add a path to an existing installation.</small>");
    msgLabel->setWordWrap(true);
    layout->addWidget(msgLabel);

    layout->addSpacing(8);

    // Buttons
    auto *btnLayout = new QHBoxLayout();

    auto *cancelBtn = new QPushButton("Cancel");
    cancelBtn->setToolTip("Continue without FFmpeg — processing will not work");
    connect(cancelBtn, &QPushButton::clicked, this, [this]() {
        m_choice = Cancelled;
        reject();
    });
    btnLayout->addWidget(cancelBtn);

    btnLayout->addStretch();

    auto *addPathBtn = new QPushButton("Add FFmpeg Path...");
    addPathBtn->setToolTip("Open Settings to enter the path to an existing FFmpeg installation");
    connect(addPathBtn, &QPushButton::clicked, this, &FFmpegMissingDialog::onAddPath);
    btnLayout->addWidget(addPathBtn);

    auto *installBtn = new QPushButton("Install FFmpeg...");
    installBtn->setDefault(true);
    connect(installBtn, &QPushButton::clicked, this, &FFmpegMissingDialog::onInstallFFmpeg);
    btnLayout->addWidget(installBtn);

    layout->addLayout(btnLayout);
}

void FFmpegMissingDialog::onInstallFFmpeg()
{
    FFmpegInstallDialog installDlg(this);
    if (installDlg.exec() == QDialog::Accepted) {
        m_installedPath = installDlg.installedPath();
        m_choice = Installed;
        accept();
    }
    // If user cancelled the install dialog, we stay open
}

void FFmpegMissingDialog::onAddPath()
{
    m_choice = OpenSettings;
    accept();
}
