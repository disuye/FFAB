#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDateTime>

/**
 * LogFileWriter - Captures FFmpeg stderr output to a tab-separated TXT file.
 *
 * Usage:
 *   1. Call open() at batch/preview start
 *   2. Call writeLine() for each stderr line received from FFmpeg
 *   3. Call close() at batch/preview end
 *
 * Line format:  filename\tHH:mm:ss\tstderr_line
 */
class LogFileWriter : public QObject {
    Q_OBJECT

public:
    explicit LogFileWriter(QObject* parent = nullptr);
    ~LogFileWriter() override;

    // Open a new log file in the given output folder.
    // context: "batch" or "preview"
    // fileCount: number of files in the run (for header)
    // Returns true if file opened successfully.
    bool open(const QString& outputFolder, const QString& context, int fileCount);

    // Write a single stderr line to the log, prefixed with filename and timestamp.
    // inputFileName: just the filename (not full path), e.g. "drumloop.wav"
    // stderrLine: one line of FFmpeg stderr output (will be trimmed)
    void writeLine(const QString& inputFileName, const QString& stderrLine);

    // Write multiple lines (convenience — splits on \n and calls writeLine for each)
    void writeLines(const QString& inputFileName, const QString& stderrOutput);

    // Close the log file. Safe to call multiple times.
    void close();

    // Returns true if a log file is currently open
    bool isOpen() const;

    // Returns the full path of the current/last log file (for View Log window)
    QString filePath() const;

signals:
    // Emitted whenever new content is written (for live View Log updates)
    void contentWritten();

    // Emitted when the log file is opened (path is the full file path)
    void logOpened(const QString& path);

    // Emitted when the log file is closed
    void logClosed();

private:
    QFile m_file;
    QTextStream m_stream;
    QString m_filePath;
};
