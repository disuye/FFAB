#include "AuxOutputFilter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QJsonObject>
#include <QGroupBox>
#include <QStackedWidget>
#include <QStandardPaths>

AuxOutputFilter::AuxOutputFilter() {
    position = Position::MIDDLE;  // Insertable, unlike OUTPUT
    updateFFmpegFlags();
}

void AuxOutputFilter::setFilterId(int id) {
    BaseFilter::setFilterId(id);
    
    // Generate suffix based on hex ID when ID is assigned
    m_filenameSuffix = QString("_%1").arg(QString::number(id, 16).toUpper().rightJustified(4, '0'));
}

QWidget* AuxOutputFilter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("auxOutputFilterTarget");
    auto* mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // Header info
    auto* infoLabel = new QLabel("Aux Output Filter");
    infoLabel->setWordWrap(true);
    infoLabel->setToolTip("Main Chain: Automatically branches.\nAsplit Sub-Chains: Terminates the chain.");
    mainLayout->addWidget(infoLabel);
    
    // ========== CATEGORY SELECTION ==========
    auto* categoryLabel = new QLabel("Audio File:");
    mainLayout->addWidget(categoryLabel);
    
    categoryCombo = new QComboBox();
    categoryCombo->addItem("Uncompressed");
    categoryCombo->addItem("Lossless");
    categoryCombo->addItem("Compressed");
    categoryCombo->addItem("Advanced");
    categoryCombo->setCurrentIndex(static_cast<int>(m_category));
    connect(categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuxOutputFilter::onCategoryChanged);
    mainLayout->addWidget(categoryCombo);

    // ========== SETTINGS PAGES WITH QSTACKEDWIDGET ==========
    settingsStack = new QStackedWidget();
    
    // Create all settings pages upfront
    uncompressedPage = new QWidget();
    uncompressedPage->setMaximumWidth(440);
    createUncompressedSettings(uncompressedPage);
    settingsStack->addWidget(uncompressedPage);  // Index 0

    losslessPage = new QWidget();
    losslessPage->setMaximumWidth(440);
    createLosslessSettings(losslessPage);
    settingsStack->addWidget(losslessPage);  // Index 1

    compressedPage = new QWidget();
    compressedPage->setMaximumWidth(440);
    createCompressedSettings(compressedPage);
    settingsStack->addWidget(compressedPage);  // Index 2

    // ========== ADVANCED SETTINGS PAGE ==========
    auto* advancedPage = new QWidget();
    auto* advancedLayout = new QVBoxLayout(advancedPage);
    advancedLayout->setContentsMargins(0, 0, 0, 0);
    advancedLayout->setSpacing(12);

    // Title
    auto* advancedTitle = new QLabel("Use Custom Output Flags");
    QFont titleFont = advancedTitle->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    advancedTitle->setFont(titleFont);
    advancedLayout->addWidget(advancedTitle);

    // Custom flags text box
    customFlagsEdit = new QLineEdit();
    customFlagsEdit->setPlaceholderText("-c:a libopus -b:a 320k -vbr on -f opus");
    customFlagsEdit->setText(m_customFlags);
    connect(customFlagsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_customFlags = text;
        updateFFmpegFlags();
    });
    advancedLayout->addWidget(customFlagsEdit);

    // File extension row
    auto* extLayout = new QHBoxLayout();
    extLayout->addWidget(new QLabel("File Extension:"));

    auto* customExtEdit = new QLineEdit();
    customExtEdit->setPlaceholderText("opus");
    customExtEdit->setText(m_customExtension);
    customExtEdit->setMaximumWidth(80);
    customExtEdit->setToolTip("Enter file extension without dot (e.g., opus, ogg, webm)");

                
    connect(customExtEdit, &QLineEdit::textChanged, [this](const QString& text) {
        QString cleaned = text.toLower().trimmed();
        cleaned.remove('.');  // Remove dots if user types them
        m_customExtension = cleaned.isEmpty() ? "opus" : cleaned;
    });
    extLayout->addWidget(customExtEdit);
    extLayout->addStretch();

    advancedLayout->addLayout(extLayout);
    advancedLayout->addStretch();

    settingsStack->addWidget(advancedPage);  // Index 3

    settingsStack->setCurrentIndex(static_cast<int>(m_category));
    mainLayout->addWidget(settingsStack);
    mainLayout->addStretch();


    // ========== OUTPUT DESTINATION ==========
    auto* destGroup = new QGroupBox("Output Destination");
    auto* destLayout = new QVBoxLayout(destGroup);

    // Filename suffix
    auto* suffixLayout = new QHBoxLayout();
    suffixLayout->addWidget(new QLabel("Append Text to Filename:"));
    
    filenameSuffixEdit = new QLineEdit();
    filenameSuffixEdit->setText(m_filenameSuffix);
    filenameSuffixEdit->setPlaceholderText("_aux");
    connect(filenameSuffixEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_filenameSuffix = text;
    });
    suffixLayout->addWidget(filenameSuffixEdit, 1);
    
    destLayout->addLayout(suffixLayout);
    
    // Output folder
    auto* folderLayout = new QHBoxLayout();
    folderLayout->addWidget(new QLabel("Output Folder:"));
    
    outputFolderEdit = new QLineEdit();
    outputFolderEdit->setMinimumWidth(240);
    outputFolderEdit->setMinimumHeight(22);
    outputFolderEdit->setStyleSheet("");
    outputFolderEdit->setPlaceholderText(m_useCustomOutputFolder ? "Select output folder..." : "Using Main Output Folder");
    outputFolderEdit->setText(m_outputFolder);
    outputFolderEdit->setReadOnly(true);
    outputFolderEdit->setEnabled(m_useCustomOutputFolder);
    folderLayout->addWidget(outputFolderEdit, 1);
    
    browseOutputFolderButton = new QPushButton("Browse...");
    browseOutputFolderButton->setMinimumWidth(80);
    browseOutputFolderButton->setMinimumHeight(22);
    browseOutputFolderButton->setAutoDefault(false);  // Prevents default button styling
    browseOutputFolderButton->setStyleSheet("");   
    browseOutputFolderButton->setEnabled(m_useCustomOutputFolder);
    connect(browseOutputFolderButton, &QPushButton::clicked, this, &AuxOutputFilter::onBrowseOutputFolder);
    folderLayout->addWidget(browseOutputFolderButton);

    // Checkbox to enable custom output folder
    useCustomOutputFolderCheck = new QCheckBox("Choose alternative to Main Output Folder");
    useCustomOutputFolderCheck->setChecked(m_useCustomOutputFolder);
    connect(useCustomOutputFolderCheck, &QCheckBox::toggled, [this](bool checked) {
        m_useCustomOutputFolder = checked;
        outputFolderEdit->setEnabled(checked);
        browseOutputFolderButton->setEnabled(checked);
        if (!checked) {
            // Clear custom folder when unchecked (will use main OUTPUT folder)
            m_outputFolder.clear();
            outputFolderEdit->clear();
            outputFolderEdit->setPlaceholderText("Using Main Output Folder");
        } else {
            outputFolderEdit->setPlaceholderText("Select output folder...");
        }
    });

    destLayout->addLayout(folderLayout);
    
    destLayout->addWidget(useCustomOutputFolderCheck);
    
    mainLayout->addWidget(destGroup);
    
    mainLayout->addStretch();

    return parametersWidget;
}

void AuxOutputFilter::onCategoryChanged(int index) {
    m_category = static_cast<Category>(index);
    if (settingsStack) {
        settingsStack->setCurrentIndex(index);
    }
    updateFFmpegFlags();
}

void AuxOutputFilter::onSettingsChanged() {
    updateFFmpegFlags();
}

void AuxOutputFilter::onBrowseOutputFolder() {
    QString defaultPath = m_outputFolder.isEmpty() 
        ? QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
        : m_outputFolder;
    
    QString folder = QFileDialog::getExistingDirectory(
        parametersWidget,
        "Select output folder...",
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folder.isEmpty()) {
        m_outputFolder = folder;
        if (outputFolderEdit) {
            outputFolderEdit->setText(folder);
        }
    }
}

void AuxOutputFilter::createUncompressedSettings(QWidget* container) {
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop); 

    // File Type
    auto* formatLabel = new QLabel("File Type:");
    formatLabel->setMaximumWidth(240);
    auto* formatCombo = new QComboBox();
    formatCombo->addItem("WAV");
    formatCombo->addItem("AIF/AIFF");
    formatCombo->addItem("CAF");
    formatCombo->setCurrentIndex(static_cast<int>(m_uncompressedFormat));
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_uncompressedFormat = static_cast<UncompressedFormat>(idx);
        onSettingsChanged();
    });
    formLayout->addRow(formatLabel, formatCombo);

    // Bit Depth
    auto* bitDepthLabel = new QLabel("Bit Depth:");
    bitDepthLabel->setMaximumWidth(240);
    auto* bitDepthCombo = new QComboBox();
    bitDepthCombo->addItem("32-bit (float)");
    bitDepthCombo->addItem("24-bit");
    bitDepthCombo->addItem("16-bit");
    bitDepthCombo->addItem("8-bit");
    bitDepthCombo->setCurrentIndex(static_cast<int>(m_bitDepth));
    connect(bitDepthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_bitDepth = static_cast<BitDepth>(idx);
        onSettingsChanged();
    });
    formLayout->addRow(bitDepthLabel, bitDepthCombo);

    // Sample Rate
    auto* sampleRateLabel = new QLabel("Sample Rate (Hz):");
    sampleRateLabel->setMaximumWidth(240);
    auto* sampleRateCombo = new QComboBox();
    sampleRateCombo->addItems({
        "192000", "176400", "96000", "88200", "64000", 
        "48000", "44100", "32000", "24000", "22050", 
        "12000", "11025"
    });
    sampleRateCombo->setCurrentText(QString::number(m_sampleRate));
    connect(sampleRateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, sampleRateCombo]() {
        m_sampleRate = sampleRateCombo->currentText().toInt();
        onSettingsChanged();
    });
    formLayout->addRow(sampleRateLabel, sampleRateCombo);

    // Format (Interleaved/Split)
    auto* audioFormatLabel = new QLabel("Format:");
    audioFormatLabel->setMaximumWidth(240);
    auto* audioFormatCombo = new QComboBox();
    audioFormatCombo->addItem("Interleaved");
    audioFormatCombo->addItem("Split");
    audioFormatCombo->setCurrentIndex(static_cast<int>(m_audioFormat));
    connect(audioFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_audioFormat = static_cast<AudioFormat>(idx);
        onSettingsChanged();
    });
//    formLayout->addRow(audioFormatLabel, audioFormatCombo);

    layout->addLayout(formLayout);
}

void AuxOutputFilter::createLosslessSettings(QWidget* container) {
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Encoding (only FLAC for now)
    auto* encodingLabel = new QLabel("Encoding:");
    encodingLabel->setMaximumWidth(240);
    auto* encodingCombo = new QComboBox();
    encodingCombo->addItem("FLAC");
    encodingCombo->setCurrentIndex(0);
    encodingCombo->setEnabled(false);
    formLayout->addRow(encodingLabel, encodingCombo);

    // Encoding Level
    auto* levelLabel = new QLabel("Encoding Level:");
    levelLabel->setMaximumWidth(240);
    auto* levelCombo = new QComboBox();
    levelCombo->addItems({
        "0 (fastest)", "1", "2", "3", "4", "5", "6", 
        "7", "8", "9", "10", "11", "12 (smallest)"
    });
    levelCombo->setCurrentIndex(m_flacLevel);
    connect(levelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_flacLevel = idx;
        onSettingsChanged();
    });
    formLayout->addRow(levelLabel, levelCombo);

    layout->addLayout(formLayout);
}

void AuxOutputFilter::createCompressedSettings(QWidget* container) {
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Compression Type
    auto* compressionLabel = new QLabel("Compression Type:");
    compressionLabel->setMaximumWidth(240);
    auto* compressionCombo = new QComboBox();
    compressionCombo->addItem("MP3");
    compressionCombo->addItem("AAC");
    compressionCombo->setCurrentIndex(static_cast<int>(m_compressedFormat));
    connect(compressionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_compressedFormat = static_cast<CompressedFormat>(idx);
        onSettingsChanged();
    });
    formLayout->addRow(compressionLabel, compressionCombo);

    // Constant Bitrate checkbox - empty label
    auto* emptyLabel = new QLabel("");
    emptyLabel->setMaximumWidth(240);
    auto* cbrCheck = new QCheckBox("Constant Bitrate");
    cbrCheck->setChecked(m_constantBitrate);
    connect(cbrCheck, &QCheckBox::toggled, [this](bool checked) {
        m_constantBitrate = checked;
        onSettingsChanged();
    });
    formLayout->addRow(emptyLabel, cbrCheck);

    // Bitrate
    auto* bitrateLabel = new QLabel("Bitrate (kbps):");
    bitrateLabel->setMaximumWidth(240);
    auto* bitrateCombo = new QComboBox();
    bitrateCombo->addItems({
        "320", "256", "224", "160", "128", "112", "96", "80", "64"
    });
    bitrateCombo->setCurrentText(QString::number(m_bitrate));
    connect(bitrateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, bitrateCombo]() {
        m_bitrate = bitrateCombo->currentText().toInt();
        onSettingsChanged();
    });
    formLayout->addRow(bitrateLabel, bitrateCombo);

    layout->addLayout(formLayout);
}

QString AuxOutputFilter::getFileExtension() const {
    if (m_category == Category::Advanced) {
        return m_customExtension.isEmpty() ? "wav" : m_customExtension;
    }
    
    switch (m_category) {
        case Category::Uncompressed:
            switch (m_uncompressedFormat) {
                case UncompressedFormat::WAV: return "wav";
                case UncompressedFormat::AIFF: return "aiff";
                case UncompressedFormat::CAF: return "caf";
            }
            break;
        case Category::Lossless:
            return "flac";
        case Category::Compressed:
            return (m_compressedFormat == CompressedFormat::MP3) ? "mp3" : "aac";
        case Category::Advanced:
            break;  // Already handled above
    }
    return "wav";
}

QString AuxOutputFilter::buildUncompressedFlags() const {
    QString flags;
    
    // Determine endianness based on format
    bool isBigEndian = (m_uncompressedFormat == UncompressedFormat::AIFF || 
                        m_uncompressedFormat == UncompressedFormat::CAF);
    QString endian = isBigEndian ? "be" : "le";
    
    // Codec based on bit depth
    QString codec;
    switch (m_bitDepth) {
        case BitDepth::Float32:
            codec = QString("pcm_f32%1").arg(endian);
            break;
        case BitDepth::Int24:
            codec = QString("pcm_s24%1").arg(endian);
            break;
        case BitDepth::Int16:
            codec = QString("pcm_s16%1").arg(endian);
            break;
        case BitDepth::Int8:
            codec = "pcm_s8";  // 8-bit has no endianness
            break;
    }
    
    flags = QString("-c:a %1 -ar %2").arg(codec).arg(m_sampleRate);
    
    // Format-specific flags
    if (m_uncompressedFormat == UncompressedFormat::AIFF) {
        flags += " -f aiff";
    } else if (m_uncompressedFormat == UncompressedFormat::CAF) {
        flags += " -f caf";
    }
    
    return flags;
}

QString AuxOutputFilter::buildLosslessFlags() const {
    return QString("-c:a flac -compression_level %1").arg(m_flacLevel);
}

QString AuxOutputFilter::buildCompressedFlags() const {
    QString flags;
    
    if (m_compressedFormat == CompressedFormat::MP3) {
        flags = QString("-c:a libmp3lame -b:a %1k").arg(m_bitrate);
        if (m_constantBitrate) {
            flags += " -abr 0";
        }
    } else { // AAC
        flags = QString("-c:a aac -b:a %1k").arg(m_bitrate);
        if (m_constantBitrate) {
            flags += " -vbr 0";
        }
    }
    
    return flags;
}

void AuxOutputFilter::updateFFmpegFlags() {
    switch (m_category) {
        case Category::Uncompressed:
            ffmpegFlags = buildUncompressedFlags();
            break;
        case Category::Lossless:
            ffmpegFlags = buildLosslessFlags();
            break;
        case Category::Compressed:
            ffmpegFlags = buildCompressedFlags();
            break;
        case Category::Advanced:
            ffmpegFlags = m_customFlags;
            break;
    }
}

QString AuxOutputFilter::buildFFmpegFlags() const {
    // Aux Output is not a filter - it's an output target
    // The codec flags are retrieved separately by FilterChain for output mapping
    return "";
}

QString AuxOutputFilter::getDefaultCustomCommandTemplate() const {
    return "-map [NNNN] -ac 2 -ar 48000 -c:a pcm_s24le";
}

void AuxOutputFilter::toJSON(QJsonObject& json) const {
    json["type"] = "aux-output";
    json["category"] = static_cast<int>(m_category);
    
    // Uncompressed
    json["uncompressedFormat"] = static_cast<int>(m_uncompressedFormat);
    json["bitDepth"] = static_cast<int>(m_bitDepth);
    json["sampleRate"] = m_sampleRate;
    json["audioFormat"] = static_cast<int>(m_audioFormat);
    
    // Lossless
    json["losslessFormat"] = static_cast<int>(m_losslessFormat);
    json["flacLevel"] = m_flacLevel;
    
    // Compressed
    json["compressedFormat"] = static_cast<int>(m_compressedFormat);
    json["constantBitrate"] = m_constantBitrate;
    json["bitrate"] = m_bitrate;
    
    // Custom flags (for Advanced category)
    json["customFlags"] = m_customFlags;
    json["customExtension"] = m_customExtension;
    
    // Output settings
    json["outputFolder"] = m_outputFolder;
    json["filenameSuffix"] = m_filenameSuffix;
    json["useCustomOutputFolder"] = m_useCustomOutputFolder;
}

void AuxOutputFilter::fromJSON(const QJsonObject& json) {
    m_category = static_cast<Category>(json["category"].toInt(0));
    
    // Uncompressed
    m_uncompressedFormat = static_cast<UncompressedFormat>(json["uncompressedFormat"].toInt(0));
    m_bitDepth = static_cast<BitDepth>(json["bitDepth"].toInt(1));
    m_sampleRate = json["sampleRate"].toInt(48000);
    m_audioFormat = static_cast<AudioFormat>(json["audioFormat"].toInt(0));
    
    // Lossless
    m_losslessFormat = static_cast<LosslessFormat>(json["losslessFormat"].toInt(0));
    m_flacLevel = json["flacLevel"].toInt(5);
    
    // Compressed
    m_compressedFormat = static_cast<CompressedFormat>(json["compressedFormat"].toInt(0));
    m_constantBitrate = json["constantBitrate"].toBool(true);
    m_bitrate = json["bitrate"].toInt(320);
    
    // Custom flags (for Advanced category)
    m_customFlags = json["customFlags"].toString();
    m_customExtension = json["customExtension"].toString("wav");
    
    // Output settings
    m_outputFolder = json["outputFolder"].toString();
    m_useCustomOutputFolder = json["useCustomOutputFolder"].toBool(false);
    
    // Load suffix, defaulting to hex ID if not present
    QString defaultSuffix = QString("_%1").arg(QString::number(m_filterId, 16).toUpper().rightJustified(4, '0'));
    m_filenameSuffix = json["filenameSuffix"].toString(defaultSuffix);
    
    updateFFmpegFlags();
    
    // Update UI if it exists
    if (categoryCombo) {
        categoryCombo->setCurrentIndex(static_cast<int>(m_category));
    }
    if (settingsStack) {
        settingsStack->setCurrentIndex(static_cast<int>(m_category));
    }
    if (useCustomOutputFolderCheck) {
        useCustomOutputFolderCheck->setChecked(m_useCustomOutputFolder);
    }
    if (outputFolderEdit) {
        outputFolderEdit->setText(m_outputFolder);
        outputFolderEdit->setEnabled(m_useCustomOutputFolder);
        outputFolderEdit->setPlaceholderText(m_useCustomOutputFolder ? "Select output folder..." : "(Using main OUTPUT folder)");
    }
    if (browseOutputFolderButton) {
        browseOutputFolderButton->setEnabled(m_useCustomOutputFolder);
    }
    if (filenameSuffixEdit) {
        filenameSuffixEdit->setText(m_filenameSuffix);
    }
    if (customFlagsEdit) {
        customFlagsEdit->setText(m_customFlags);
    }
}
