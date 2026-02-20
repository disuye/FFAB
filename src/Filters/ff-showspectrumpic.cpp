#include "ff-showspectrumpic.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QJsonObject>

FFShowspectrumpic::FFShowspectrumpic() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
    updateVideoCodecFlags();
}

void FFShowspectrumpic::setFilterId(int id) {
    BaseFilter::setFilterId(id);
    updateFFmpegFlags();
}

QString FFShowspectrumpic::getFilenameSuffix() const {
    QString hexId = QString("%1").arg(m_filterId, 4, 16, QChar('0')).toUpper();
    return QString("_%1-spectrum").arg(hexId);
}

QString FFShowspectrumpic::colorSchemeToString(ColorScheme scheme) const {
    switch (scheme) {
        case ColorScheme::CHANNEL:   return "channel";
        case ColorScheme::INTENSITY: return "intensity";
        case ColorScheme::RAINBOW:   return "rainbow";
        case ColorScheme::MORELAND:  return "moreland";
        case ColorScheme::NEBULAE:   return "nebulae";
        case ColorScheme::FIRE:      return "fire";
        case ColorScheme::FIERY:     return "fiery";
        case ColorScheme::FRUIT:     return "fruit";
        case ColorScheme::COOL:      return "cool";
        case ColorScheme::MAGMA:     return "magma";
        case ColorScheme::GREEN:     return "green";
        case ColorScheme::VIRIDIS:   return "viridis";
        case ColorScheme::PLASMA:    return "plasma";
        case ColorScheme::CIVIDIS:   return "cividis";
        case ColorScheme::TERRAIN:   return "terrain";
    }
    return "intensity";
}

QString FFShowspectrumpic::scaleModeToString(ScaleMode mode) const {
    switch (mode) {
        case ScaleMode::LINEAR:   return "lin";
        case ScaleMode::SQRT:     return "sqrt";
        case ScaleMode::CBRT:     return "cbrt";
        case ScaleMode::LOG:      return "log";
        case ScaleMode::FOURTHRT: return "4thrt";
        case ScaleMode::FIFTHRT:  return "5thrt";
    }
    return "log";
}

QString FFShowspectrumpic::freqScaleToString(FreqScale scale) const {
    switch (scale) {
        case FreqScale::LINEAR: return "lin";
        case FreqScale::LOG:    return "log";
    }
    return "lin";
}

QString FFShowspectrumpic::orientationToString(Orientation orient) const {
    switch (orient) {
        case Orientation::VERTICAL:   return "vertical";
        case Orientation::HORIZONTAL: return "horizontal";
    }
    return "vertical";
}

QString FFShowspectrumpic::windowFuncToString(WindowFunc func) const {
    switch (func) {
        case WindowFunc::RECT:     return "rect";
        case WindowFunc::HANN:     return "hann";
        case WindowFunc::HAMMING:  return "hamming";
        case WindowFunc::BLACKMAN: return "blackman";
        case WindowFunc::BARTLETT: return "bartlett";
        case WindowFunc::WELCH:    return "welch";
        case WindowFunc::FLATTOP:  return "flattop";
        case WindowFunc::BHARRIS:  return "bharris";
        case WindowFunc::BNUTTALL: return "bnuttall";
        case WindowFunc::SINE:     return "sine";
        case WindowFunc::NUTTALL:  return "nuttall";
        case WindowFunc::BHANN:    return "bhann";
        case WindowFunc::LANCZOS:  return "lanczos";
        case WindowFunc::GAUSS:    return "gauss";
        case WindowFunc::TUKEY:    return "tukey";
        case WindowFunc::DOLPH:    return "dolph";
        case WindowFunc::CAUCHY:   return "cauchy";
        case WindowFunc::PARZEN:   return "parzen";
        case WindowFunc::POISSON:  return "poisson";
        case WindowFunc::BOHMAN:   return "bohman";
        case WindowFunc::KAISER:   return "kaiser";
    }
    return "hann";
}

QWidget* FFShowspectrumpic::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto* mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(12);

    // Info label
    auto* infoLabel = new QLabel("Creates a PNG spectrum image alongside the audio output.");
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
    widthSpinBox->setMaximum(8192);
    widthSpinBox->setValue(m_width);
    widthSpinBox->setSingleStep(256);
    widthSpinBox->setSuffix(" px");
    connect(widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_width = val;
        updateFFmpegFlags();
    });
    sizeLayout->addWidget(widthSpinBox);

    sizeLayout->addSpacing(16);
    sizeLayout->addWidget(new QLabel("Height:"));
    heightSpinBox = new QSpinBox();
    heightSpinBox->setMinimum(100);
    heightSpinBox->setMaximum(4096);
    heightSpinBox->setValue(m_height);
    heightSpinBox->setSingleStep(128);
    heightSpinBox->setSuffix(" px");
    connect(heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_height = val;
        updateFFmpegFlags();
    });
    sizeLayout->addWidget(heightSpinBox);
    sizeLayout->addStretch();
    settingsLayout->addLayout(sizeLayout);

    // Color Scheme
    auto* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Color:"));
    colorSchemeCombo = new QComboBox();
    colorSchemeCombo->addItem("Channel",   static_cast<int>(ColorScheme::CHANNEL));
    colorSchemeCombo->addItem("Intensity", static_cast<int>(ColorScheme::INTENSITY));
    colorSchemeCombo->addItem("Rainbow",   static_cast<int>(ColorScheme::RAINBOW));
    colorSchemeCombo->addItem("Moreland",  static_cast<int>(ColorScheme::MORELAND));
    colorSchemeCombo->addItem("Nebulae",   static_cast<int>(ColorScheme::NEBULAE));
    colorSchemeCombo->addItem("Fire",      static_cast<int>(ColorScheme::FIRE));
    colorSchemeCombo->addItem("Fiery",     static_cast<int>(ColorScheme::FIERY));
    colorSchemeCombo->addItem("Fruit",     static_cast<int>(ColorScheme::FRUIT));
    colorSchemeCombo->addItem("Cool",      static_cast<int>(ColorScheme::COOL));
    colorSchemeCombo->addItem("Magma",     static_cast<int>(ColorScheme::MAGMA));
    colorSchemeCombo->addItem("Green",     static_cast<int>(ColorScheme::GREEN));
    colorSchemeCombo->addItem("Viridis",   static_cast<int>(ColorScheme::VIRIDIS));
    colorSchemeCombo->addItem("Plasma",    static_cast<int>(ColorScheme::PLASMA));
    colorSchemeCombo->addItem("Cividis",   static_cast<int>(ColorScheme::CIVIDIS));
    colorSchemeCombo->addItem("Terrain",   static_cast<int>(ColorScheme::TERRAIN));
    colorSchemeCombo->setCurrentIndex(static_cast<int>(m_colorScheme));
    connect(colorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_colorScheme = static_cast<ColorScheme>(idx);
        updateFFmpegFlags();
    });
    colorLayout->addWidget(colorSchemeCombo);
    colorLayout->addStretch();
    settingsLayout->addLayout(colorLayout);

    // Scale Mode
    auto* scaleLayout = new QHBoxLayout();
    scaleLayout->addWidget(new QLabel("Scale:"));
    scaleModeCombo = new QComboBox();
    scaleModeCombo->addItem("Linear",      static_cast<int>(ScaleMode::LINEAR));
    scaleModeCombo->addItem("Square Root", static_cast<int>(ScaleMode::SQRT));
    scaleModeCombo->addItem("Cubic Root",  static_cast<int>(ScaleMode::CBRT));
    scaleModeCombo->addItem("Logarithmic", static_cast<int>(ScaleMode::LOG));
    scaleModeCombo->addItem("4th Root",    static_cast<int>(ScaleMode::FOURTHRT));
    scaleModeCombo->addItem("5th Root",    static_cast<int>(ScaleMode::FIFTHRT));
    scaleModeCombo->setCurrentIndex(static_cast<int>(m_scaleMode));
    connect(scaleModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_scaleMode = static_cast<ScaleMode>(idx);
        updateFFmpegFlags();
    });
    scaleLayout->addWidget(scaleModeCombo);
    scaleLayout->addStretch();
    settingsLayout->addLayout(scaleLayout);

    // Frequency Scale
    auto* fscaleLayout = new QHBoxLayout();
    fscaleLayout->addWidget(new QLabel("Freq Scale:"));
    freqScaleCombo = new QComboBox();
    freqScaleCombo->addItem("Linear",      static_cast<int>(FreqScale::LINEAR));
    freqScaleCombo->addItem("Logarithmic", static_cast<int>(FreqScale::LOG));
    freqScaleCombo->setCurrentIndex(static_cast<int>(m_freqScale));
    connect(freqScaleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_freqScale = static_cast<FreqScale>(idx);
        updateFFmpegFlags();
    });
    fscaleLayout->addWidget(freqScaleCombo);
    fscaleLayout->addStretch();
    settingsLayout->addLayout(fscaleLayout);

    // Orientation
    auto* orientLayout = new QHBoxLayout();
    orientLayout->addWidget(new QLabel("Orientation:"));
    orientationCombo = new QComboBox();
    orientationCombo->addItem("Vertical",   static_cast<int>(Orientation::VERTICAL));
    orientationCombo->addItem("Horizontal", static_cast<int>(Orientation::HORIZONTAL));
    orientationCombo->setCurrentIndex(static_cast<int>(m_orientation));
    connect(orientationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_orientation = static_cast<Orientation>(idx);
        updateFFmpegFlags();
    });
    orientLayout->addWidget(orientationCombo);
    orientLayout->addStretch();
    settingsLayout->addLayout(orientLayout);

    // Window Function
    auto* winLayout = new QHBoxLayout();
    winLayout->addWidget(new QLabel("Window:"));
    windowFuncCombo = new QComboBox();
    windowFuncCombo->addItem("Rectangular",     static_cast<int>(WindowFunc::RECT));
    windowFuncCombo->addItem("Hann",            static_cast<int>(WindowFunc::HANN));
    windowFuncCombo->addItem("Hamming",         static_cast<int>(WindowFunc::HAMMING));
    windowFuncCombo->addItem("Blackman",        static_cast<int>(WindowFunc::BLACKMAN));
    windowFuncCombo->addItem("Bartlett",        static_cast<int>(WindowFunc::BARTLETT));
    windowFuncCombo->addItem("Welch",           static_cast<int>(WindowFunc::WELCH));
    windowFuncCombo->addItem("Flat-top",        static_cast<int>(WindowFunc::FLATTOP));
    windowFuncCombo->addItem("Blackman-Harris", static_cast<int>(WindowFunc::BHARRIS));
    windowFuncCombo->addItem("Blackman-Nuttall",static_cast<int>(WindowFunc::BNUTTALL));
    windowFuncCombo->addItem("Sine",            static_cast<int>(WindowFunc::SINE));
    windowFuncCombo->addItem("Nuttall",         static_cast<int>(WindowFunc::NUTTALL));
    windowFuncCombo->addItem("Bartlett-Hann",   static_cast<int>(WindowFunc::BHANN));
    windowFuncCombo->addItem("Lanczos",         static_cast<int>(WindowFunc::LANCZOS));
    windowFuncCombo->addItem("Gauss",           static_cast<int>(WindowFunc::GAUSS));
    windowFuncCombo->addItem("Tukey",           static_cast<int>(WindowFunc::TUKEY));
    windowFuncCombo->addItem("Dolph-Chebyshev", static_cast<int>(WindowFunc::DOLPH));
    windowFuncCombo->addItem("Cauchy",          static_cast<int>(WindowFunc::CAUCHY));
    windowFuncCombo->addItem("Parzen",          static_cast<int>(WindowFunc::PARZEN));
    windowFuncCombo->addItem("Poisson",         static_cast<int>(WindowFunc::POISSON));
    windowFuncCombo->addItem("Bohman",          static_cast<int>(WindowFunc::BOHMAN));
    windowFuncCombo->addItem("Kaiser",          static_cast<int>(WindowFunc::KAISER));
    windowFuncCombo->setCurrentIndex(static_cast<int>(m_windowFunc));
    connect(windowFuncCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_windowFunc = static_cast<WindowFunc>(idx);
        updateFFmpegFlags();
    });
    winLayout->addWidget(windowFuncCombo);
    winLayout->addStretch();
    settingsLayout->addLayout(winLayout);

    // Display Mode + Legend
    auto* modeLayout = new QHBoxLayout();
    displayModeCombo = new QComboBox();
    displayModeCombo->addItem("Combined Channels", static_cast<int>(DisplayMode::COMBINED));
    displayModeCombo->addItem("Separate Channels", static_cast<int>(DisplayMode::SEPARATE));
    displayModeCombo->setCurrentIndex(static_cast<int>(m_displayMode));
    connect(displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_displayMode = static_cast<DisplayMode>(idx);
        updateFFmpegFlags();
    });
    modeLayout->addWidget(displayModeCombo);

    legendCheckBox = new QCheckBox("Legend");
    legendCheckBox->setChecked(m_legend);
    connect(legendCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_legend = checked;
        updateFFmpegFlags();
    });
    modeLayout->addWidget(legendCheckBox);
    modeLayout->addStretch();
    settingsLayout->addLayout(modeLayout);

    mainLayout->addWidget(settingsGroup);

    // ========== ADVANCED SETTINGS ==========
    auto* advGroup = new QGroupBox("Advanced");
    auto* advLayout = new QFormLayout(advGroup);

    // Gain
    gainSpinBox = new QDoubleSpinBox();
    gainSpinBox->setMinimum(0.0);
    gainSpinBox->setMaximum(128.0);
    gainSpinBox->setValue(m_gain);
    gainSpinBox->setSingleStep(0.5);
    gainSpinBox->setDecimals(1);
    connect(gainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double val) {
        m_gain = static_cast<float>(val);
        updateFFmpegFlags();
    });
    advLayout->addRow("Gain:", gainSpinBox);

    // Saturation
    saturationSpinBox = new QDoubleSpinBox();
    saturationSpinBox->setMinimum(-10.0);
    saturationSpinBox->setMaximum(10.0);
    saturationSpinBox->setValue(m_saturation);
    saturationSpinBox->setSingleStep(0.1);
    saturationSpinBox->setDecimals(1);
    connect(saturationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double val) {
        m_saturation = static_cast<float>(val);
        updateFFmpegFlags();
    });
    advLayout->addRow("Saturation:", saturationSpinBox);

    // Dynamic Range
    dynamicRangeSpinBox = new QDoubleSpinBox();
    dynamicRangeSpinBox->setMinimum(10.0);
    dynamicRangeSpinBox->setMaximum(200.0);
    dynamicRangeSpinBox->setValue(m_dynamicRange);
    dynamicRangeSpinBox->setSingleStep(5.0);
    dynamicRangeSpinBox->setDecimals(0);
    dynamicRangeSpinBox->setSuffix(" dB");
    connect(dynamicRangeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double val) {
        m_dynamicRange = static_cast<float>(val);
        updateFFmpegFlags();
    });
    advLayout->addRow("Dynamic Range:", dynamicRangeSpinBox);

    // Start Frequency
    startFreqSpinBox = new QSpinBox();
    startFreqSpinBox->setMinimum(0);
    startFreqSpinBox->setMaximum(96000);
    startFreqSpinBox->setValue(m_startFreq);
    startFreqSpinBox->setSingleStep(100);
    startFreqSpinBox->setSuffix(" Hz");
    startFreqSpinBox->setSpecialValueText("Auto");
    connect(startFreqSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_startFreq = val;
        updateFFmpegFlags();
    });
    advLayout->addRow("Start Freq:", startFreqSpinBox);

    // Stop Frequency
    stopFreqSpinBox = new QSpinBox();
    stopFreqSpinBox->setMinimum(0);
    stopFreqSpinBox->setMaximum(96000);
    stopFreqSpinBox->setValue(m_stopFreq);
    stopFreqSpinBox->setSingleStep(1000);
    stopFreqSpinBox->setSuffix(" Hz");
    stopFreqSpinBox->setSpecialValueText("Auto");
    connect(stopFreqSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int val) {
        m_stopFreq = val;
        updateFFmpegFlags();
    });
    advLayout->addRow("Stop Freq:", stopFreqSpinBox);

    mainLayout->addWidget(advGroup);

    // ========== OUTPUT DESTINATION ==========
    destGroup = new QGroupBox("Output Destination");
    auto* destLayout = new QVBoxLayout(destGroup);

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
    connect(browseOutputFolderButton, &QPushButton::clicked, this, &FFShowspectrumpic::onBrowseOutputFolder);
    folderLayout->addWidget(browseOutputFolderButton);

    destLayout->addLayout(folderLayout);

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
    previewLabel->setText(QString("<i>Output: &lt;input_name&gt;_%1-spectrum.png</i>").arg(hexId));
    previewLabel->setStyleSheet("color: #808080;");
    destLayout->addWidget(previewLabel);

    mainLayout->addWidget(destGroup);
    mainLayout->addStretch();

    return parametersWidget;
}

void FFShowspectrumpic::onBrowseOutputFolder() {
    QString defaultPath = m_outputFolder.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        : m_outputFolder;

    QString folder = QFileDialog::getExistingDirectory(
        parametersWidget,
        "Select output folder for spectrum image...",
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

void FFShowspectrumpic::updateFFmpegFlags() {
    QStringList params;

    params << QString("s=%1x%2").arg(m_width).arg(m_height);
    params << QString("color=%1").arg(colorSchemeToString(m_colorScheme));
    params << QString("scale=%1").arg(scaleModeToString(m_scaleMode));

    if (m_displayMode != DisplayMode::COMBINED) {
        params << QString("mode=%1").arg(m_displayMode == DisplayMode::SEPARATE ? "separate" : "combined");
    }
    if (m_freqScale != FreqScale::LINEAR) {
        params << QString("fscale=%1").arg(freqScaleToString(m_freqScale));
    }
    if (m_orientation != Orientation::VERTICAL) {
        params << QString("orientation=%1").arg(orientationToString(m_orientation));
    }
    if (m_windowFunc != WindowFunc::HANN) {
        params << QString("win_func=%1").arg(windowFuncToString(m_windowFunc));
    }
    if (!m_legend) {
        params << "legend=0";
    }
    if (m_gain != 1.0f) {
        params << QString("gain=%1").arg(m_gain, 0, 'f', 1);
    }
    if (m_saturation != 1.0f) {
        params << QString("saturation=%1").arg(m_saturation, 0, 'f', 1);
    }
    if (m_dynamicRange != 120.0f) {
        params << QString("drange=%1").arg(m_dynamicRange, 0, 'f', 0);
    }
    if (m_startFreq > 0) {
        params << QString("start=%1").arg(m_startFreq);
    }
    if (m_stopFreq > 0) {
        params << QString("stop=%1").arg(m_stopFreq);
    }

    ffmpegFlags = "showspectrumpic=" + params.join(":");
}

void FFShowspectrumpic::updateVideoCodecFlags() {
    m_videoCodecFlags = "-frames:v 1 -c:v png";
}

QString FFShowspectrumpic::buildFFmpegFlags() const {
    return ffmpegFlags;
}

QString FFShowspectrumpic::getDefaultCustomCommandTemplate() const {
    return "showspectrumpic=s=2048x1024:color=intensity:scale=log:legend=1";
}

void FFShowspectrumpic::toJSON(QJsonObject& json) const {
    json["type"] = "ff-showspectrumpic";

    json["width"] = m_width;
    json["height"] = m_height;
    json["displayMode"] = static_cast<int>(m_displayMode);
    json["colorScheme"] = static_cast<int>(m_colorScheme);
    json["scaleMode"] = static_cast<int>(m_scaleMode);
    json["freqScale"] = static_cast<int>(m_freqScale);
    json["orientation"] = static_cast<int>(m_orientation);
    json["windowFunc"] = static_cast<int>(m_windowFunc);
    json["legend"] = m_legend;
    json["gain"] = static_cast<double>(m_gain);
    json["saturation"] = static_cast<double>(m_saturation);
    json["dynamicRange"] = static_cast<double>(m_dynamicRange);
    json["startFreq"] = m_startFreq;
    json["stopFreq"] = m_stopFreq;

    json["outputFolder"] = m_outputFolder;
    json["useCustomOutputFolder"] = m_useCustomOutputFolder;
}

void FFShowspectrumpic::fromJSON(const QJsonObject& json) {
    m_width = json["width"].toInt(2048);
    m_height = json["height"].toInt(1024);
    m_displayMode = static_cast<DisplayMode>(json["displayMode"].toInt(0));
    m_colorScheme = static_cast<ColorScheme>(json["colorScheme"].toInt(1));
    m_scaleMode = static_cast<ScaleMode>(json["scaleMode"].toInt(3));
    m_freqScale = static_cast<FreqScale>(json["freqScale"].toInt(0));
    m_orientation = static_cast<Orientation>(json["orientation"].toInt(0));
    m_windowFunc = static_cast<WindowFunc>(json["windowFunc"].toInt(1));
    m_legend = json["legend"].toBool(true);
    m_gain = static_cast<float>(json["gain"].toDouble(1.0));
    m_saturation = static_cast<float>(json["saturation"].toDouble(1.0));
    m_dynamicRange = static_cast<float>(json["dynamicRange"].toDouble(120.0));
    m_startFreq = json["startFreq"].toInt(0);
    m_stopFreq = json["stopFreq"].toInt(0);

    m_outputFolder = json["outputFolder"].toString();
    m_useCustomOutputFolder = json["useCustomOutputFolder"].toBool(false);

    updateFFmpegFlags();
    updateVideoCodecFlags();

    // Update UI if it exists
    if (widthSpinBox) widthSpinBox->setValue(m_width);
    if (heightSpinBox) heightSpinBox->setValue(m_height);
    if (displayModeCombo) displayModeCombo->setCurrentIndex(static_cast<int>(m_displayMode));
    if (colorSchemeCombo) colorSchemeCombo->setCurrentIndex(static_cast<int>(m_colorScheme));
    if (scaleModeCombo) scaleModeCombo->setCurrentIndex(static_cast<int>(m_scaleMode));
    if (freqScaleCombo) freqScaleCombo->setCurrentIndex(static_cast<int>(m_freqScale));
    if (orientationCombo) orientationCombo->setCurrentIndex(static_cast<int>(m_orientation));
    if (windowFuncCombo) windowFuncCombo->setCurrentIndex(static_cast<int>(m_windowFunc));
    if (legendCheckBox) legendCheckBox->setChecked(m_legend);
    if (gainSpinBox) gainSpinBox->setValue(m_gain);
    if (saturationSpinBox) saturationSpinBox->setValue(m_saturation);
    if (dynamicRangeSpinBox) dynamicRangeSpinBox->setValue(m_dynamicRange);
    if (startFreqSpinBox) startFreqSpinBox->setValue(m_startFreq);
    if (stopFreqSpinBox) stopFreqSpinBox->setValue(m_stopFreq);

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
