#include "OutputPanel.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>

OutputPanel::OutputPanel(QWidget* parent) : QWidget(parent) {
    setupUI();
}

OutputPanel::~OutputPanel() = default;

void OutputPanel::setupUI() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);
    
    // Output Folder Row
    auto outputLayout = new QHBoxLayout();
    outputLayout->setSpacing(8);
    
    auto outputLabel = new QLabel("Output Folder:");
    outputLabel->setFixedWidth(100);
    outputLayout->addWidget(outputLabel);
    
    outputFolderEdit = new QLineEdit();
    outputFolderEdit->setReadOnly(true);
    outputFolderEdit->setPlaceholderText("Select output folder...");
    outputLayout->addWidget(outputFolderEdit, 1);
    
    browseOutputButton = new QPushButton("Browse...");
    connect(browseOutputButton, &QPushButton::clicked, this, &OutputPanel::onBrowseOutputClicked);
    outputLayout->addWidget(browseOutputButton);
    
    mainLayout->addLayout(outputLayout);
    
    // Placeholder for future codec/naming settings
    auto placeholderLabel = new QLabel("Codec and naming settings coming soon...");
    placeholderLabel->setStyleSheet("QLabel { color: #808080; font-style: italic; }");
    mainLayout->addWidget(placeholderLabel);
}

void OutputPanel::onBrowseOutputClicked() {
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "Select Output Folder",
        outputFolderEdit->text().isEmpty() ? defaultPath : outputFolderEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folderPath.isEmpty()) {
        outputFolderEdit->setText(folderPath);
        emit outputFolderChanged(folderPath);
    }
}

QString OutputPanel::getOutputFolder() const {
    return outputFolderEdit->text();
}

void OutputPanel::setOutputFolder(const QString& path) {
    outputFolderEdit->setText(path);
    if (!path.isEmpty()) {
        emit outputFolderChanged(path);
    }
}
