#include "OperationPreview.h"
#include "FilterChain.h"
#include <QDebug>

bool OperationPreview::previewFile(const QString& inputFile,
                                    const QString& outputFile,
                                    FilterChain* chain) {
    qDebug() << "Preview:" << inputFile << "->" << outputFile;
    return true;
}
