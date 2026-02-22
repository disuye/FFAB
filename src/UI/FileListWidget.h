#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QList>
#include <QString>

/**
 * FileListWidget - Display scanned audio files with metadata
 * 
 * Features:
 * - Zebra-striped background (light/dark mode aware)
 * - Checkbox to enable/disable files for processing
 * - Metadata columns: filename, format, duration, sample rate, channels, bitrate
 * - Native macOS appearance
 * - SMPLR-style aesthetic
 */
class FileListWidget : public QWidget {
    Q_OBJECT
    
public:
    struct AudioFileInfo {
        QString filePath;
        QString fileName;
        QString format;        // e.g. "WAV", "MP3", "FLAC"
        QString duration;      // e.g. "00:03:45"
        int sampleRate;        // e.g. 48000 Hz
        int bitsPerSample;     // e.g. 24 bit     
        int channels;          // e.g. 2 (stereo)
        int bitrate;           // e.g. 320 (kbps)
        bool enabled;          // Whether to process this file
    };
    
    explicit FileListWidget(QWidget* parent = nullptr);
    ~FileListWidget() override;
    
    // Add files to the list
    void addFile(const AudioFileInfo& fileInfo);
    void addFiles(const QList<AudioFileInfo>& files);
    
    // Clear all files
    void clearFiles();
    
    // Delete currently selected/highlighted files
    void deleteSelectedFiles();
    
    // Rescan metadata for all files
    void rescanMetadata();
    
    // Get list of enabled files for processing
    QList<AudioFileInfo> getEnabledFiles() const;
    
    // Get currently selected/highlighted files
    QList<AudioFileInfo> getSelectedFiles() const;
    
    // Get all files
    QList<AudioFileInfo> getAllFiles() const;
    
    // Update background colors (public so MainWindow can call on theme change)
    void updateBackground();
    
signals:
    void fileSelectionChanged();  // Emitted when checkboxes change
    void rescanRequested();  // Emitted when user clicks Rescan Metadata button
    void previewRequested();  // Emitted on double-click: generate preview + auto-play
    
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    
private:
    void setupUI();
    void populateTable();
    void populateSingleRow(int index);
    
    QTableWidget* tableWidget;
    QList<AudioFileInfo> files;
};