#include "ff-adeclick.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFAdeclick::FFAdeclick() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdeclick::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: DETECTION ==========
    auto detectionTab = new QWidget();
    auto detectionLayout = new QVBoxLayout(detectionTab);

    // Window size parameter (10 - 100 ms)
    auto windowGroup = new QGroupBox("Window Size (ms)");
    auto windowLayout = new QVBoxLayout(windowGroup);

    windowSpinBox = new QDoubleSpinBox();
    windowSpinBox->setMinimum(10.0);
    windowSpinBox->setMaximum(100.0);
    windowSpinBox->setValue(m_window);
    windowSpinBox->setDecimals(1);
    windowSpinBox->setSingleStep(1.0);
    windowSpinBox->setSuffix(" ms");

    auto windowControlLayout = new QHBoxLayout();
    windowControlLayout->addWidget(windowSpinBox);
    windowControlLayout->addStretch();
    
    auto windowMinLabel = new QLabel("10");
    windowControlLayout->addWidget(windowMinLabel);

    windowSlider = new QSlider(Qt::Horizontal);
    windowSlider->setMinimum(100);
    windowSlider->setMaximum(1000);
    windowSlider->setValue(static_cast<int>(m_window * 10));
    windowControlLayout->addWidget(windowSlider, 1);

    auto windowMaxLabel = new QLabel("100");
    windowControlLayout->addWidget(windowMaxLabel);

    windowLayout->addLayout(windowControlLayout);
    detectionLayout->addWidget(windowGroup);

    // Overlap parameter (50 - 95 %)
    auto overlapGroup = new QGroupBox("Window Overlap (%)");
    auto overlapLayout = new QVBoxLayout(overlapGroup);

    overlapSpinBox = new QDoubleSpinBox();
    overlapSpinBox->setMinimum(50.0);
    overlapSpinBox->setMaximum(95.0);
    overlapSpinBox->setValue(m_overlap);
    overlapSpinBox->setDecimals(1);
    overlapSpinBox->setSingleStep(1.0);
    overlapSpinBox->setSuffix(" %");

    auto overlapControlLayout = new QHBoxLayout();
    overlapControlLayout->addWidget(overlapSpinBox);
    overlapControlLayout->addStretch();
    
    auto overlapMinLabel = new QLabel("50%");
    overlapControlLayout->addWidget(overlapMinLabel);

    overlapSlider = new QSlider(Qt::Horizontal);
    overlapSlider->setMinimum(500);
    overlapSlider->setMaximum(950);
    overlapSlider->setValue(static_cast<int>(m_overlap * 10));
    overlapControlLayout->addWidget(overlapSlider, 1);

    auto overlapMaxLabel = new QLabel("95%");
    overlapControlLayout->addWidget(overlapMaxLabel);

    overlapLayout->addLayout(overlapControlLayout);
    detectionLayout->addWidget(overlapGroup);

    // Threshold parameter (1 - 100)
    auto thresholdGroup = new QGroupBox("Threshold");
    auto thresholdLayout = new QVBoxLayout(thresholdGroup);

    thresholdSpinBox = new QDoubleSpinBox();
    thresholdSpinBox->setMinimum(1.0);
    thresholdSpinBox->setMaximum(100.0);
    thresholdSpinBox->setValue(m_threshold);
    thresholdSpinBox->setDecimals(1);
    thresholdSpinBox->setSingleStep(0.5);

    auto thresholdControlLayout = new QHBoxLayout();
    thresholdControlLayout->addWidget(thresholdSpinBox);
    thresholdControlLayout->addStretch();
    
    auto thresholdMinLabel = new QLabel("1");
    thresholdControlLayout->addWidget(thresholdMinLabel);

    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setMinimum(10);
    thresholdSlider->setMaximum(1000);
    thresholdSlider->setValue(static_cast<int>(m_threshold * 10));
    thresholdControlLayout->addWidget(thresholdSlider, 1);

    auto thresholdMaxLabel = new QLabel("100");
    thresholdControlLayout->addWidget(thresholdMaxLabel);

    auto thresholdHint = new QLabel("Lower = more sensitive detection");
    thresholdHint->setStyleSheet("color: gray; font-size: 11px;");
    thresholdLayout->addLayout(thresholdControlLayout);
    thresholdLayout->addWidget(thresholdHint);
    detectionLayout->addWidget(thresholdGroup);

    detectionLayout->addStretch();
    tabWidget->addTab(detectionTab, "Detection");

    // ========== TAB 2: ADVANCED ==========
    auto advancedTab = new QWidget();
    auto advancedLayout = new QVBoxLayout(advancedTab);

    // AR Order parameter (0 - 25)
    auto arorderGroup = new QGroupBox("Autoregression Order");
    auto arorderLayout = new QVBoxLayout(arorderGroup);

    arorderSpinBox = new QDoubleSpinBox();
    arorderSpinBox->setMinimum(0.0);
    arorderSpinBox->setMaximum(25.0);
    arorderSpinBox->setValue(m_arorder);
    arorderSpinBox->setDecimals(1);
    arorderSpinBox->setSingleStep(0.5);

    auto arorderControlLayout = new QHBoxLayout();
    arorderControlLayout->addWidget(arorderSpinBox);
    arorderControlLayout->addStretch();
    
    auto arorderMinLabel = new QLabel("0");
    arorderControlLayout->addWidget(arorderMinLabel);

    arorderSlider = new QSlider(Qt::Horizontal);
    arorderSlider->setMinimum(0);
    arorderSlider->setMaximum(250);
    arorderSlider->setValue(static_cast<int>(m_arorder * 10));
    arorderControlLayout->addWidget(arorderSlider, 1);

    auto arorderMaxLabel = new QLabel("25");
    arorderControlLayout->addWidget(arorderMaxLabel);

    auto arorderHint = new QLabel("Prediction model complexity");
    arorderHint->setStyleSheet("color: gray; font-size: 11px;");
    arorderLayout->addLayout(arorderControlLayout);
    arorderLayout->addWidget(arorderHint);
    advancedLayout->addWidget(arorderGroup);

    // Burst fusion parameter (0 - 10)
    auto burstGroup = new QGroupBox("Burst Fusion");
    auto burstLayout = new QVBoxLayout(burstGroup);

    burstSpinBox = new QDoubleSpinBox();
    burstSpinBox->setMinimum(0.0);
    burstSpinBox->setMaximum(10.0);
    burstSpinBox->setValue(m_burst);
    burstSpinBox->setDecimals(1);
    burstSpinBox->setSingleStep(0.5);

    auto burstControlLayout = new QHBoxLayout();
    burstControlLayout->addWidget(burstSpinBox);
    burstControlLayout->addStretch();
    
    auto burstMinLabel = new QLabel("0");
    burstControlLayout->addWidget(burstMinLabel);

    burstSlider = new QSlider(Qt::Horizontal);
    burstSlider->setMinimum(0);
    burstSlider->setMaximum(100);
    burstSlider->setValue(static_cast<int>(m_burst * 10));
    burstControlLayout->addWidget(burstSlider, 1);

    auto burstMaxLabel = new QLabel("10");
    burstControlLayout->addWidget(burstMaxLabel);

    auto burstHint = new QLabel("Merge adjacent clicks into bursts");
    burstHint->setStyleSheet("color: gray; font-size: 11px;");
    burstLayout->addLayout(burstControlLayout);
    burstLayout->addWidget(burstHint);
    advancedLayout->addWidget(burstGroup);

    // Method combo
    auto methodGroup = new QGroupBox("Overlap Method");
    auto methodLayout = new QVBoxLayout(methodGroup);

    methodCombo = new QComboBox();
    methodCombo->addItem("Add", 0);
    methodCombo->addItem("Save", 1);
    methodCombo->setCurrentIndex(m_method);

    methodLayout->addWidget(methodCombo);
    advancedLayout->addWidget(methodGroup);

    advancedLayout->addStretch();
    tabWidget->addTab(advancedTab, "Advanced");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Window
    connect(windowSlider, &QSlider::valueChanged, [this](int value) {
        m_window = value / 10.0;
        windowSpinBox->blockSignals(true);
        windowSpinBox->setValue(m_window);
        windowSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(windowSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_window = value;
        windowSlider->blockSignals(true);
        windowSlider->setValue(static_cast<int>(value * 10));
        windowSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Overlap
    connect(overlapSlider, &QSlider::valueChanged, [this](int value) {
        m_overlap = value / 10.0;
        overlapSpinBox->blockSignals(true);
        overlapSpinBox->setValue(m_overlap);
        overlapSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(overlapSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_overlap = value;
        overlapSlider->blockSignals(true);
        overlapSlider->setValue(static_cast<int>(value * 10));
        overlapSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Threshold
    connect(thresholdSlider, &QSlider::valueChanged, [this](int value) {
        m_threshold = value / 10.0;
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(m_threshold);
        thresholdSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_threshold = value;
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>(value * 10));
        thresholdSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // AR Order
    connect(arorderSlider, &QSlider::valueChanged, [this](int value) {
        m_arorder = value / 10.0;
        arorderSpinBox->blockSignals(true);
        arorderSpinBox->setValue(m_arorder);
        arorderSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(arorderSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_arorder = value;
        arorderSlider->blockSignals(true);
        arorderSlider->setValue(static_cast<int>(value * 10));
        arorderSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Burst
    connect(burstSlider, &QSlider::valueChanged, [this](int value) {
        m_burst = value / 10.0;
        burstSpinBox->blockSignals(true);
        burstSpinBox->setValue(m_burst);
        burstSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(burstSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_burst = value;
        burstSlider->blockSignals(true);
        burstSlider->setValue(static_cast<int>(value * 10));
        burstSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Method combo
    connect(methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_method = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdeclick::updateFFmpegFlags() {
    QStringList methodNames = {"add", "save"};
    QString methodName = (m_method >= 0 && m_method < methodNames.size()) ? methodNames[m_method] : "add";
    
    ffmpegFlags = QString("adeclick=window=%1:overlap=%2:arorder=%3:threshold=%4:burst=%5:method=%6")
                      .arg(m_window, 0, 'f', 1)
                      .arg(m_overlap, 0, 'f', 1)
                      .arg(m_arorder, 0, 'f', 1)
                      .arg(m_threshold, 0, 'f', 1)
                      .arg(m_burst, 0, 'f', 1)
                      .arg(methodName);
}

QString FFAdeclick::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdeclick::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adeclick";
    json["window"] = m_window;
    json["overlap"] = m_overlap;
    json["arorder"] = m_arorder;
    json["threshold"] = m_threshold;
    json["burst"] = m_burst;
    json["method"] = m_method;
}

void FFAdeclick::fromJSON(const QJsonObject& json) {
    m_window = json["window"].toDouble(55.0);
    m_overlap = json["overlap"].toDouble(75.0);
    m_arorder = json["arorder"].toDouble(2.0);
    m_threshold = json["threshold"].toDouble(2.0);
    m_burst = json["burst"].toDouble(2.0);
    m_method = json["method"].toInt(0);
    updateFFmpegFlags();
}
