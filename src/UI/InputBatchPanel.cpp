#include "InputBatchPanel.h"
#include <QHBoxLayout>
#include <QCheckBox>

InputBatchPanel::InputBatchPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

InputBatchPanel::~InputBatchPanel() = default;

void InputBatchPanel::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);
    
    // Scan Metadata Checkbox
    scanMetadataCheckbox = new QCheckBox("FFprobe metadata on import");
    scanMetadataCheckbox->setChecked(true);  // Default: enabled
    mainLayout->addWidget(scanMetadataCheckbox);
    
    // Add Folder & Files Button Row
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    
    addFolderButton = new QPushButton("Add Folder");
    connect(addFolderButton, &QPushButton::clicked, this, &InputBatchPanel::onAddFolderClicked);
    buttonLayout->addWidget(addFolderButton);
    
    addFilesButton = new QPushButton("Add Files");
    connect(addFilesButton, &QPushButton::clicked, this, &InputBatchPanel::onAddFilesClicked);
    buttonLayout->addWidget(addFilesButton);
    
    clearButton = new QPushButton("Clear File List");
    connect(clearButton, &QPushButton::clicked, this, &InputBatchPanel::onClearClicked);
    buttonLayout->addWidget(clearButton);
    
    buttonLayout->addStretch(1);
    
    mainLayout->addLayout(buttonLayout);
}

void InputBatchPanel::onAddFolderClicked() {
    emit addFolderRequested();
}

void InputBatchPanel::onAddFilesClicked() {
    emit addFilesRequested();
}

void InputBatchPanel::onClearClicked() {
    emit clearRequested();
}

bool InputBatchPanel::shouldScanMetadata() const {
    return scanMetadataCheckbox->isChecked();
}
