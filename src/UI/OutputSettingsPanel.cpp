#include "OutputSettingsPanel.h"
#include "Core/Preferences.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

OutputSettingsPanel::OutputSettingsPanel(QWidget* parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);
    
    // Form layout for settings
    auto formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    
    // ========== OUTPUT FORMAT ==========
    formatCombo = new QComboBox();
    formatCombo->addItems({
        "WAV",
        "AIFF", 
        "FLAC", 
        "MP3", 
        "AAC", 
        "OGG", 
        "Opus" 
    });
    formatCombo->setCurrentIndex(0); // WAV default

    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutputSettingsPanel::settingsChanged);
    
    formLayout->addRow("Output Format", formatCombo);
    
    // ========== SAMPLE RATE ==========
    sampleRateComboBox = new QComboBox();
    sampleRateComboBox->addItems({
        "192000",
        "176400", 
        "96000", 
        "88200", 
        "48000", 
        "44100", 
        "24000", 
        "22050", 
        "32000", 
        "16000", 
        "12000", 
        "11025", 
        "10000", 
        "8000"
    });
    sampleRateComboBox->setCurrentIndex(4); // 48kHz default

    connect(sampleRateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutputSettingsPanel::settingsChanged);
    
    formLayout->addRow("Sample Rate (Hz)", sampleRateComboBox);
    
    // ========== BIT DEPTH ==========
    bitDepthComboBox = new QComboBox();
    bitDepthComboBox->addItems({
        "32",
        "24",
        "16",
        "12",
        "10",
        "8"
    });
    bitDepthComboBox->setCurrentIndex(1); // 24bit default
    
    connect(bitDepthComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutputSettingsPanel::settingsChanged);
    
    formLayout->addRow("Bit Depth", bitDepthComboBox);
    
    // ========== BITRATE ==========
    bitrateComboBox = new QComboBox();
    bitrateComboBox->addItems({
        "320",
        "256",
        "192",
        "160",
        "128",
        "96",
        "64",
        "32"
    });
    bitrateComboBox->setCurrentIndex(0); // 24bit default
    
    connect(bitrateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutputSettingsPanel::settingsChanged);
    
    formLayout->addRow("Bitrate (kbps)", bitrateComboBox);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();  // Push everything to top

    // ============ OUTPUT PANEL ==========
    // Batch Admin Section
    mainLayout->addWidget(new QLabel("Batch Admin"));

    auto outputLayout = new QHBoxLayout();
    outputLayout->addWidget(new QLabel("Output Folder:"));

    outputFolderEdit = new QLineEdit();
    outputFolderEdit->setPlaceholderText("Select output folder ...");
    outputFolderEdit->setReadOnly(true);
    outputLayout->addWidget(outputFolderEdit, 1);

    auto browseBtn = new QPushButton("Browse ...");
    connect(browseBtn, &QPushButton::clicked, this, &OutputSettingsPanel::onBrowseOutputFolder);
    outputLayout->addWidget(browseBtn);

    mainLayout->addLayout(outputLayout);

    auto placeholderLabel = new QLabel("<i>Codec and naming settings coming soon...</i>");
    placeholderLabel->setStyleSheet("color: #808080;");
    mainLayout->addWidget(placeholderLabel);

    mainLayout->addStretch();  // Push everything to top
}

QString OutputSettingsPanel::getOutputFormat() const {
    return formatCombo->currentText();
}

int OutputSettingsPanel::getSampleRate() const {
    return sampleRateComboBox->currentText().toInt();
}

int OutputSettingsPanel::getBitDepth() const {
    return bitDepthComboBox->currentText().toInt();
}

int OutputSettingsPanel::getBitrate() const {
    return bitrateComboBox->currentText().toInt();
}

// OUTPUT METHODS

QString OutputSettingsPanel::getOutputFolder() const {
    return outputFolderEdit->text();
}

void OutputSettingsPanel::setOutputFormat(const QString& format) {
    int index = formatCombo->findText(format, Qt::MatchFixedString);
    if (index >= 0) {
        formatCombo->setCurrentIndex(index);
    }
}

void OutputSettingsPanel::setSampleRate(int rate) {
    QString rateStr = QString::number(rate);
    int index = sampleRateComboBox->findText(rateStr, Qt::MatchFixedString);
    if (index >= 0) {
        sampleRateComboBox->setCurrentIndex(index);
    }
}

void OutputSettingsPanel::setBitDepth(int depth) {
    QString depthStr = QString::number(depth);
    int index = bitDepthComboBox->findText(depthStr, Qt::MatchFixedString);
    if (index >= 0) {
        bitDepthComboBox->setCurrentIndex(index);
    }
}

void OutputSettingsPanel::setBitrate(int rate) {
    QString rateStr = QString::number(rate);
    int index = bitrateComboBox->findText(rateStr, Qt::MatchFixedString);
    if (index >= 0) {
        bitrateComboBox->setCurrentIndex(index);
    }
}

void OutputSettingsPanel::setOutputFolder(const QString& folder) {
    outputFolderEdit->setText(folder);
    emit outputFolderChanged(folder);
}

void OutputSettingsPanel::onBrowseOutputFolder() {
    QString defaultPath = Preferences::instance().lastOutputDirectory();
    QString folder = QFileDialog::getExistingDirectory(this, "Select Output Folder", defaultPath);
    if (!folder.isEmpty()) {
        outputFolderEdit->setText(folder);
        Preferences::instance().setLastOutputDirectory(folder);
        emit outputFolderChanged(folder);
    }
}
