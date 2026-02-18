#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include "Core/JobListBuilder.h"

/**
 * BatchConfirmDialog - Confirmation before batch processing starts
 * 
 * Tier 1: Shown for EVERY batch
 *   - Algorithm name
 *   - File counts per input
 *   - Output count
 *   - Estimated size + free space
 *   - Destination folder
 *   - [Cancel] [Process N Files] (button text includes count)
 * 
 * Tier 2: Additional dialog for >10,000 files
 *   - Bold warning
 *   - Count + estimated size + time
 *   - [Cancel] [Yes, Process N Files]
 */
class BatchConfirmDialog : public QDialog {
    Q_OBJECT
    
public:
    // Show Tier 1 confirmation. Returns Accepted or Rejected.
    static int confirmBatch(
        QWidget* parent,
        JobListBuilder::Algorithm algorithm,
        int mainFileCount,
        int aux1FileCount,
        int aux2FileCount,
        int outputCount,
        const QString& sizeEstimate,
        const QString& outputFolder,
        const QString& formatInfo,
        const QString& aux1Name = QString(),   // Display name for aux1 (if broadcast fixed)
        const QString& aux2Name = QString());
    
    // Show Tier 2 confirmation (>10,000 files). Returns Accepted or Rejected.
    static int confirmLargeBatch(
        QWidget* parent,
        int outputCount,
        const QString& sizeEstimate,
        const QString& timeEstimate);

private:
    explicit BatchConfirmDialog(QWidget* parent = nullptr);
};
