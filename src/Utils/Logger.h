#pragma once
#include <QString>

class Logger {
public:
    static void log(const QString& message);
    static void logError(const QString& message);
    static void logWarning(const QString& message);
};
