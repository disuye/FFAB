#include "LogFileWriter.h"
#include <QDir>
#include <QDebug>

LogFileWriter::LogFileWriter(QObject* parent)
    : QObject(parent)
{
}

LogFileWriter::~LogFileWriter() {
    close();
}

bool LogFileWriter::open(const QString& outputFolder, const QString& context, int fileCount) {
    close();  // Close any previous file

    // Build filename: _FFAB_batch_2026-02-14_00-00-00.txt
    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString("yyyy-MM-dd_HH-mm-ss");
    QString fileName = QString("_FFAB_%1_%2.txt").arg(context, timestamp);

    m_filePath = QDir(outputFolder).filePath(fileName);

    m_file.setFileName(m_filePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "LogFileWriter: Failed to open" << m_filePath;
        m_filePath.clear();
        return false;
    }

    m_stream.setDevice(&m_file);

    // Write header
    m_stream << "# FFAB Log — " << context << " — "
             << now.toString("yyyy-MM-dd HH:mm:ss") << " — "
             << fileCount << " files" << "\n";
    m_stream << "# filename\ttimestamp\toutput" << "\n";
    m_stream.flush();

    qDebug() << "LogFileWriter: Opened" << m_filePath;
    emit logOpened(m_filePath);
    return true;
}

void LogFileWriter::writeLine(const QString& inputFileName, const QString& stderrLine) {
    if (!m_file.isOpen()) return;

    QString trimmed = stderrLine.trimmed();
    if (trimmed.isEmpty()) return;

    QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_stream << inputFileName << "\t" << time << "\t" << trimmed << "\n";
    m_stream.flush();

    emit contentWritten();
}

void LogFileWriter::writeLines(const QString& inputFileName, const QString& stderrOutput) {
    if (!m_file.isOpen()) return;

    const QStringList lines = stderrOutput.split('\n');
    for (const QString& line : lines) {
        writeLine(inputFileName, line);
    }
}

void LogFileWriter::close() {
    if (m_file.isOpen()) {
        m_stream.flush();
        m_file.close();
        qDebug() << "LogFileWriter: Closed" << m_filePath;
        emit logClosed();
    }
}

bool LogFileWriter::isOpen() const {
    return m_file.isOpen();
}

QString LogFileWriter::filePath() const {
    return m_filePath;
}
