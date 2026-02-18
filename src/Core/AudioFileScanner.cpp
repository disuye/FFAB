#include "AudioFileScanner.h"
#include <QFileInfo>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QStringList AudioFileScanner::getSupportedExtensions() {
    return {
        "*.wav", "*.WAV",
        "*.mp3", "*.MP3",
        "*.flac", "*.FLAC",
        "*.aiff", "*.AIFF", "*.aif", "*.AIF",
        "*.m4a", "*.M4A",
        "*.ogg", "*.OGG",
        "*.opus", "*.OPUS",
        "*.wma", "*.WMA",
        "*.aac", "*.AAC"
    };
}

QList<FileListWidget::AudioFileInfo> AudioFileScanner::scanFolder(const QString& folderPath,
                                                                  const QString& ffprobePath,
                                                                  ProgressCallback progressCallback) {
    QList<FileListWidget::AudioFileInfo> files;
    
    QDir dir(folderPath);
    if (!dir.exists()) {
        qWarning() << "Folder does not exist:" << folderPath;
        return files;
    }
    
    // First, count total files for progress
    QStringList filters = getSupportedExtensions();
    QDirIterator countIt(folderPath, filters, QDir::Files, QDirIterator::Subdirectories);
    int totalFiles = 0;
    while (countIt.hasNext()) {
        countIt.next();
        totalFiles++;
    }
    
    qDebug() << "Found" << totalFiles << "audio files to scan";
    
    // Now scan with progress updates
    QDirIterator it(folderPath, filters, QDir::Files, QDirIterator::Subdirectories);
    int currentFile = 0;
    
    while (it.hasNext()) {
        QString filePath = it.next();
        FileListWidget::AudioFileInfo audioInfo = extractMetadata(filePath, ffprobePath);
        files.append(audioInfo);
        
        currentFile++;
        if (progressCallback) {
            progressCallback(currentFile, totalFiles);
        }
    }
    
    qDebug() << "Scanned" << folderPath << "(recursive) - Loaded" << files.size() << "audio files";
    
    return files;
}

FileListWidget::AudioFileInfo AudioFileScanner::extractMetadata(const QString& filePath,
                                                               const QString& ffprobePath) {
    FileListWidget::AudioFileInfo info;
    info.filePath = filePath;
    info.fileName = QFileInfo(filePath).fileName();
    info.enabled = true;  // Enabled by default
    
    // Default values
    info.format = QFileInfo(filePath).suffix().toUpper();
    info.duration = "00:00:00";
    info.sampleRate = 0;
    info.channels = 0;
    info.bitrate = 0;
    info.bitsPerSample = 0;
    
    // Skip FFprobe if path is empty (fast mode)
    if (ffprobePath.isEmpty()) {
        return info;
    }
    
    // Run FFprobe to get metadata
    QProcess ffprobe;
    QStringList args = {
        "-v", "quiet",
        "-print_format", "json",
        "-show_format",
        "-show_streams",
        filePath
    };
    
    ffprobe.start(ffprobePath, args);
    
    if (!ffprobe.waitForStarted()) {
        qWarning() << "Failed to start ffprobe for:" << filePath;
        return info;
    }
    
    if (!ffprobe.waitForFinished(5000)) {  // 5 second timeout
        qWarning() << "FFprobe timeout for:" << filePath;
        ffprobe.kill();
        return info;
    }
    
    if (ffprobe.exitCode() != 0) {
        qWarning() << "FFprobe error for:" << filePath;
        qWarning() << ffprobe.readAllStandardError();
        return info;
    }
    
    QString output = ffprobe.readAllStandardOutput();
    return parseFFprobeOutput(filePath, output);
}

FileListWidget::AudioFileInfo AudioFileScanner::parseFFprobeOutput(const QString& filePath, const QString& output) {
    FileListWidget::AudioFileInfo info;
    info.filePath = filePath;
    info.fileName = QFileInfo(filePath).fileName();
    info.enabled = true;
    
    // Default values
    info.format = QFileInfo(filePath).suffix().toUpper();
    info.duration = "00:00:00";
    info.sampleRate = 0;
    info.channels = 0;
    info.bitrate = 0;
    info.bitsPerSample = 0;
    
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Failed to parse FFprobe JSON for:" << filePath;
        return info;
    }
    
    QJsonObject root = doc.object();
    
    // Get format info
    if (root.contains("format")) {
        QJsonObject format = root["format"].toObject();
        
        // Duration
        if (format.contains("duration")) {
            double durationSecs = format["duration"].toString().toDouble();
            info.duration = formatDuration(durationSecs);
        }
        
        // Bitrate (overall)
        if (format.contains("bit_rate")) {
            QJsonValue bitrateValue = format["bit_rate"];
            int bitrate = 0;
            
            if (bitrateValue.isString()) {
                bitrate = bitrateValue.toString().toInt();
            } else if (bitrateValue.isDouble()) {
                bitrate = static_cast<int>(bitrateValue.toDouble());
            }
            
            info.bitrate = bitrate / 1000;  // Convert to kbps
        }
        
        // Format name
        if (format.contains("format_name")) {
            QString formatName = format["format_name"].toString();
            if (!formatName.isEmpty()) {
                info.format = formatName.split(",").first().toUpper();
            }
        }
    }
    
    // Get first audio stream info
    if (root.contains("streams")) {
        QJsonArray streams = root["streams"].toArray();
        
        for (const QJsonValue& streamVal : streams) {
            QJsonObject stream = streamVal.toObject();
            
            // Only process audio streams
            if (stream["codec_type"].toString() == "audio") {
                // Sample rate
                if (stream.contains("sample_rate")) {
                    info.sampleRate = stream["sample_rate"].toString().toInt();
                }
                
                // Bit Depth - try bits_per_raw_sample first, fall back to bits_per_sample
                if (stream.contains("bits_per_raw_sample") && !stream["bits_per_raw_sample"].toString().isEmpty()) {
                    info.bitsPerSample = stream["bits_per_raw_sample"].toString().toInt();
                } else if (stream.contains("bits_per_sample")) {
                    info.bitsPerSample = stream["bits_per_sample"].toInt();
                }
                
                // Channels
                if (stream.contains("channels")) {
                    info.channels = stream["channels"].toInt();
                }

                // Codec name (for better format detection)
                if (stream.contains("codec_name")) {
                    QString codec = stream["codec_name"].toString().toUpper();
                    // Override format with codec if more specific
                    if (codec == "PCM_S16LE" || codec == "PCM_S24LE" || codec == "PCM_S32LE") {
                        info.format = "WAV";
                    } else if (codec == "FLAC") {
                        info.format = "FLAC";
                    } else if (codec == "MP3") {
                        info.format = "MP3";
                    } else if (codec == "AAC") {
                        info.format = "AAC";
                    }
                }
                
                // Break after first audio stream
                break;
            }
        }
    }
    
    return info;
}

QString AudioFileScanner::formatDuration(double seconds) {
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int secs = static_cast<int>(seconds) % 60;
    
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}
