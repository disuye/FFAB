#include "AudioInputFilter.h"
#include "FileListWidget.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QFont>

AudioInputFilter::AudioInputFilter() {
    position = Position::MIDDLE;
}

QWidget* AudioInputFilter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("audioInputFilterTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(4);

    // Title with input label
    auto titleLayout = new QHBoxLayout();
    auto titleLabel = new QLabel("Audio Input (Sidechain / Additional Source)");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    titleLayout->addWidget(titleLabel);
    
    titleLayout->addStretch();
    
    // Show which label this creates
    labelDisplay = new QLabel(getInputLabel());
    QFont monoFont("Courier");
    monoFont.setBold(true);
    labelDisplay->setFont(monoFont);
    labelDisplay->setStyleSheet("QLabel { background-color: rgba(128, 128, 128, 0.1); padding: 4px; border-radius: 4px; }");
    titleLayout->addWidget(labelDisplay);
    
    mainLayout->addLayout(titleLayout);

    // ========== TOP BUTTONS ROW ==========
    auto buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(4);
    
    addFolderBtn = new QPushButton("Add Folder");
    addFilesBtn = new QPushButton("Add File(s)");
    clearBtn = new QPushButton("Clear File List");
    rescanBtn = new QPushButton("Rescan Metadata");
    
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
    filesListLabel = new QLabel(QString("File List → %1").arg(getInputLabel()));
    mainLayout->addWidget(filesListLabel);
    
    // ========== FILE LIST WIDGET ==========
    fileListWidget = new FileListWidget();
    mainLayout->addWidget(fileListWidget, 1);  // Takes remaining space
    
    // Connect file list changes
    connect(fileListWidget, &FileListWidget::fileSelectionChanged,
            this, &AudioInputFilter::filesChanged);
    
    // Connect rescan button to file list widget signal
    connect(rescanBtn, &QPushButton::clicked, fileListWidget, &FileListWidget::rescanRequested);
    
    // Note: Button signal connections will be handled by MainWindow
    // (add/clear actions need access to AudioFileScanner, etc.)

    // Collapsible help section at bottom
    auto helpSection = new CollapsibleHelpSection(
        "<b>What is this?</b><br>"
        "This Audio Input filter injects a secondary audio stream into your processing chain. "
        "Files loaded here become available as <b>" + getInputLabel() + "</b> to downstream filters.<br><br>"
        
        "<b>Common uses:</b><br>"
        "• <b>Sidechain Compression/Gating:</b> Use kick drum to duck music (sidechaincompress, sidechaingate)<br>"
        "• <b>Impulse Response:</b> Apply reverb or cabinet simulation (afir filter)<br>"
        "• <b>Channel Merging:</b> Combine mono files into stereo/multichannel (amerge filter)<br><br>"
        
        "<b>File pairing:</b><br>"
        "When processing multiple files, FFAB pairs primary files with sidechain files:<br>"
        "• <b>Many-to-many:</b> Sequential pairing (file 1 → sidechain 1, file 2 → sidechain 2, etc.)<br>"
        "• <b>One-to-many:</b> Single sidechain file used for all primary files<br>"
        "• <b>All combinations:</b> Cartesian product (every combination, opt-in)"
    );
    mainLayout->addWidget(helpSection);

    return parametersWidget;
}

QString AudioInputFilter::buildFFmpegFlags() const {
    // AudioInputFilter doesn't add to -filter_complex
    // It signals FFmpegCommandBuilder to add an additional -i input
    return QString();
}

QString AudioInputFilter::getInputLabel() const {
    return QString("[%1:a]").arg(inputIndex);
}

void AudioInputFilter::setInputIndex(int index) {
    inputIndex = index;
    refreshLabels();
}

void AudioInputFilter::refreshLabels() {
    // Update the blue label display in the title area
    if (labelDisplay) {
        labelDisplay->setText(getInputLabel());
    }
    // Update the "Files for [X:a]:" label
    if (filesListLabel) {
        filesListLabel->setText(QString("File List → %1").arg(getInputLabel()));
    }
}

bool AudioInputFilter::shouldScanMetadata() const {
    return scanMetadataCheckbox ? scanMetadataCheckbox->isChecked() : true;
}

QStringList AudioInputFilter::getEnabledFilePaths() const {
    if (!fileListWidget) return QStringList();
    
    QStringList paths;
    auto files = fileListWidget->getEnabledFiles();
    for (const auto& fileInfo : files) {
        paths.append(fileInfo.filePath);
    }
    return paths;
}

QString AudioInputFilter::getSelectedFilePath() const {
    if (!fileListWidget) {
        return "";
    }
    
    // First, try to get highlighted/selected files
    auto selectedFiles = fileListWidget->getSelectedFiles();
    if (!selectedFiles.isEmpty()) {
        return selectedFiles.first().filePath;
    }
    
    // Fall back to first enabled file if nothing is highlighted
    auto enabledFiles = fileListWidget->getEnabledFiles();
    if (!enabledFiles.isEmpty()) {
        return enabledFiles.first().filePath;
    }
    
    // No files available...
    return "";
}

void AudioInputFilter::toJSON(QJsonObject& json) const {
    json["type"] = "audio-input";
    json["inputIndex"] = inputIndex;
    
    // NOTE: File lists are now handled by PresetManager, not by the filter itself
    // This prevents duplicate serialization and allows PresetManager to handle
    // missing files gracefully during load
}

void AudioInputFilter::fromJSON(const QJsonObject& json) {
    inputIndex = json["inputIndex"].toInt(1);
    
    // Update UI if it exists
    if (labelDisplay) {
        labelDisplay->setText(getInputLabel());
    }
    
    // NOTE: File lists are now loaded by PresetManager, not by the filter itself
    // This prevents duplicate deserialization and allows PresetManager to handle
    // missing files with proper reporting
}
