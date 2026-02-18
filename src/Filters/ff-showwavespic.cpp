#include "ff-showwavespic.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonObject>

FFShowwavespic::FFShowwavespic() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
    updateVideoCodecFlags();
}

void FFShowwavespic::setFilterId(int id) {
    BaseFilter::setFilterId(id);
    // Update flags when ID is assigned (suffix uses hex ID)
    updateFFmpegFlags();
}

QString FFShowwavespic::getFilenameSuffix() const {
    // Format: _XXXX-waveform where XXXX is hex filter ID
    QString hexId = QString("%1").arg(m_filterId, 4, 16, QChar('0')).toUpper();
    return QString("_%1-waveform").arg(hexId);
}

QString FFShowwavespic::scaleModeToString(ScaleMode mode) const {
    switch (mode) {
        case ScaleMode::LINEAR: return "lin";
        case ScaleMode::SQRT: return "sqrt";
        case ScaleMode::LOG: return "log";
    }
    return "sqrt";
}

QString FFShowwavespic::drawModeToString(DrawMode mode) const {
    switch (mode) {
        case DrawMode::FULL: return "full";
        case DrawMode::SCALE: return "scale";
    }
    return "full";
}

QString FFShowwavespic::filterModeToString(FilterMode mode) const {
    switch (mode) {
        case FilterMode::AVERAGE: return "average";
        case FilterMode::PEAK: return "peak";
    }
    return "average";
}

QWidget* FFShowwavespic::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto* mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // Info label
    auto* infoLabel = new QLabel("Creates a PNG waveform image alongside the audio output.");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // ========== IMAGE SETTINGS ==========
    auto* settingsGroup = new QGroupBox("Image Settings");
    auto* settingsLayout = new QVBoxLayout(settingsGroup);
    
    // Size
    auto* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Width:"));
    widthSpinBox = new QSpinBox();
    widthSpinBox->setMinimum(100);
    widthSpinBox->setMaximum(4096);
    widthSpinBox->setValue(m_width);
    widthSpinBox->setSingleStep(100);
    widthSpinBox->setSuffix(" px");
    connect(widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_width = val;
        updateFFmpegFlags();
    });
    sizeLayout->addWidget(widthSpinBox);
    
    sizeLayout->addSpacing(16);
    sizeLayout->addWidget(new QLabel("Height:"));
    heightSpinBox = new QSpinBox();
    heightSpinBox->setMinimum(50);
    heightSpinBox->setMaximum(2048);
    heightSpinBox->setValue(m_height);
    heightSpinBox->setSingleStep(50);
    heightSpinBox->setSuffix(" px");
    connect(heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_height = val;
        updateFFmpegFlags();
    });
    sizeLayout->addWidget(heightSpinBox);
    sizeLayout->addStretch();
    settingsLayout->addLayout(sizeLayout);

    // Colors
    auto* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Channel 1:"));
    color1Button = new QPushButton();
    color1Button->setMinimumWidth(60);
    color1Button->setMaximumWidth(60);
    color1Button->setStyleSheet(QString("background-color: %1;").arg(m_color1.name()));
    connect(color1Button, &QPushButton::clicked, [this]() {
        QColor newColor = QColorDialog::getColor(m_color1, nullptr, "Channel 1 Color");
        if (newColor.isValid()) {
            m_color1 = newColor;
            color1Button->setStyleSheet(QString("background-color: %1;").arg(m_color1.name()));
            updateFFmpegFlags();
        }
    });
    colorLayout->addWidget(color1Button);
    
    colorLayout->addSpacing(16);
    colorLayout->addWidget(new QLabel("Channel 2:"));
    color2Button = new QPushButton();
    color2Button->setMinimumWidth(60);
    color2Button->setMaximumWidth(60);
    color2Button->setStyleSheet(QString("background-color: %1;").arg(m_color2.name()));
    connect(color2Button, &QPushButton::clicked, [this]() {
        QColor newColor = QColorDialog::getColor(m_color2, nullptr, "Channel 2 Color");
        if (newColor.isValid()) {
            m_color2 = newColor;
            color2Button->setStyleSheet(QString("background-color: %1;").arg(m_color2.name()));
            updateFFmpegFlags();
        }
    });
    colorLayout->addWidget(color2Button);
    colorLayout->addStretch();
    settingsLayout->addLayout(colorLayout);

    // Scale Mode
    auto* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("Scale:"));
    scaleModeCombo = new QComboBox();
    scaleModeCombo->addItem("Linear", static_cast<int>(ScaleMode::LINEAR));
    scaleModeCombo->addItem("Square Root", static_cast<int>(ScaleMode::SQRT));
    scaleModeCombo->addItem("Logarithmic", static_cast<int>(ScaleMode::LOG));
    scaleModeCombo->setCurrentIndex(static_cast<int>(m_scaleMode));
    connect(scaleModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_scaleMode = static_cast<ScaleMode>(idx);
        updateFFmpegFlags();
    });
    scaleLayout->addWidget(scaleModeCombo);
    scaleLayout->addStretch();
    settingsLayout->addLayout(scaleLayout);

    // Draw Mode
    auto* drawLayout = new QHBoxLayout();
    drawLayout->addWidget(new QLabel("Draw Mode:"));
    drawModeCombo = new QComboBox();
    drawModeCombo->addItem("Full", static_cast<int>(DrawMode::FULL));
    drawModeCombo->addItem("Scale", static_cast<int>(DrawMode::SCALE));
    drawModeCombo->setCurrentIndex(static_cast<int>(m_drawMode));
    connect(drawModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_drawMode = static_cast<DrawMode>(idx);
        updateFFmpegFlags();
    });
    drawLayout->addWidget(drawModeCombo);
    drawLayout->addStretch();
    settingsLayout->addLayout(drawLayout);

    // Filter Mode
    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter:"));
    filterModeCombo = new QComboBox();
    filterModeCombo->addItem("Average", static_cast<int>(FilterMode::AVERAGE));
    filterModeCombo->addItem("Peak", static_cast<int>(FilterMode::PEAK));
    filterModeCombo->setCurrentIndex(static_cast<int>(m_filterMode));
    connect(filterModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_filterMode = static_cast<FilterMode>(idx);
        updateFFmpegFlags();
    });
    filterLayout->addWidget(filterModeCombo);
    filterLayout->addStretch();
    settingsLayout->addLayout(filterLayout);

    // Split Channels
    splitChannelsCheckBox = new QCheckBox("Split Channels");
    splitChannelsCheckBox->setChecked(m_splitChannels);
    connect(splitChannelsCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_splitChannels = checked;
        updateFFmpegFlags();
    });
    settingsLayout->addWidget(splitChannelsCheckBox);

    mainLayout->addWidget(settingsGroup);

    // ========== OUTPUT DESTINATION ==========
    destGroup = new QGroupBox("Output Destination");
    auto* destLayout = new QVBoxLayout(destGroup);

    // Output folder row
    auto* folderLayout = new QHBoxLayout();
    folderLayout->addWidget(new QLabel("Output Folder:"));
    
    outputFolderEdit = new QLineEdit();
    outputFolderEdit->setMinimumWidth(200);
    outputFolderEdit->setPlaceholderText(m_useCustomOutputFolder ? "Select output folder..." : "Using Main Output Folder");
    outputFolderEdit->setText(m_outputFolder);
    outputFolderEdit->setReadOnly(true);
    outputFolderEdit->setEnabled(m_useCustomOutputFolder);
    folderLayout->addWidget(outputFolderEdit, 1);
    
    browseOutputFolderButton = new QPushButton("Browse...");
    browseOutputFolderButton->setMinimumWidth(80);
    browseOutputFolderButton->setAutoDefault(false);
    browseOutputFolderButton->setEnabled(m_useCustomOutputFolder);
    connect(browseOutputFolderButton, &QPushButton::clicked, this, &FFShowwavespic::onBrowseOutputFolder);
    folderLayout->addWidget(browseOutputFolderButton);
    
    destLayout->addLayout(folderLayout);

    // Checkbox to enable custom output folder
    useCustomOutputFolderCheck = new QCheckBox("Choose alternative to Main Output Folder");
    useCustomOutputFolderCheck->setChecked(m_useCustomOutputFolder);
    connect(useCustomOutputFolderCheck, &QCheckBox::toggled, [this](bool checked) {
        m_useCustomOutputFolder = checked;
        outputFolderEdit->setEnabled(checked);
        browseOutputFolderButton->setEnabled(checked);
        if (!checked) {
            m_outputFolder.clear();
            outputFolderEdit->clear();
            outputFolderEdit->setPlaceholderText("Using Main Output Folder");
        } else {
            outputFolderEdit->setPlaceholderText("Select output folder...");
        }
    });
    destLayout->addWidget(useCustomOutputFolderCheck);

    // Filename preview
    auto* previewLabel = new QLabel();
    QString hexId = QString("%1").arg(m_filterId >= 0 ? m_filterId : 0, 4, 16, QChar('0')).toUpper();
    previewLabel->setText(QString("<i>Output: &lt;input_name&gt;_%1-waveform.png</i>").arg(hexId));
    previewLabel->setStyleSheet("color: #808080;");
    destLayout->addWidget(previewLabel);

    mainLayout->addWidget(destGroup);
    mainLayout->addStretch();

    return parametersWidget;
}

void FFShowwavespic::onBrowseOutputFolder() {
    QString defaultPath = m_outputFolder.isEmpty() 
        ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        : m_outputFolder;
    
    QString folder = QFileDialog::getExistingDirectory(
        parametersWidget,
        "Select output folder for waveform image...",
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

void FFShowwavespic::updateFFmpegFlags() {
    // Build color string (strip # from hex colors)
    QString colorStr = QString("%1|%2")
        .arg(m_color1.name().mid(1))
        .arg(m_color2.name().mid(1));
    
    // Build filter portion only (no output flags here)
    ffmpegFlags = QString("showwavespic=s=%1x%2:colors=%3:scale=%4:draw=%5:filter=%6:split_channels=%7")
        .arg(m_width)
        .arg(m_height)
        .arg(colorStr)
        .arg(scaleModeToString(m_scaleMode))
        .arg(drawModeToString(m_drawMode))
        .arg(filterModeToString(m_filterMode))
        .arg(m_splitChannels ? 1 : 0);
}

void FFShowwavespic::updateVideoCodecFlags() {
    // Video output flags for the -map section
    m_videoCodecFlags = "-frames:v 1 -c:v png";
}

QString FFShowwavespic::buildFFmpegFlags() const {
    // Return only the filter portion
    // The asplit branching and output mapping are handled by FilterChain
    return ffmpegFlags;
}

QString FFShowwavespic::getDefaultCustomCommandTemplate() const {
    return "showwavespic=s=1600x400:colors=ff6b35|004e89:scale=sqrt:draw=full:split_channels=1";
}

void FFShowwavespic::toJSON(QJsonObject& json) const {
    json["type"] = "ff-showwavespic";
    
    // Image settings
    json["width"] = m_width;
    json["height"] = m_height;
    json["color1"] = m_color1.name();
    json["color2"] = m_color2.name();
    json["scaleMode"] = static_cast<int>(m_scaleMode);
    json["drawMode"] = static_cast<int>(m_drawMode);
    json["filterMode"] = static_cast<int>(m_filterMode);
    json["splitChannels"] = m_splitChannels;
    
    // Output settings
    json["outputFolder"] = m_outputFolder;
    json["useCustomOutputFolder"] = m_useCustomOutputFolder;
}

void FFShowwavespic::fromJSON(const QJsonObject& json) {
    // Image settings
    m_width = json["width"].toInt(1600);
    m_height = json["height"].toInt(400);
    m_color1 = QColor(json["color1"].toString("#FF6B35"));
    m_color2 = QColor(json["color2"].toString("#004E89"));
    m_scaleMode = static_cast<ScaleMode>(json["scaleMode"].toInt(1));  // Default SQRT
    m_drawMode = static_cast<DrawMode>(json["drawMode"].toInt(0));     // Default FULL
    m_filterMode = static_cast<FilterMode>(json["filterMode"].toInt(0)); // Default AVERAGE
    m_splitChannels = json["splitChannels"].toBool(true);
    
    // Output settings
    m_outputFolder = json["outputFolder"].toString();
    m_useCustomOutputFolder = json["useCustomOutputFolder"].toBool(false);
    
    updateFFmpegFlags();
    updateVideoCodecFlags();
    
    // Update UI if it exists
    if (widthSpinBox) widthSpinBox->setValue(m_width);
    if (heightSpinBox) heightSpinBox->setValue(m_height);
    if (color1Button) color1Button->setStyleSheet(QString("background-color: %1;").arg(m_color1.name()));
    if (color2Button) color2Button->setStyleSheet(QString("background-color: %1;").arg(m_color2.name()));
    if (scaleModeCombo) scaleModeCombo->setCurrentIndex(static_cast<int>(m_scaleMode));
    if (drawModeCombo) drawModeCombo->setCurrentIndex(static_cast<int>(m_drawMode));
    if (filterModeCombo) filterModeCombo->setCurrentIndex(static_cast<int>(m_filterMode));
    if (splitChannelsCheckBox) splitChannelsCheckBox->setChecked(m_splitChannels);
    
    if (useCustomOutputFolderCheck) {
        useCustomOutputFolderCheck->setChecked(m_useCustomOutputFolder);
    }
    if (outputFolderEdit) {
        outputFolderEdit->setText(m_outputFolder);
        outputFolderEdit->setEnabled(m_useCustomOutputFolder);
        outputFolderEdit->setPlaceholderText(m_useCustomOutputFolder ? "Select output folder..." : "Using Main Output Folder");
    }
    if (browseOutputFolderButton) {
        browseOutputFolderButton->setEnabled(m_useCustomOutputFolder);
    }
}
