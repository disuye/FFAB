#pragma once
#include <QString>

class FilterChain;

class OperationPreview {
public:
    bool previewFile(const QString& inputFile, const QString& outputFile, FilterChain* chain);
    bool isProcessing() const { return processing; }
    QString getLastError() const { return lastError; }
private:
    bool processing = false;
    QString lastError;
};
