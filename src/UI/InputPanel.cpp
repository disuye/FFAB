#include "InputPanel.h"
#include "FileListWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QFileDialog>
#include <QStandardPaths>

InputPanel::InputPanel(QWidget* parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);
    
    // ========== TOP BUTTONS ROW ==========
    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(4);
    
    auto addFolderBtn = new QPushButton("Add Folder");
    auto addFilesBtn = new QPushButton("Add File(s)");
    auto clearBtn = new QPushButton("Clear File List");
    auto rescanBtn = new QPushButton("Rescan Metadata");
    
    connect(addFolderBtn, &QPushButton::clicked, this, &InputPanel::addFolderRequested);
    connect(addFilesBtn, &QPushButton::clicked, this, &InputPanel::addFilesRequested);
    connect(clearBtn, &QPushButton::clicked, this, &InputPanel::clearRequested);
    
    buttonsLayout->addWidget(addFolderBtn);
    buttonsLayout->addWidget(addFilesBtn);
    buttonsLayout->addWidget(clearBtn);
    buttonsLayout->addStretch();
    
    mainLayout->addLayout(buttonsLayout);
    
    // ========== RE/SCAN METADATA CHECKBOX ==========
    auto metaScanLayout = new QHBoxLayout();
    metaScanLayout->setSpacing(4);

    scanMetadataCheckbox = new QCheckBox("FFprobe metadata on import");
    scanMetadataCheckbox->setChecked(true);  // Default: ON
    metaScanLayout->addWidget(rescanBtn);
    metaScanLayout->addWidget(scanMetadataCheckbox);
    metaScanLayout->addStretch();

    mainLayout->addLayout(metaScanLayout);
    
    // ========== FILE LIST LABEL ==========
    mainLayout->addSpacing(12);
    mainLayout->addWidget(new QLabel("File List → [0:a]"));
    
    // ========== FILE LIST WIDGET ==========
    fileListWidget = new FileListWidget();
    mainLayout->addWidget(fileListWidget, 1);  // Stretches
    
    // Connect rescan button to file list widget signal
    connect(rescanBtn, &QPushButton::clicked, fileListWidget, &FileListWidget::rescanRequested);
}

bool InputPanel::shouldScanMetadata() const {
    return scanMetadataCheckbox->isChecked();
}

FileListWidget* InputPanel::getFileListWidget() const {
    return fileListWidget;
}

void InputPanel::setScanMetadata(bool enabled) {
    scanMetadataCheckbox->setChecked(enabled);
}

void InputPanel::setOutputFolder(const QString& folder) {
    if (outputFolderEdit) {
        outputFolderEdit->setText(folder);
        emit outputFolderChanged(folder);
    }
}

void InputPanel::onBrowseOutputFolder() {
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QString folder = QFileDialog::getExistingDirectory(
        this,
        "Select Output Folder",
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folder.isEmpty()) {
        outputFolderEdit->setText(folder);
        emit outputFolderChanged(folder);
    }
}
