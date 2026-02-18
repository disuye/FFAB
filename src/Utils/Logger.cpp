#include "Logger.h"
#include <QDebug>

void Logger::log(const QString& message) {
    qDebug() << "[FFAB]" << message;
}

void Logger::logError(const QString& message) {
    qWarning() << "[FFAB ERROR]" << message;
}

void Logger::logWarning(const QString& message) {
    qWarning() << "[FFAB WARNING]" << message;
}
