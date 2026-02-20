#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject* parent = nullptr);

    void checkForUpdate(bool force = false);

    bool updateAvailable() const { return m_updateAvailable; }
    QString latestVersion() const { return m_latestVersion; }
    QString downloadUrl() const { return m_downloadUrl; }
    QDateTime lastCheckTime() const { return m_lastCheckTime; }

    bool weeklyCheckEnabled() const;
    void setWeeklyCheckEnabled(bool enabled);

    void downloadUpdate();
    void cancelDownload();

signals:
    void checkFinished(bool updateAvailable);
    void checkFailed(const QString& errorMessage);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(const QString& savedPath);
    void downloadFailed(const QString& errorMessage);

private slots:
    void onApiReplyFinished();
    void onDownloadReplyFinished();

private:
    struct SemVer {
        int major = 0, minor = 0, patch = 0;
        bool valid = false;
    };

    static SemVer parseSemVer(const QString& versionStr);
    static bool isNewer(const SemVer& remote, const SemVer& local);
    QString platformAssetKeyword() const;

    void loadCache();
    void saveCache();

    QNetworkAccessManager* m_nam;
    QNetworkReply* m_apiReply = nullptr;
    QNetworkReply* m_downloadReply = nullptr;

    bool    m_updateAvailable = false;
    QString m_latestVersion;
    QString m_downloadUrl;
    QDateTime m_lastCheckTime;
};
