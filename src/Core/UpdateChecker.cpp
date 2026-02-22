#include "UpdateChecker.h"
#include "AppConfig.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

static const QString API_URL =
    "https://api.github.com/repos/disuye/FFAB/releases/latest";

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
{
    loadCache();
}

// ── Settings accessors ──────────────────────────────────────────────

bool UpdateChecker::weeklyCheckEnabled() const {
    QSettings s;
    return s.value("updates/checkWeekly", true).toBool();
}

void UpdateChecker::setWeeklyCheckEnabled(bool enabled) {
    QSettings s;
    s.setValue("updates/checkWeekly", enabled);
}

// ── Cache persistence ───────────────────────────────────────────────

void UpdateChecker::loadCache() {
    QSettings s;
    m_lastCheckTime  = s.value("updates/lastCheckTime").toDateTime();
    m_latestVersion  = s.value("updates/latestVersion").toString();
    m_downloadUrl    = s.value("updates/downloadUrl").toString();

    if (!m_latestVersion.isEmpty()) {
        auto remote = parseSemVer(m_latestVersion);
        auto local  = parseSemVer(VERSION_STR);
        m_updateAvailable = remote.valid && local.valid && isNewer(remote, local);
    }
}

void UpdateChecker::saveCache() {
    QSettings s;
    s.setValue("updates/lastCheckTime", m_lastCheckTime);
    s.setValue("updates/latestVersion", m_latestVersion);
    s.setValue("updates/downloadUrl",   m_downloadUrl);
    s.sync();
}

// ── Version comparison ──────────────────────────────────────────────

UpdateChecker::SemVer UpdateChecker::parseSemVer(const QString& versionStr) {
    QString v = versionStr.trimmed();
    // Strip any non-numeric prefix (e.g. "FFABv1.0.1" → "1.0.1")
    int firstDigit = -1;
    for (int i = 0; i < v.size(); ++i) {
        if (v[i].isDigit()) { firstDigit = i; break; }
    }
    if (firstDigit < 0)
        return {};
    v = v.mid(firstDigit);

    QStringList parts = v.split('.');
    if (parts.size() < 3)
        return {};

    bool ok1, ok2, ok3;
    int major = parts[0].toInt(&ok1);
    int minor = parts[1].toInt(&ok2);
    int patch = parts[2].toInt(&ok3);

    if (!ok1 || !ok2 || !ok3)
        return {};

    return { major, minor, patch, true };
}

bool UpdateChecker::isNewer(const SemVer& remote, const SemVer& local) {
    if (remote.major != local.major) return remote.major > local.major;
    if (remote.minor != local.minor) return remote.minor > local.minor;
    return remote.patch > local.patch;
}

QString UpdateChecker::platformAssetKeyword() const {
#if defined(Q_OS_MACOS)
    return ".dmg";
#elif defined(Q_OS_WINDOWS) && defined(Q_PROCESSOR_ARM_64)
    return "windows-arm64";
#elif defined(Q_OS_WINDOWS) && defined(Q_PROCESSOR_X86_64)
    return "windows-x86_64";
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_ARM_64)
    return "linux-arm64";
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)
    return "linux-x86_64";
#else
    return ".zip";
#endif
}

// ── Check for update ────────────────────────────────────────────────

void UpdateChecker::checkForUpdate(bool force) {
    if (!force && m_lastCheckTime.isValid()) {
        if (m_lastCheckTime.daysTo(QDateTime::currentDateTime()) < 7) {
            emit checkFinished(m_updateAvailable);
            return;
        }
    }

    if (m_apiReply) {
        m_apiReply->abort();
        m_apiReply->deleteLater();
        m_apiReply = nullptr;
    }

    QUrl apiUrl(API_URL);
    QNetworkRequest req{apiUrl};
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QString("FFAB/%1").arg(VERSION_STR));
    req.setRawHeader("Accept", "application/vnd.github.v3+json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    m_apiReply = m_nam->get(req);
    connect(m_apiReply, &QNetworkReply::finished,
            this, &UpdateChecker::onApiReplyFinished);
}

void UpdateChecker::onApiReplyFinished() {
    auto* reply = m_apiReply;
    m_apiReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit checkFailed(reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) {
        emit checkFailed("Invalid response from GitHub API");
        return;
    }

    QJsonObject release = doc.object();
    QString tagName = release["tag_name"].toString();
    auto remote = parseSemVer(tagName);
    auto local  = parseSemVer(VERSION_STR);

    if (!remote.valid) {
        emit checkFailed("Could not parse remote version: " + tagName);
        return;
    }

    m_latestVersion   = tagName;
    m_updateAvailable = remote.valid && local.valid && isNewer(remote, local);
    m_downloadUrl.clear();

    // Find platform-specific asset
    QString keyword = platformAssetKeyword();
    QJsonArray assets = release["assets"].toArray();
    for (const QJsonValue& val : assets) {
        QJsonObject asset = val.toObject();
        QString name = asset["name"].toString();
        if (name.contains(keyword, Qt::CaseInsensitive)) {
            m_downloadUrl = asset["browser_download_url"].toString();
            break;
        }
    }

    m_lastCheckTime = QDateTime::currentDateTime();
    saveCache();

    emit checkFinished(m_updateAvailable);
}

// ── Download ────────────────────────────────────────────────────────

void UpdateChecker::downloadUpdate() {
    if (m_downloadUrl.isEmpty())
        return;

    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }

    QUrl dlUrl(m_downloadUrl);
    QNetworkRequest req{dlUrl};
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QString("FFAB/%1").arg(VERSION_STR));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    m_downloadReply = m_nam->get(req);
    connect(m_downloadReply, &QNetworkReply::downloadProgress,
            this, &UpdateChecker::downloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished,
            this, &UpdateChecker::onDownloadReplyFinished);
}

void UpdateChecker::cancelDownload() {
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
}

void UpdateChecker::onDownloadReplyFinished() {
    auto* reply = m_downloadReply;
    m_downloadReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit downloadFailed(reply->errorString());
        return;
    }

    QString downloadsDir =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString filename = QFileInfo(QUrl(m_downloadUrl).path()).fileName();
    QString savePath = downloadsDir + "/" + filename;

    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit downloadFailed("Could not write to " + savePath);
        return;
    }

    file.write(reply->readAll());
    file.close();

    emit downloadFinished(savePath);
}
