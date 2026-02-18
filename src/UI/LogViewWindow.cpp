#include "LogViewWindow.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>
#include <QSettings>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QTextCursor>

LogViewWindow::LogViewWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("View Log");
    setMinimumSize(500, 120);
    resize(800, 240);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);

    logText = new QTextEdit();
    logText->setReadOnly(true);
    logText->setPlaceholderText("Logging must be enabled: Settings -> Log Level *and* Settings -> Log File '[✓] Save log to file'");

    QFont monoFont("Fira Code");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(12);
    logText->setFont(monoFont);

    logText->setStyleSheet(R"(
        QTextEdit {
            background-color: #2A2A2A;
            color: #FFFFFF;
            border: 1px solid #2A2A2A;
            border-radius: 4px;
            padding: 12px;
        }
    )");

    logText->setLineWrapMode(QTextEdit::WidgetWidth);

    auto infoLabel = new QLabel("Tab-separated: filename → timestamp → output. Cmd+A to select all, Cmd+C to copy.");
    infoLabel->setStyleSheet("padding: 0; margin: 0; font-size: 9px; color: #808080");
    layout->addWidget(infoLabel);

    layout->addWidget(logText);

    // Throttle timer — coalesces rapid reload() calls into one actual read
    m_reloadTimer = new QTimer(this);
    m_reloadTimer->setSingleShot(true);
    m_reloadTimer->setInterval(500);  // max 2 updates/sec
    connect(m_reloadTimer, &QTimer::timeout, this, &LogViewWindow::doIncrementalReload);

    // Restore geometry
    QSettings settings;
    QByteArray geometry = settings.value("logWindow/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void LogViewWindow::setLogFile(const QString& filePath) {
    m_logFilePath = filePath;
    m_lastReadPos = 0;
    m_reloadTimer->stop();

    // Full load of the file
    logText->clear();
    if (m_logFilePath.isEmpty()) return;

    QFile file(m_logFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logText->setPlainText("(Could not open log file: " + m_logFilePath + ")");
        return;
    }

    QTextStream in(&file);
    logText->setPlainText(in.readAll());
    m_lastReadPos = file.pos();

    // Scroll to bottom on initial load
    QScrollBar* scrollBar = logText->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void LogViewWindow::reload() {
    // Coalesce: if timer is already running, the pending timeout will handle it.
    // Otherwise start it so we update within 500ms.
    if (!m_reloadTimer->isActive()) {
        m_reloadTimer->start();
    }
}

void LogViewWindow::doIncrementalReload() {
    if (m_logFilePath.isEmpty()) return;

    QFile file(m_logFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    qint64 fileSize = file.size();
    if (fileSize <= m_lastReadPos) return;  // nothing new

    file.seek(m_lastReadPos);
    QTextStream in(&file);
    QString newContent = in.readAll();
    m_lastReadPos = file.pos();

    if (newContent.isEmpty()) return;

    // Check if user is scrolled to bottom before appending
    QScrollBar* scrollBar = logText->verticalScrollBar();
    bool wasAtBottom = (scrollBar->value() >= scrollBar->maximum() - 10);

    // Append only the new content (avoids re-processing entire document)
    QTextCursor cursor = logText->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(newContent);

    // Auto-scroll to bottom if user was already at bottom
    if (wasAtBottom) {
        scrollBar->setValue(scrollBar->maximum());
    }
}

QString LogViewWindow::logFilePath() const {
    return m_logFilePath;
}

void LogViewWindow::closeEvent(QCloseEvent* event) {
    QSettings settings;
    settings.setValue("logWindow/geometry", saveGeometry());
    settings.setValue("logWindow/visible", false);
    QWidget::closeEvent(event);
}
