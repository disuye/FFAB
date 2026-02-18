#include "ff-anequalizer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <QSplitter>

FFAnequalizer::FFAnequalizer() {
    position = Position::MIDDLE;
    // Start with one default band
    m_bands.append({1000.0, 100.0, 0.0, 0, -1});
    updateFFmpegFlags();
}

QWidget* FFAnequalizer::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(8);

    // ===== BAND LIST =====
    auto listGroup = new QGroupBox("EQ Bands");
    auto listLayout = new QVBoxLayout(listGroup);

    bandList = new QListWidget();
    bandList->setMaximumHeight(120);
    listLayout->addWidget(bandList);

    auto buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("+ Add Band");
    removeButton = new QPushButton("- Remove");
    removeButton->setEnabled(false);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();
    listLayout->addLayout(buttonLayout);

    mainLayout->addWidget(listGroup);

    // ===== BAND EDITOR =====
    bandEditorWidget = new QWidget();
    auto editorLayout = new QVBoxLayout(bandEditorWidget);
    editorLayout->setContentsMargins(0, 0, 0, 0);

    // Frequency
    auto freqGroup = new QGroupBox("Frequency (Hz)");
    auto freqLayout = new QVBoxLayout(freqGroup);

    freqSpinBox = new QDoubleSpinBox();
    freqSpinBox->setMinimum(20.0);
    freqSpinBox->setMaximum(20000.0);
    freqSpinBox->setValue(1000.0);
    freqSpinBox->setDecimals(1);
    freqSpinBox->setSuffix(" Hz");

    auto freqControlLayout = new QHBoxLayout();
    freqControlLayout->addWidget(freqSpinBox);
    freqControlLayout->addWidget(new QLabel("20"));
    freqSlider = new QSlider(Qt::Horizontal);
    freqSlider->setMinimum(20);
    freqSlider->setMaximum(20000);
    freqSlider->setValue(1000);
    freqControlLayout->addWidget(freqSlider, 1);
    freqControlLayout->addWidget(new QLabel("20k"));
    freqLayout->addLayout(freqControlLayout);
    editorLayout->addWidget(freqGroup);

    // Gain
    auto gainGroup = new QGroupBox("Gain (dB)");
    auto gainLayout = new QVBoxLayout(gainGroup);

    gainSpinBox = new QDoubleSpinBox();
    gainSpinBox->setMinimum(-30.0);
    gainSpinBox->setMaximum(30.0);
    gainSpinBox->setValue(0.0);
    gainSpinBox->setDecimals(1);
    gainSpinBox->setSuffix(" dB");

    auto gainControlLayout = new QHBoxLayout();
    gainControlLayout->addWidget(gainSpinBox);
    gainControlLayout->addWidget(new QLabel("-30"));
    gainSlider = new QSlider(Qt::Horizontal);
    gainSlider->setMinimum(-300);
    gainSlider->setMaximum(300);
    gainSlider->setValue(0);
    gainControlLayout->addWidget(gainSlider, 1);
    gainControlLayout->addWidget(new QLabel("+30"));
    gainLayout->addLayout(gainControlLayout);
    editorLayout->addWidget(gainGroup);

    // Width (bandwidth)
    auto widthGroup = new QGroupBox("Bandwidth");
    auto widthLayout = new QVBoxLayout(widthGroup);

    widthSpinBox = new QDoubleSpinBox();
    widthSpinBox->setMinimum(10.0);
    widthSpinBox->setMaximum(5000.0);
    widthSpinBox->setValue(100.0);
    widthSpinBox->setDecimals(1);

    auto widthControlLayout = new QHBoxLayout();
    widthControlLayout->addWidget(widthSpinBox);
    widthControlLayout->addWidget(new QLabel("10"));
    widthSlider = new QSlider(Qt::Horizontal);
    widthSlider->setMinimum(10);
    widthSlider->setMaximum(5000);
    widthSlider->setValue(100);
    widthControlLayout->addWidget(widthSlider, 1);
    widthControlLayout->addWidget(new QLabel("5000"));
    widthLayout->addLayout(widthControlLayout);
    editorLayout->addWidget(widthGroup);

    // Type and Channel row
    auto optionsLayout = new QHBoxLayout();

    auto typeGroup = new QGroupBox("Filter Type");
    auto typeLayout = new QVBoxLayout(typeGroup);
    typeCombo = new QComboBox();
    typeCombo->addItem("Butterworth", 0);
    typeCombo->addItem("Chebyshev I", 1);
    typeCombo->addItem("Chebyshev II", 2);
    typeCombo->addItem("Elliptic", 3);
    typeLayout->addWidget(typeCombo);
    optionsLayout->addWidget(typeGroup);

    auto channelGroup = new QGroupBox("Channel");
    auto channelLayout = new QVBoxLayout(channelGroup);
    channelCombo = new QComboBox();
    channelCombo->addItem("All Channels", -1);
    channelCombo->addItem("Channel 0 (L)", 0);
    channelCombo->addItem("Channel 1 (R)", 1);
    channelCombo->addItem("Channel 2", 2);
    channelCombo->addItem("Channel 3", 3);
    channelLayout->addWidget(channelCombo);
    optionsLayout->addWidget(channelGroup);

    editorLayout->addLayout(optionsLayout);

    mainLayout->addWidget(bandEditorWidget);
    bandEditorWidget->setEnabled(false);

    mainLayout->addStretch();

    // Populate initial band list
    refreshBandList();

    // ========== SIGNAL CONNECTIONS ==========

    connect(bandList, &QListWidget::currentRowChanged, [this](int row) {
        selectBand(row);
    });

    connect(addButton, &QPushButton::clicked, [this]() {
        addBand();
    });

    connect(removeButton, &QPushButton::clicked, [this]() {
        removeBand();
    });

    // Frequency
    connect(freqSlider, &QSlider::valueChanged, [this](int value) {
        freqSpinBox->blockSignals(true);
        freqSpinBox->setValue(value);
        freqSpinBox->blockSignals(false);
        updateBandFromUI();
    });
    connect(freqSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        freqSlider->blockSignals(true);
        freqSlider->setValue(static_cast<int>(value));
        freqSlider->blockSignals(false);
        updateBandFromUI();
    });

    // Gain
    connect(gainSlider, &QSlider::valueChanged, [this](int value) {
        gainSpinBox->blockSignals(true);
        gainSpinBox->setValue(value / 10.0);
        gainSpinBox->blockSignals(false);
        updateBandFromUI();
    });
    connect(gainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        gainSlider->blockSignals(true);
        gainSlider->setValue(static_cast<int>(value * 10));
        gainSlider->blockSignals(false);
        updateBandFromUI();
    });

    // Width
    connect(widthSlider, &QSlider::valueChanged, [this](int value) {
        widthSpinBox->blockSignals(true);
        widthSpinBox->setValue(value);
        widthSpinBox->blockSignals(false);
        updateBandFromUI();
    });
    connect(widthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        widthSlider->blockSignals(true);
        widthSlider->setValue(static_cast<int>(value));
        widthSlider->blockSignals(false);
        updateBandFromUI();
    });

    // Type
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        updateBandFromUI();
    });

    // Channel
    connect(channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        updateBandFromUI();
    });

    return parametersWidget;
}

void FFAnequalizer::addBand() {
    // Add a new band with default values
    EQBand newBand;
    newBand.freq = 1000.0;
    newBand.width = 100.0;
    newBand.gain = 0.0;
    newBand.type = 0;
    newBand.channel = -1;
    m_bands.append(newBand);
    refreshBandList();
    bandList->setCurrentRow(m_bands.size() - 1);
    updateFFmpegFlags();
}

void FFAnequalizer::removeBand() {
    if (m_selectedBand >= 0 && m_selectedBand < m_bands.size()) {
        m_bands.removeAt(m_selectedBand);
        refreshBandList();
        if (m_bands.isEmpty()) {
            m_selectedBand = -1;
            bandEditorWidget->setEnabled(false);
        } else {
            int newSelection = qMin(m_selectedBand, m_bands.size() - 1);
            bandList->setCurrentRow(newSelection);
        }
        updateFFmpegFlags();
    }
}

void FFAnequalizer::selectBand(int index) {
    m_selectedBand = index;
    bool valid = (index >= 0 && index < m_bands.size());
    bandEditorWidget->setEnabled(valid);
    removeButton->setEnabled(valid && m_bands.size() > 1);

    if (valid) {
        const EQBand& band = m_bands[index];

        freqSpinBox->blockSignals(true);
        freqSlider->blockSignals(true);
        freqSpinBox->setValue(band.freq);
        freqSlider->setValue(static_cast<int>(band.freq));
        freqSpinBox->blockSignals(false);
        freqSlider->blockSignals(false);

        gainSpinBox->blockSignals(true);
        gainSlider->blockSignals(true);
        gainSpinBox->setValue(band.gain);
        gainSlider->setValue(static_cast<int>(band.gain * 10));
        gainSpinBox->blockSignals(false);
        gainSlider->blockSignals(false);

        widthSpinBox->blockSignals(true);
        widthSlider->blockSignals(true);
        widthSpinBox->setValue(band.width);
        widthSlider->setValue(static_cast<int>(band.width));
        widthSpinBox->blockSignals(false);
        widthSlider->blockSignals(false);

        typeCombo->blockSignals(true);
        typeCombo->setCurrentIndex(band.type);
        typeCombo->blockSignals(false);

        channelCombo->blockSignals(true);
        int chIndex = channelCombo->findData(band.channel);
        if (chIndex >= 0) channelCombo->setCurrentIndex(chIndex);
        channelCombo->blockSignals(false);
    }
}

void FFAnequalizer::updateBandFromUI() {
    if (m_selectedBand < 0 || m_selectedBand >= m_bands.size()) return;

    EQBand& band = m_bands[m_selectedBand];
    band.freq = freqSpinBox->value();
    band.gain = gainSpinBox->value();
    band.width = widthSpinBox->value();
    band.type = typeCombo->currentData().toInt();
    band.channel = channelCombo->currentData().toInt();

    refreshBandList();
    bandList->setCurrentRow(m_selectedBand);
    updateFFmpegFlags();
}

void FFAnequalizer::refreshBandList() {
    bandList->blockSignals(true);
    bandList->clear();
    for (int i = 0; i < m_bands.size(); ++i) {
        const EQBand& band = m_bands[i];
        QString gainStr = (band.gain >= 0) ? QString("+%1").arg(band.gain, 0, 'f', 1) 
                                           : QString::number(band.gain, 'f', 1);
        QString chStr = (band.channel < 0) ? "All" : QString("Ch%1").arg(band.channel);
        QString text = QString("Band %1: %2 Hz, %3 dB (%4)")
                           .arg(i + 1)
                           .arg(band.freq, 0, 'f', 0)
                           .arg(gainStr)
                           .arg(chStr);
        bandList->addItem(text);
    }
    bandList->blockSignals(false);
}


// This generates a proper 'default' output, one string per audio channel...
// anequalizer=params='c0 f=1000.0 w=100.0 g=6.0 t=0|c1 f=1000.0 w=100.0 g=6.0 t=0'

void FFAnequalizer::updateFFmpegFlags() {
    if (m_bands.isEmpty()) {
        ffmpegFlags = "anequalizer";
        return;
    }

    QStringList bandParams;
    for (const EQBand& band : m_bands) {
        if (band.channel < 0) {
            // "All Channels" - duplicate for stereo (c0 and c1)
            QString param0 = QString("c0 f=%1 w=%2 g=%3 t=%4")
                                .arg(band.freq, 0, 'f', 1)
                                .arg(band.width, 0, 'f', 1)
                                .arg(band.gain, 0, 'f', 1)
                                .arg(band.type);
            QString param1 = QString("c1 f=%1 w=%2 g=%3 t=%4")
                                .arg(band.freq, 0, 'f', 1)
                                .arg(band.width, 0, 'f', 1)
                                .arg(band.gain, 0, 'f', 1)
                                .arg(band.type);
            bandParams.append(param0);
            bandParams.append(param1);
        } else {
            // Specific channel
            QString param = QString("c%1 f=%2 w=%3 g=%4 t=%5")
                                .arg(band.channel)
                                .arg(band.freq, 0, 'f', 1)
                                .arg(band.width, 0, 'f', 1)
                                .arg(band.gain, 0, 'f', 1)
                                .arg(band.type);
            bandParams.append(param);
        }
    }

    ffmpegFlags = QString("anequalizer=params='%1'").arg(bandParams.join("|"));
}

QString FFAnequalizer::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAnequalizer::toJSON(QJsonObject& json) const {
    json["type"] = "ff-anequalizer";
    
    QJsonArray bandsArray;
    for (const EQBand& band : m_bands) {
        QJsonObject bandObj;
        bandObj["freq"] = band.freq;
        bandObj["width"] = band.width;
        bandObj["gain"] = band.gain;
        bandObj["filterType"] = band.type;
        bandObj["channel"] = band.channel;
        bandsArray.append(bandObj);
    }
    json["bands"] = bandsArray;
}

void FFAnequalizer::fromJSON(const QJsonObject& json) {
    m_bands.clear();

    QJsonArray bandsArray = json["bands"].toArray();
    for (const QJsonValue& val : bandsArray) {
        QJsonObject bandObj = val.toObject();
        EQBand band;
        band.freq = bandObj["freq"].toDouble(1000.0);
        band.width = bandObj["width"].toDouble(100.0);
        band.gain = bandObj["gain"].toDouble(0.0);
        band.type = bandObj["filterType"].toInt(0);
        band.channel = bandObj["channel"].toInt(-1);
        m_bands.append(band);
    }

    if (m_bands.isEmpty()) {
        m_bands.append({1000.0, 100.0, 0.0, 0, -1});
    }

    updateFFmpegFlags();
}
