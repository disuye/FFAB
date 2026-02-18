#include "FFmpegDetector.h"
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

FFmpegDetector::Paths FFmpegDetector::detect() {
    Paths paths;
    paths.ffmpeg  = findExecutable("ffmpeg");
    paths.ffprobe = findExecutable("ffprobe");
    paths.ffplay  = findExecutable("ffplay");
    
    if (paths.isValid()) {
        qDebug() << "FFmpeg found at:" << paths.ffmpeg;
        qDebug() << "FFprobe found at:" << paths.ffprobe;
        qDebug() << "FFplay found at:" << (paths.ffplay.isEmpty() ? "NOT FOUND" : paths.ffplay);
    } else {
        qDebug() << "FFmpeg/FFprobe not found";
        if (paths.ffmpeg.isEmpty())  qDebug() << "  - ffmpeg: NOT FOUND";
        if (paths.ffprobe.isEmpty()) qDebug() << "  - ffprobe: NOT FOUND";
        if (paths.ffplay.isEmpty())  qDebug() << "  - ffplay: NOT FOUND";
    }
    
    return paths;
}

QString FFmpegDetector::findExecutable(const QString& executableName) {
    QStringList candidates = getCandidatePaths(executableName);
    
    for (const QString& path : candidates) {
        bool exists = QFile::exists(path);
        qDebug() << "[FFmpegDetector] checking:" << path << "->" << (exists ? "EXISTS" : "miss");
        if (exists) {
            return path;
        }
    }
    
    qDebug() << "[FFmpegDetector]" << executableName << "not found in any candidate path";
    return QString();
}

QStringList FFmpegDetector::getCandidatePaths(const QString& executableName) {
    QStringList candidates;

    qDebug() << "[FFmpegDetector] getCandidatePaths() for:" << executableName;

    // 0. User-configured path from QSettings (highest priority)
    {
        QSettings settings;
        QString savedPath = settings.value("processing/ffmpegPath").toString().trimmed();
        qDebug() << "[FFmpegDetector] QSettings raw value:" << savedPath;
        if (!savedPath.isEmpty()) {
            QFileInfo fi(savedPath);
            if (fi.isDir()) {
                candidates << savedPath + "/" + executableName;
            } else if (fi.fileName() == executableName) {
                candidates << savedPath;
            } else {
                candidates << fi.absolutePath() + "/" + executableName;
            }
            qDebug() << "[FFmpegDetector] QSettings candidate:" << candidates.last()
                       << "exists?" << QFile::exists(candidates.last());
        }
    }

    // 1. FFAB's own download directory
    {
        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        qDebug() << "[FFmpegDetector] AppDataLocation:" << appData;
        QString ffabDir = appData + "/ffmpeg";
        QString candidate = ffabDir + "/" + executableName;
        candidates << candidate;
        qDebug() << "[FFmpegDetector] FFAB dir candidate:" << candidate
                   << "exists?" << QFile::exists(candidate);

        QDir dir(ffabDir);
        if (dir.exists()) {
            for (const QFileInfo& entry : dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                candidates << entry.absoluteFilePath() + "/" + executableName;
                candidates << entry.absoluteFilePath() + "/bin/" + executableName;
            }
        }
    }
    
#ifdef Q_OS_MAC
    // macOS common locations
    candidates << "/usr/local/bin/" + executableName          // Homebrew Intel
               << "/opt/homebrew/bin/" + executableName       // Homebrew Apple Silicon
               << "/opt/local/bin/" + executableName          // MacPorts
               << "/usr/bin/" + executableName;               // System
#endif
    
#ifdef Q_OS_LINUX
    // Linux common locations
    candidates << "/usr/bin/" + executableName
               << "/usr/local/bin/" + executableName
               << "/snap/bin/" + executableName
               << "/opt/ffmpeg/bin/" + executableName;
#endif
    
#ifdef Q_OS_WIN
    // Windows common locations
    QString exeName = executableName + ".exe";
    candidates << QCoreApplication::applicationDirPath() + "/" + exeName
               << QCoreApplication::applicationDirPath() + "/../" + exeName
               << "C:/ffmpeg/bin/" + exeName
               << "C:/Program Files/ffmpeg/bin/" + exeName
               << "C:/Program Files (x86)/ffmpeg/bin/" + exeName;
#endif
    
    // Try just the executable name (searches PATH)
    candidates << executableName;
    
    return candidates;
}
