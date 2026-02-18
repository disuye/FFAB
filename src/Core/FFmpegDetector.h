#pragma once

#include <QString>
#include <QStringList>

/**
 * FFmpegDetector - Find FFmpeg, FFprobe, and FFplay on the system
 * 
 * Searches common installation directories for:
 * - ffmpeg executable
 * - ffprobe executable
 * - ffplay executable
 * 
 * Supports macOS, Linux, and Windows
 */
class FFmpegDetector {
public:
    struct Paths {
        QString ffmpeg;
        QString ffprobe;
        QString ffplay;
        bool isValid() const { return !ffmpeg.isEmpty() && !ffprobe.isEmpty(); }
    };
    
    // Find FFmpeg, FFprobe, and FFplay on the system
    static Paths detect();
    
    // Find specific executable
    static QString findExecutable(const QString& executableName);
    
private:
    // Get list of candidate paths for an executable
    static QStringList getCandidatePaths(const QString& executableName);
};
