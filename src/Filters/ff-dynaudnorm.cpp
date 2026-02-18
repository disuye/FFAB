#include "ff-dynaudnorm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <cmath>

FFDynaudnorm::FFDynaudnorm() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFDynaudnorm::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(8);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Use tabs to organize the many parameters
    auto tabWidget = new QTabWidget();

    // TAB 1: Frame Settings
    auto frameTab = new QWidget();
    auto frameLayout = new QVBoxLayout(frameTab);

    // Frame Length (10 - 8000 ms)
    auto framelenGroup = new QGroupBox("Frame Length (ms)");
    auto framelenLayout = new QVBoxLayout(framelenGroup);

    framelenSpinBox = new QDoubleSpinBox();
    framelenSpinBox->setMinimum(10.0);
    framelenSpinBox->setMaximum(8000.0);
    framelenSpinBox->setValue(m_framelen);
    framelenSpinBox->setDecimals(0);
    framelenSpinBox->setSuffix(" ms");

    auto framelenControlLayout = new QHBoxLayout();
    framelenControlLayout->addWidget(framelenSpinBox);
    framelenControlLayout->addStretch();
    
    auto framelenMinLabel = new QLabel("10");
    framelenControlLayout->addWidget(framelenMinLabel);

    framelenSlider = new QSlider(Qt::Horizontal);
    framelenSlider->setMinimum(10);
    framelenSlider->setMaximum(8000);
    framelenSlider->setValue(m_framelen);
    framelenControlLayout->addWidget(framelenSlider, 1);
    
    auto framelenMaxLabel = new QLabel("8000");
    framelenControlLayout->addWidget(framelenMaxLabel);

    framelenLayout->addLayout(framelenControlLayout);
    frameLayout->addWidget(framelenGroup);

    // Gauss Size (3 - 301)
    auto gausssizeGroup = new QGroupBox("Filter Size");
    auto gausssizeLayout = new QVBoxLayout(gausssizeGroup);

    gausssizeSpinBox = new QDoubleSpinBox();
    gausssizeSpinBox->setMinimum(3.0);
    gausssizeSpinBox->setMaximum(301.0);
    gausssizeSpinBox->setValue(m_gausssize);
    gausssizeSpinBox->setDecimals(0);

    auto gausssizeControlLayout = new QHBoxLayout();
    gausssizeControlLayout->addWidget(gausssizeSpinBox);
    gausssizeControlLayout->addStretch();
    
    auto gausssizeMinLabel = new QLabel("3");
    gausssizeControlLayout->addWidget(gausssizeMinLabel);

    gausssizeSlider = new QSlider(Qt::Horizontal);
    gausssizeSlider->setMinimum(3);
    gausssizeSlider->setMaximum(301);
    gausssizeSlider->setValue(m_gausssize);
    gausssizeControlLayout->addWidget(gausssizeSlider, 1);
    
    auto gausssizeMaxLabel = new QLabel("301");
    gausssizeControlLayout->addWidget(gausssizeMaxLabel);

    gausssizeLayout->addLayout(gausssizeControlLayout);
    frameLayout->addWidget(gausssizeGroup);

    // Overlap (0 - 1)
    auto overlapGroup = new QGroupBox("Frame Overlap");
    auto overlapLayout = new QVBoxLayout(overlapGroup);

    overlapSpinBox = new QDoubleSpinBox();
    overlapSpinBox->setMinimum(0.0);
    overlapSpinBox->setMaximum(1.0);
    overlapSpinBox->setValue(m_overlap);
    overlapSpinBox->setDecimals(2);

    auto overlapControlLayout = new QHBoxLayout();
    overlapControlLayout->addWidget(overlapSpinBox);
    overlapControlLayout->addStretch();
    
    auto overlapMinLabel = new QLabel("0%");
    overlapControlLayout->addWidget(overlapMinLabel);

    overlapSlider = new QSlider(Qt::Horizontal);
    overlapSlider->setMinimum(0);
    overlapSlider->setMaximum(100);
    overlapSlider->setValue(m_overlap * 100);
    overlapControlLayout->addWidget(overlapSlider, 1);
    
    auto overlapMaxLabel = new QLabel("100%");
    overlapControlLayout->addWidget(overlapMaxLabel);

    overlapLayout->addLayout(overlapControlLayout);
    frameLayout->addWidget(overlapGroup);

    frameLayout->addStretch();
    tabWidget->addTab(frameTab, "Frame");

    // TAB 2: Peak & Gain
    auto peakTab = new QWidget();
    auto peakLayout = new QVBoxLayout(peakTab);

    // Peak (-96 to 0 dB)
    auto peakGroup = new QGroupBox("Peak Value (dB)");
    auto peakGLayout = new QVBoxLayout(peakGroup);

    peakSpinBox = new QDoubleSpinBox();
    peakSpinBox->setMinimum(-96.0);
    peakSpinBox->setMaximum(0.0);
    peakSpinBox->setValue(linearToDb(m_peak));
    peakSpinBox->setDecimals(1);
    peakSpinBox->setSingleStep(0.1);
    peakSpinBox->setSuffix(" dB");

    auto peakControlLayout = new QHBoxLayout();
    peakControlLayout->addWidget(peakSpinBox);
    peakControlLayout->addStretch();

    auto peakMinLabel = new QLabel("-96");
    peakControlLayout->addWidget(peakMinLabel);

    peakSlider = new QSlider(Qt::Horizontal);
    peakSlider->setMinimum(0);
    peakSlider->setMaximum(960);
    peakSlider->setValue(static_cast<int>((linearToDb(m_peak) + 96.0) * 10.0));
    peakControlLayout->addWidget(peakSlider, 1);

    auto peakMaxLabel = new QLabel("0");
    peakControlLayout->addWidget(peakMaxLabel);

    peakGLayout->addLayout(peakControlLayout);
    peakLayout->addWidget(peakGroup);

    // Max Gain (1 - 100)
    auto maxgainGroup = new QGroupBox("Max Amplification");
    auto maxgainLayout = new QVBoxLayout(maxgainGroup);

    maxgainSpinBox = new QDoubleSpinBox();
    maxgainSpinBox->setMinimum(1.0);
    maxgainSpinBox->setMaximum(100.0);
    maxgainSpinBox->setValue(m_maxgain);
    maxgainSpinBox->setDecimals(2);

    auto maxgainControlLayout = new QHBoxLayout();
    maxgainControlLayout->addWidget(maxgainSpinBox);
    maxgainControlLayout->addStretch();
    
    auto maxgainMinLabel = new QLabel("1.0");
    maxgainControlLayout->addWidget(maxgainMinLabel);

    maxgainSlider = new QSlider(Qt::Horizontal);
    maxgainSlider->setMinimum(100);
    maxgainSlider->setMaximum(10000);
    maxgainSlider->setValue(m_maxgain * 100);
    maxgainControlLayout->addWidget(maxgainSlider, 1);
    
    auto maxgainMaxLabel = new QLabel("100");
    maxgainControlLayout->addWidget(maxgainMaxLabel);

    maxgainLayout->addLayout(maxgainControlLayout);
    peakLayout->addWidget(maxgainGroup);

    // Target RMS (-96 to 0 dB)
    auto targetrmsGroup = new QGroupBox("Target RMS (dB)");
    auto targetrmsLayout = new QVBoxLayout(targetrmsGroup);

    targetrmsSpinBox = new QDoubleSpinBox();
    targetrmsSpinBox->setMinimum(-96.0);
    targetrmsSpinBox->setMaximum(0.0);
    targetrmsSpinBox->setValue(linearToDb(m_targetrms));
    targetrmsSpinBox->setDecimals(1);
    targetrmsSpinBox->setSingleStep(0.1);
    targetrmsSpinBox->setSuffix(" dB");

    auto targetrmsControlLayout = new QHBoxLayout();
    targetrmsControlLayout->addWidget(targetrmsSpinBox);
    targetrmsControlLayout->addStretch();

    auto targetrmsMinLabel = new QLabel("-96");
    targetrmsControlLayout->addWidget(targetrmsMinLabel);

    targetrmsSlider = new QSlider(Qt::Horizontal);
    targetrmsSlider->setMinimum(0);
    targetrmsSlider->setMaximum(960);
    targetrmsSlider->setValue(static_cast<int>((linearToDb(m_targetrms) + 96.0) * 10.0));
    targetrmsControlLayout->addWidget(targetrmsSlider, 1);

    auto targetrmsMaxLabel = new QLabel("0");
    targetrmsControlLayout->addWidget(targetrmsMaxLabel);

    targetrmsLayout->addLayout(targetrmsControlLayout);
    peakLayout->addWidget(targetrmsGroup);

    peakLayout->addStretch();
    tabWidget->addTab(peakTab, "Peak/Gain");

    // TAB 3: Compression & Threshold
    auto compressTab = new QWidget();
    auto compressLayout = new QVBoxLayout(compressTab);

    // Compress Factor (0 - 30)
    auto compressGroup = new QGroupBox("Compress Factor");
    auto compressGLayout = new QVBoxLayout(compressGroup);

    compressSpinBox = new QDoubleSpinBox();
    compressSpinBox->setMinimum(0.0);
    compressSpinBox->setMaximum(30.0);
    compressSpinBox->setValue(m_compress);
    compressSpinBox->setDecimals(2);

    auto compressControlLayout = new QHBoxLayout();
    compressControlLayout->addWidget(compressSpinBox);
    compressControlLayout->addStretch();
    
    auto compressMinLabel = new QLabel("0");
    compressControlLayout->addWidget(compressMinLabel);

    compressSlider = new QSlider(Qt::Horizontal);
    compressSlider->setMinimum(0);
    compressSlider->setMaximum(3000);
    compressSlider->setValue(m_compress * 100);
    compressControlLayout->addWidget(compressSlider, 1);
    
    auto compressMaxLabel = new QLabel("30");
    compressControlLayout->addWidget(compressMaxLabel);

    compressGLayout->addLayout(compressControlLayout);
    compressLayout->addWidget(compressGroup);

    // Threshold (-96 to 0 dB)
    auto thresholdGroup = new QGroupBox("Threshold Value (dB)");
    auto thresholdGLayout = new QVBoxLayout(thresholdGroup);

    thresholdSpinBox = new QDoubleSpinBox();
    thresholdSpinBox->setMinimum(-96.0);
    thresholdSpinBox->setMaximum(0.0);
    thresholdSpinBox->setValue(linearToDb(m_threshold));
    thresholdSpinBox->setDecimals(1);
    thresholdSpinBox->setSingleStep(0.1);
    thresholdSpinBox->setSuffix(" dB");

    auto thresholdControlLayout = new QHBoxLayout();
    thresholdControlLayout->addWidget(thresholdSpinBox);
    thresholdControlLayout->addStretch();

    auto thresholdMinLabel = new QLabel("-96");
    thresholdControlLayout->addWidget(thresholdMinLabel);

    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setMinimum(0);
    thresholdSlider->setMaximum(960);
    thresholdSlider->setValue(static_cast<int>((linearToDb(m_threshold) + 96.0) * 10.0));
    thresholdControlLayout->addWidget(thresholdSlider, 1);

    auto thresholdMaxLabel = new QLabel("0");
    thresholdControlLayout->addWidget(thresholdMaxLabel);

    thresholdGLayout->addLayout(thresholdControlLayout);
    compressLayout->addWidget(thresholdGroup);

    compressLayout->addStretch();
    tabWidget->addTab(compressTab, "Compression");

    // TAB 4: Options & Advanced
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Boolean options
    auto boolGroup = new QGroupBox("Options");
    auto boolLayout = new QVBoxLayout(boolGroup);
    
    couplingCheckBox = new QCheckBox("Channel Coupling");
    couplingCheckBox->setChecked(m_coupling);
    boolLayout->addWidget(couplingCheckBox);
    
    correctdcCheckBox = new QCheckBox("DC Correction");
    correctdcCheckBox->setChecked(m_correctdc);
    boolLayout->addWidget(correctdcCheckBox);
    
    altboundaryCheckBox = new QCheckBox("Alternative Boundary Mode");
    altboundaryCheckBox->setChecked(m_altboundary);
    boolLayout->addWidget(altboundaryCheckBox);
    
    optionsLayout->addWidget(boolGroup);

    // Channels string
    auto channelsGroup = new QGroupBox("Channels to Filter");
    auto channelsLayout = new QVBoxLayout(channelsGroup);
    auto channelsHint = new QLabel("Enter channel layout (e.g., 'all', 'FL+FR', 'stereo'):");
    channelsHint->setWordWrap(true);
    channelsLayout->addWidget(channelsHint);
    channelsEdit = new QLineEdit();
    channelsEdit->setText(m_channels);
    channelsEdit->setPlaceholderText("all");
    channelsLayout->addWidget(channelsEdit);
    optionsLayout->addWidget(channelsGroup);

    // Curve string
    auto curveGroup = new QGroupBox("Custom Peak Mapping Curve");
    auto curveLayout = new QVBoxLayout(curveGroup);
    auto curveHint = new QLabel("Optional custom curve expression (leave empty for default):");
    curveHint->setWordWrap(true);
    curveLayout->addWidget(curveHint);
    curveEdit = new QLineEdit();
    curveEdit->setText(m_curve);
    curveEdit->setPlaceholderText("(empty = default)");
    curveLayout->addWidget(curveEdit);
    optionsLayout->addWidget(curveGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // Connect all controls
    connect(framelenSlider, &QSlider::valueChanged, [this](int v) {
        m_framelen = v;
        framelenSpinBox->blockSignals(true);
        framelenSpinBox->setValue(v);
        framelenSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(framelenSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_framelen = static_cast<int>(v);
        framelenSlider->blockSignals(true);
        framelenSlider->setValue(v);
        framelenSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(gausssizeSlider, &QSlider::valueChanged, [this](int v) {
        m_gausssize = v;
        gausssizeSpinBox->blockSignals(true);
        gausssizeSpinBox->setValue(v);
        gausssizeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(gausssizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_gausssize = static_cast<int>(v);
        gausssizeSlider->blockSignals(true);
        gausssizeSlider->setValue(v);
        gausssizeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(peakSlider, &QSlider::valueChanged, [this, dbToLinear](int v) {
        double db = (v / 10.0) - 96.0;
        m_peak = dbToLinear(db);
        peakSpinBox->blockSignals(true);
        peakSpinBox->setValue(db);
        peakSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(peakSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_peak = dbToLinear(db);
        peakSlider->blockSignals(true);
        peakSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        peakSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(maxgainSlider, &QSlider::valueChanged, [this](int v) {
        m_maxgain = v / 100.0;
        maxgainSpinBox->blockSignals(true);
        maxgainSpinBox->setValue(m_maxgain);
        maxgainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(maxgainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_maxgain = v;
        maxgainSlider->blockSignals(true);
        maxgainSlider->setValue(v * 100);
        maxgainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(targetrmsSlider, &QSlider::valueChanged, [this, dbToLinear](int v) {
        double db = (v / 10.0) - 96.0;
        m_targetrms = (db <= -96.0) ? 0.0 : dbToLinear(db);
        targetrmsSpinBox->blockSignals(true);
        targetrmsSpinBox->setValue(db);
        targetrmsSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(targetrmsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_targetrms = (db <= -96.0) ? 0.0 : dbToLinear(db);
        targetrmsSlider->blockSignals(true);
        targetrmsSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        targetrmsSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(compressSlider, &QSlider::valueChanged, [this](int v) {
        m_compress = v / 100.0;
        compressSpinBox->blockSignals(true);
        compressSpinBox->setValue(m_compress);
        compressSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(compressSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_compress = v;
        compressSlider->blockSignals(true);
        compressSlider->setValue(v * 100);
        compressSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(thresholdSlider, &QSlider::valueChanged, [this, dbToLinear](int v) {
        double db = (v / 10.0) - 96.0;
        m_threshold = (db <= -96.0) ? 0.0 : dbToLinear(db);
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(db);
        thresholdSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_threshold = (db <= -96.0) ? 0.0 : dbToLinear(db);
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        thresholdSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(overlapSlider, &QSlider::valueChanged, [this](int v) {
        m_overlap = v / 100.0;
        overlapSpinBox->blockSignals(true);
        overlapSpinBox->setValue(m_overlap);
        overlapSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(overlapSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double v) {
        m_overlap = v;
        overlapSlider->blockSignals(true);
        overlapSlider->setValue(v * 100);
        overlapSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(couplingCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_coupling = checked;
        updateFFmpegFlags();
    });

    connect(correctdcCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_correctdc = checked;
        updateFFmpegFlags();
    });

    connect(altboundaryCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_altboundary = checked;
        updateFFmpegFlags();
    });

    connect(channelsEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_channels = text;
        updateFFmpegFlags();
    });

    connect(curveEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_curve = text;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFDynaudnorm::updateFFmpegFlags() {
    QStringList parts;
    parts << QString("framelen=%1").arg(m_framelen);
    parts << QString("gausssize=%1").arg(m_gausssize);
    parts << QString("peak=%1").arg(m_peak, 0, 'f', 2);
    parts << QString("maxgain=%1").arg(m_maxgain, 0, 'f', 1);
    parts << QString("targetrms=%1").arg(m_targetrms, 0, 'f', 3);
    parts << QString("coupling=%1").arg(m_coupling ? 1 : 0);
    parts << QString("correctdc=%1").arg(m_correctdc ? 1 : 0);
    parts << QString("altboundary=%1").arg(m_altboundary ? 1 : 0);
    parts << QString("compress=%1").arg(m_compress, 0, 'f', 2);
    parts << QString("threshold=%1").arg(m_threshold, 0, 'f', 3);
    parts << QString("channels=%1").arg(m_channels);
    parts << QString("overlap=%1").arg(m_overlap, 0, 'f', 2);
    
    if (!m_curve.isEmpty()) {
        parts << QString("curve=%1").arg(m_curve);
    }
    
    ffmpegFlags = "dynaudnorm=" + parts.join(":");
}

QString FFDynaudnorm::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFDynaudnorm::toJSON(QJsonObject& json) const {
    json["type"] = "ff-dynaudnorm";
    json["framelen"] = m_framelen;
    json["gausssize"] = m_gausssize;
    json["peak"] = m_peak;
    json["maxgain"] = m_maxgain;
    json["targetrms"] = m_targetrms;
    json["coupling"] = m_coupling;
    json["correctdc"] = m_correctdc;
    json["altboundary"] = m_altboundary;
    json["compress"] = m_compress;
    json["threshold"] = m_threshold;
    json["channels"] = m_channels;
    json["overlap"] = m_overlap;
    json["curve"] = m_curve;
}

void FFDynaudnorm::fromJSON(const QJsonObject& json) {
    m_framelen = json["framelen"].toInt(500);
    m_gausssize = json["gausssize"].toInt(31);
    m_peak = json["peak"].toDouble(0.95);
    m_maxgain = json["maxgain"].toDouble(10.0);
    m_targetrms = json["targetrms"].toDouble(0.0);
    m_coupling = json["coupling"].toBool(true);
    m_correctdc = json["correctdc"].toBool(false);
    m_altboundary = json["altboundary"].toBool(false);
    m_compress = json["compress"].toDouble(0.0);
    m_threshold = json["threshold"].toDouble(0.0);
    m_channels = json["channels"].toString("all");
    m_overlap = json["overlap"].toDouble(0.0);
    m_curve = json["curve"].toString("");
    updateFFmpegFlags();
}
