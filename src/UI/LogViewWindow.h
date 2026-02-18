#pragma once

#include <QWidget>
#include <QString>

class QTextEdit;
class QCloseEvent;
class QTimer;

/**
 * LogViewWindow - Displays FFmpeg log output from the current/last run.
 *
 * Appearance matches CommandViewWindow: dark background, Fira Code monospace,
 * read-only QTextEdit. Supports Cmd+A / Cmd+C for copy.
 *
 * Uses throttled incremental updates: only appends new content since last read,
 * coalesced to at most ~2 updates/second via QTimer.
 */
class LogViewWindow : public QWidget {
    Q_OBJECT
public:
    explicit LogViewWindow(QWidget* parent = nullptr);
    ~LogViewWindow() override = default;

    // Set the log file path and load its contents (full reload, resets read position)
    void setLogFile(const QString& filePath);

    // Schedule an incremental reload (throttled — coalesces rapid calls)
    void reload();

    // Get current file path
    QString logFilePath() const;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void doIncrementalReload();

    QTextEdit* logText = nullptr;
    QString m_logFilePath;
    qint64 m_lastReadPos = 0;    // byte offset for incremental reads
    QTimer* m_reloadTimer = nullptr;
};
