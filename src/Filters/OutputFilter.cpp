#include "OutputFilter.h"
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
#include <QDebug>

OutputFilter::OutputFilter() {
    position = Position::OUTPUT;
    updateFFmpegFlags();
}

QWidget* OutputFilter::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("outputFilterTarget");
    auto* mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // Header info
    auto* infoLabel = new QLabel("Main Output");
    infoLabel->setWordWrap(true);
    infoLabel->setToolTip("The buck stops here!");
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
            this, &OutputFilter::onCategoryChanged);
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

    customExtensionEdit = new QLineEdit();
    customExtensionEdit->setPlaceholderText("opus");
    customExtensionEdit->setText(m_customExtension);
    customExtensionEdit->setMaximumWidth(80);
    customExtensionEdit->setToolTip("Enter file extension without dot (e.g., opus, ogg, webm)");
    connect(customExtensionEdit, &QLineEdit::textChanged, [this](const QString& text) {
        QString cleaned = text.toLower().trimmed();
        cleaned.remove('.');  // Remove dots if user types them
        m_customExtension = cleaned.isEmpty() ? "opus" : cleaned;
    });
    extLayout->addWidget(customExtensionEdit);
    extLayout->addStretch();

    advancedLayout->addLayout(extLayout);
    advancedLayout->addStretch();

    settingsStack->addWidget(advancedPage);  // Index 3
    
    settingsStack->setCurrentIndex(static_cast<int>(m_category));
    mainLayout->addWidget(settingsStack);
    
    // ========== VIDEO PASSTHROUGH ==========
    videoPassthroughCheck = new QCheckBox("Video Passthrough");
    videoPassthroughCheck->setChecked(m_videoPassthrough);
    videoPassthroughCheck->setToolTip(
        "Input video files — video passes through to output unchanged,\n"
        "audio track is processed by the filter chain.\n\n"
        "When enabled, output format is forced to a video-capable\n"
        "container (MOV/MP4/MKV). No video re-encoding occurs.");
        connect(videoPassthroughCheck, &QCheckBox::toggled, [this](bool checked) {
            setVideoPassthrough(checked);
        });

    mainLayout->addWidget(videoPassthroughCheck);
    mainLayout->addStretch();

    // ========== OUTPUT DESTINATION ==========
    auto* destGroup = new QGroupBox("Output Destination");
    auto* destLayout = new QVBoxLayout(destGroup);

    // Filename suffix
    auto* suffixLayout = new QHBoxLayout();
    suffixLayout->addWidget(new QLabel("Append Text to Filename:"));
    
    filenameSuffixEdit = new QLineEdit();
    filenameSuffixEdit->setText(m_filenameSuffix);
    filenameSuffixEdit->setPlaceholderText("_0000");
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
    outputFolderEdit->setPlaceholderText("Select output folder...");
    outputFolderEdit->setText(m_outputFolder);
    outputFolderEdit->setReadOnly(true);
    folderLayout->addWidget(outputFolderEdit, 1);
    
    auto* browseBtn = new QPushButton("Browse...");
    browseBtn->setMinimumWidth(80);
    browseBtn->setMinimumHeight(22);
    browseBtn->setAutoDefault(false);
    browseBtn->setStyleSheet("");
    connect(browseBtn, &QPushButton::clicked, this, &OutputFilter::onBrowseOutputFolder);
    folderLayout->addWidget(browseBtn);
    
    destLayout->addLayout(folderLayout);
    
    mainLayout->addWidget(destGroup);
    mainLayout->addStretch();

    return parametersWidget;
}

void OutputFilter::onCategoryChanged(int index) {
    m_category = static_cast<Category>(index);
    if (settingsStack) {
        settingsStack->setCurrentIndex(index);
    }
    updateFFmpegFlags();
}

void OutputFilter::onSettingsChanged() {
    updateFFmpegFlags();
}

void OutputFilter::onBrowseOutputFolder() {
    QString defaultPath = m_outputFolder.isEmpty() 
        ? QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
        : m_outputFolder;
    
    QString folder = QFileDialog::getExistingDirectory(
        parametersWidget,
        "Select Output Folder",
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!folder.isEmpty()) {
        m_outputFolder = folder;
        if (outputFolderEdit) {
            outputFolderEdit->setText(folder);
        }
        qDebug() << "OutputFilter: Folder selected:" << folder;
        qDebug() << "OutputFilter: Updated line edit";
        qDebug() << "OutputFilter: Emitting outputFolderChanged signal";
        emit outputFolderChanged(folder);
    }
}

void OutputFilter::createUncompressedSettings(QWidget* container) {
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop); 

    // File Type
    auto* formatLabel = new QLabel("File Type:");
    formatLabel->setMaximumWidth(240);
    uncompressedFormatCombo = new QComboBox();
    uncompressedFormatCombo->addItem("WAV");
    uncompressedFormatCombo->addItem("AIF/AIFF");
    uncompressedFormatCombo->addItem("CAF");
    uncompressedFormatCombo->setCurrentIndex(static_cast<int>(m_uncompressedFormat));
    connect(uncompressedFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_uncompressedFormat = static_cast<UncompressedFormat>(idx);
        onSettingsChanged();
    });
    formLayout->addRow(formatLabel, uncompressedFormatCombo);

    // Bit Depth
    auto* bitDepthLabel = new QLabel("Bit Depth:");
    bitDepthLabel->setMaximumWidth(240);
    bitDepthCombo = new QComboBox();
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
    sampleRateCombo = new QComboBox();
    sampleRateCombo->addItems({
        "192000", "176400", "96000", "88200", "64000", 
        "48000", "44100", "32000", "24000", "22050", 
        "12000", "11025"
    });
    sampleRateCombo->setCurrentText(QString::number(m_sampleRate));
    connect(sampleRateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        m_sampleRate = sampleRateCombo->currentText().toInt();
        onSettingsChanged();
    });
    formLayout->addRow(sampleRateLabel, sampleRateCombo);

    // Format (Interleaved/Split)
    auto* audioFormatLabel = new QLabel("Format:");
    audioFormatLabel->setMaximumWidth(240);
    audioFormatCombo = new QComboBox();
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

void OutputFilter::createLosslessSettings(QWidget* container) {
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Encoding (only FLAC for now)
    auto* encodingLabel = new QLabel("Encoding:");
    encodingLabel->setMaximumWidth(240);
    losslessEncodingCombo = new QComboBox();
    losslessEncodingCombo->addItem("FLAC");
    losslessEncodingCombo->setCurrentIndex(0);
    losslessEncodingCombo->setEnabled(false);
    formLayout->addRow(encodingLabel, losslessEncodingCombo);

    // Encoding Level
    auto* levelLabel = new QLabel("Encoding Level:");
    levelLabel->setMaximumWidth(240);
    flacLevelCombo = new QComboBox();
    flacLevelCombo->addItems({
        "0 (fastest)", "1", "2", "3", "4", "5", "6", 
        "7", "8", "9", "10", "11", "12 (smallest)"
    });
    flacLevelCombo->setCurrentIndex(m_flacLevel);
    connect(flacLevelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_flacLevel = idx;
        onSettingsChanged();
    });
    formLayout->addRow(levelLabel, flacLevelCombo);

    layout->addLayout(formLayout);
}

void OutputFilter::createCompressedSettings(QWidget* container) {
    auto* layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Compression Type
    auto* compressionLabel = new QLabel("Compression Type:");
    compressionLabel->setMaximumWidth(240);    
    compressedFormatCombo = new QComboBox();
    compressedFormatCombo->addItem("MP3");
    compressedFormatCombo->addItem("AAC");
    compressedFormatCombo->setCurrentIndex(static_cast<int>(m_compressedFormat));
    connect(compressedFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_compressedFormat = static_cast<CompressedFormat>(idx);
        onSettingsChanged();
    });
    formLayout->addRow(compressionLabel, compressedFormatCombo);

    // Constant Bitrate checkbox
    auto* emptyLabel = new QLabel("");
    emptyLabel->setMaximumWidth(240);
    constantBitrateCheck = new QCheckBox("Constant Bitrate");
    constantBitrateCheck->setChecked(m_constantBitrate);
    connect(constantBitrateCheck, &QCheckBox::toggled, [this](bool checked) {
        m_constantBitrate = checked;
        onSettingsChanged();
    });
    formLayout->addRow(emptyLabel, constantBitrateCheck);

    // Bitrate
    auto* bitrateLabel = new QLabel("Bitrate (kbps):");
    bitrateLabel->setMaximumWidth(240);    
    bitrateCombo = new QComboBox();
    bitrateCombo->addItems({
        "320", "256", "224", "160", "128", "112", "96", "80", "64"
    });
    bitrateCombo->setCurrentText(QString::number(m_bitrate));
    connect(bitrateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]() {
        m_bitrate = bitrateCombo->currentText().toInt();
        onSettingsChanged();
    });
    formLayout->addRow(bitrateLabel, bitrateCombo);

    layout->addLayout(formLayout);
}

QString OutputFilter::getFileExtension() const {
    // Video passthrough forces a video-capable container
    if (m_videoPassthrough) {
        if (m_category == Category::Advanced) {
            // In Advanced mode, trust the user's custom extension if it's video-capable
            QStringList videoExts = {"mov", "mp4", "mkv", "avi", "webm", "ts", "mts"};
            if (videoExts.contains(m_customExtension.toLower())) {
                return m_customExtension;
            }
            return "mov";  // Default video container
        }
        // For all standard categories, force MOV (best general-purpose video container)
        // MP4 would also work; MOV is slightly more flexible with codec support
        return "mov";
    }
    
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

void OutputFilter::setVideoPassthrough(bool enabled) {
    if (m_videoPassthrough == enabled) return;
    m_videoPassthrough = enabled;
    
    // Update checkbox if it exists and differs
    if (videoPassthroughCheck && videoPassthroughCheck->isChecked() != enabled) {
        videoPassthroughCheck->blockSignals(true);
        videoPassthroughCheck->setChecked(enabled);
        videoPassthroughCheck->blockSignals(false);
    }
    
    emit videoPassthroughChanged(enabled);
}

QString OutputFilter::buildOutputMappingFlags() const {
    if (!m_videoPassthrough) {
        return QString();  // No extra mapping — FilterChain handles -map [out] normally
    }
    
    // Video passthrough: copy video and subtitle streams from input 0
    // -map 0:v copies all video streams
    // -c:v copy means no re-encoding (fast, lossless)
    // -map 0:s? copies subtitle streams if present (? = non-fatal if none exist)
    // -c:s copy preserves subtitle encoding
    return "-map 0:v -c:v copy -map 0:s? -c:s copy";
}

QString OutputFilter::buildUncompressedFlags() const {
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

QString OutputFilter::buildLosslessFlags() const {
    return QString("-c:a flac -compression_level %1").arg(m_flacLevel);
}

QString OutputFilter::buildCompressedFlags() const {
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

void OutputFilter::updateFFmpegFlags() {
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

QString OutputFilter::buildFFmpegFlags() const {
    return ffmpegFlags;
}

QString OutputFilter::getDefaultCustomCommandTemplate() const {
    return "-map [out] -ac 2 -ar 48000 -c:a pcm_s24le";
}

void OutputFilter::toJSON(QJsonObject& json) const {
    json["type"] = "output";
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
    
    // Video passthrough
    json["videoPassthrough"] = m_videoPassthrough;
}

void OutputFilter::fromJSON(const QJsonObject& json) {
    m_category = static_cast<Category>(json["category"].toInt(0));
    
    // Uncompressed
    m_uncompressedFormat = static_cast<UncompressedFormat>(json["uncompressedFormat"].toInt(0));
    m_bitDepth = static_cast<BitDepth>(json["bitDepth"].toInt(1)); // Default to 24-bit
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
    m_filenameSuffix = json["filenameSuffix"].toString("_0000");
    
    // Video passthrough
    m_videoPassthrough = json["videoPassthrough"].toBool(false);
    
    updateFFmpegFlags();
    
    // Update UI if widgets exist
    if (categoryCombo) {
        categoryCombo->blockSignals(true);
        categoryCombo->setCurrentIndex(static_cast<int>(m_category));
        categoryCombo->blockSignals(false);
    }
    if (settingsStack) {
        settingsStack->setCurrentIndex(static_cast<int>(m_category));
    }
    
    // Uncompressed settings widgets
    if (uncompressedFormatCombo) {
        uncompressedFormatCombo->blockSignals(true);
        uncompressedFormatCombo->setCurrentIndex(static_cast<int>(m_uncompressedFormat));
        uncompressedFormatCombo->blockSignals(false);
    }
    if (bitDepthCombo) {
        bitDepthCombo->blockSignals(true);
        bitDepthCombo->setCurrentIndex(static_cast<int>(m_bitDepth));
        bitDepthCombo->blockSignals(false);
    }
    if (sampleRateCombo) {
        sampleRateCombo->blockSignals(true);
        sampleRateCombo->setCurrentText(QString::number(m_sampleRate));
        sampleRateCombo->blockSignals(false);
    }
    if (audioFormatCombo) {
        audioFormatCombo->blockSignals(true);
        audioFormatCombo->setCurrentIndex(static_cast<int>(m_audioFormat));
        audioFormatCombo->blockSignals(false);
    }
    
    // Lossless settings widgets
    if (flacLevelCombo) {
        flacLevelCombo->blockSignals(true);
        flacLevelCombo->setCurrentIndex(m_flacLevel);
        flacLevelCombo->blockSignals(false);
    }
    
    // Compressed settings widgets
    if (compressedFormatCombo) {
        compressedFormatCombo->blockSignals(true);
        compressedFormatCombo->setCurrentIndex(static_cast<int>(m_compressedFormat));
        compressedFormatCombo->blockSignals(false);
    }
    if (constantBitrateCheck) {
        constantBitrateCheck->blockSignals(true);
        constantBitrateCheck->setChecked(m_constantBitrate);
        constantBitrateCheck->blockSignals(false);
    }
    if (bitrateCombo) {
        bitrateCombo->blockSignals(true);
        bitrateCombo->setCurrentText(QString::number(m_bitrate));
        bitrateCombo->blockSignals(false);
    }
    
    // Advanced settings widgets
    if (customFlagsEdit) {
        customFlagsEdit->blockSignals(true);
        customFlagsEdit->setText(m_customFlags);
        customFlagsEdit->blockSignals(false);
    }
    if (customExtensionEdit) {
        customExtensionEdit->blockSignals(true);
        customExtensionEdit->setText(m_customExtension);
        customExtensionEdit->blockSignals(false);
    }
    
    // Output destination widgets
    if (outputFolderEdit) {
        outputFolderEdit->setText(m_outputFolder);
    }
    if (filenameSuffixEdit) {
        filenameSuffixEdit->blockSignals(true);
        filenameSuffixEdit->setText(m_filenameSuffix);
        filenameSuffixEdit->blockSignals(false);
    }
    if (videoPassthroughCheck) {
        videoPassthroughCheck->blockSignals(true);
        videoPassthroughCheck->setChecked(m_videoPassthrough);
        videoPassthroughCheck->blockSignals(false);
    }
}
