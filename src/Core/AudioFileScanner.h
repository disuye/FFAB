#pragma once

#include <QString>
#include <QStringList>
#include <QDir>
#include <QProcess>
#include <functional>
#include "UI/FileListWidget.h"

/**
 * AudioFileScanner - Scan folders for audio files and extract metadata
 * 
 * Uses FFprobe to extract:
 * - Format (WAV, MP3, FLAC, etc.)
 * - Duration
 * - Sample rate
 * - Channels
 * - Bitrate
 * 
 * Supported formats: .wav, .mp3, .flac, .aiff, .m4a, .ogg, .opus
 */
class AudioFileScanner {
public:
    // Progress callback: void callback(int current, int total)
    using ProgressCallback = std::function<void(int, int)>;
    
    // Scan a folder for audio files
    static QList<FileListWidget::AudioFileInfo> scanFolder(const QString& folderPath, 
                                                           const QString& ffprobePath = "ffprobe",
                                                           ProgressCallback progressCallback = nullptr);
    
    // Extract metadata from a single file using FFprobe
    static FileListWidget::AudioFileInfo extractMetadata(const QString& filePath, 
                                                         const QString& ffprobePath = "ffprobe");
    
    // Get list of supported audio extensions
    static QStringList getSupportedExtensions();
    
private:
    // Parse FFprobe JSON output
    static FileListWidget::AudioFileInfo parseFFprobeOutput(const QString& filePath, const QString& output);
    
    // Format duration from seconds to HH:MM:SS
    static QString formatDuration(double seconds);
};
