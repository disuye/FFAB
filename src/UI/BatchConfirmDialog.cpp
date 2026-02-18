#include "BatchConfirmDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStorageInfo>
#include <QLocale>
#include <QStyle>
#include <QFont>

BatchConfirmDialog::BatchConfirmDialog(QWidget* parent)
    : QDialog(parent)
{
}

int BatchConfirmDialog::confirmBatch(
    QWidget* parent,
    JobListBuilder::Algorithm algorithm,
    int mainFileCount,
    int aux1FileCount,
    int aux2FileCount,
    int outputCount,
    const QString& sizeEstimate,
    const QString& outputFolder,
    const QString& formatInfo,
    const QString& aux1Name,
    const QString& aux2Name) {
    
    QDialog dialog(parent);
    dialog.setWindowTitle("Confirm Batch Processing");
    dialog.setMinimumWidth(420);
    
    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);
    
    // Algorithm name
    auto algoInfo = JobListBuilder::getAlgorithmInfo(algorithm);
    auto* algoLabel = new QLabel(QString("<b>Algorithm %1: %2</b>")
        .arg(static_cast<int>(algorithm))
        .arg(algoInfo.name));
    algoLabel->setStyleSheet("font-size: 13px;");
    layout->addWidget(algoLabel);
    
    layout->addSpacing(4);
    
    // Input summary
    auto* inputSection = new QLabel();
    QString inputText;
    
    // Main input — always shown
    QLocale locale;
    inputText += QString("<b>■</b> Main Input: %1 files<br>").arg(locale.toString(mainFileCount));
    
    // Aux 1
    if (aux1FileCount > 0 || !aux1Name.isEmpty()) {
        if (algorithm == JobListBuilder::Algorithm::BroadcastFixed && !aux1Name.isEmpty()) {
            inputText += QString("<b>●</b> Audio Input #1: \"%1\"<br>").arg(aux1Name);
        } else if (algorithm == JobListBuilder::Algorithm::BroadcastRandom) {
            inputText += QString("<b>▲</b> Audio Input #1: %1 files (random)<br>").arg(locale.toString(aux1FileCount));
        } else {
            inputText += QString("<b>■</b> Audio Input #1: %1 files<br>").arg(locale.toString(aux1FileCount));
        }
    }
    
    // Aux 2
    if (aux2FileCount > 0) {
        inputText += QString("<b>■</b> Audio Input #2: %1 files<br>").arg(locale.toString(aux2FileCount));
    }
    
    inputSection->setText(inputText);
    inputSection->setTextFormat(Qt::RichText);
    layout->addWidget(inputSection);
    
    // Separator
    auto* sep = new QLabel();
    sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    sep->setFixedHeight(2);
    layout->addWidget(sep);
    
    // Output info
    QString outputText;
    outputText += QString("Output: <b>%1 files</b><br>").arg(locale.toString(outputCount));
    outputText += QString("Est. size: <b>%1</b>").arg(sizeEstimate);
    if (!formatInfo.isEmpty()) {
        outputText += QString(" (%1)").arg(formatInfo);
    }
    outputText += "<br>";
    outputText += QString("Destination: %1<br>").arg(outputFolder);
    
    // Free space check
    QStorageInfo storage(outputFolder);
    if (storage.isValid()) {
        qint64 freeBytes = storage.bytesAvailable();
        outputText += QString("Free space: %1").arg(JobListBuilder::formatBytes(freeBytes));
    }
    
    auto* outputLabel = new QLabel(outputText);
    outputLabel->setTextFormat(Qt::RichText);
    outputLabel->setWordWrap(true);
    layout->addWidget(outputLabel);
    
    layout->addSpacing(8);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setMinimumWidth(80);
    buttonLayout->addWidget(cancelBtn);
    
    auto* processBtn = new QPushButton(QString("Process %1 Files").arg(locale.toString(outputCount)));
    processBtn->setDefault(true);
    processBtn->setMinimumWidth(140);
    buttonLayout->addWidget(processBtn);
    
    layout->addLayout(buttonLayout);
    
    // Connections
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(processBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    return dialog.exec();
}

int BatchConfirmDialog::confirmLargeBatch(
    QWidget* parent,
    int outputCount,
    const QString& sizeEstimate,
    const QString& timeEstimate) {
    
    QDialog dialog(parent);
    dialog.setWindowTitle("Large Batch Warning");
    dialog.setMinimumWidth(420);
    
    auto* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(12);
    layout->setContentsMargins(20, 16, 20, 16);
    
    QLocale locale;
    
    // Warning icon + title
    auto* warningLabel = new QLabel(
        QString("⚠️ <b>This batch will produce %1 files</b>")
            .arg(locale.toString(outputCount)));
    QFont warningFont = warningLabel->font();
    warningFont.setPointSize(warningFont.pointSize() + 2);
    warningLabel->setFont(warningFont);
    warningLabel->setTextFormat(Qt::RichText);
    layout->addWidget(warningLabel);
    
    layout->addSpacing(8);
    
    // Details
    QString detailText;
    detailText += QString("Estimated size: <b>%1</b><br>").arg(sizeEstimate);
    if (!timeEstimate.isEmpty()) {
        detailText += QString("Estimated time: <b>%1</b><br>").arg(timeEstimate);
    }
    detailText += "<br>Are you absolutely sure?";
    
    auto* detailLabel = new QLabel(detailText);
    detailLabel->setTextFormat(Qt::RichText);
    detailLabel->setWordWrap(true);
    layout->addWidget(detailLabel);
    
    layout->addSpacing(12);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setMinimumWidth(80);
    buttonLayout->addWidget(cancelBtn);
    
    // Button text includes count — forces user to read the number
    auto* confirmBtn = new QPushButton(
        QString("Yes, Process %1 Files").arg(locale.toString(outputCount)));
    confirmBtn->setMinimumWidth(180);
    buttonLayout->addWidget(confirmBtn);
    
    layout->addLayout(buttonLayout);
    
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(confirmBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    return dialog.exec();
}
