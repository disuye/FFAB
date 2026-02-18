#include "ff-adeclip.h"
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

FFAdeclip::FFAdeclip() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdeclip::getParametersWidget() {
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

    auto thresholdHint = new QLabel("Clipping detection sensitivity");
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

    auto arorderHint = new QLabel("Higher = more accurate reconstruction");
    arorderHint->setStyleSheet("color: gray; font-size: 11px;");
    arorderLayout->addLayout(arorderControlLayout);
    arorderLayout->addWidget(arorderHint);
    advancedLayout->addWidget(arorderGroup);

    // Histogram size parameter (100 - 9999)
    auto hsizeGroup = new QGroupBox("Histogram Size");
    auto hsizeLayout = new QVBoxLayout(hsizeGroup);

    hsizeSpinBox = new QSpinBox();
    hsizeSpinBox->setMinimum(100);
    hsizeSpinBox->setMaximum(9999);
    hsizeSpinBox->setValue(m_hsize);
    hsizeSpinBox->setSingleStep(100);

    auto hsizeControlLayout = new QHBoxLayout();
    hsizeControlLayout->addWidget(hsizeSpinBox);
    hsizeControlLayout->addStretch();
    
    auto hsizeMinLabel = new QLabel("100");
    hsizeControlLayout->addWidget(hsizeMinLabel);

    hsizeSlider = new QSlider(Qt::Horizontal);
    hsizeSlider->setMinimum(100);
    hsizeSlider->setMaximum(9999);
    hsizeSlider->setValue(m_hsize);
    hsizeControlLayout->addWidget(hsizeSlider, 1);

    auto hsizeMaxLabel = new QLabel("9999");
    hsizeControlLayout->addWidget(hsizeMaxLabel);

    auto hsizeHint = new QLabel("Clipping level histogram resolution");
    hsizeHint->setStyleSheet("color: gray; font-size: 11px;");
    hsizeLayout->addLayout(hsizeControlLayout);
    hsizeLayout->addWidget(hsizeHint);
    advancedLayout->addWidget(hsizeGroup);

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

    // Histogram size
    connect(hsizeSlider, &QSlider::valueChanged, [this](int value) {
        m_hsize = value;
        hsizeSpinBox->blockSignals(true);
        hsizeSpinBox->setValue(m_hsize);
        hsizeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(hsizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_hsize = value;
        hsizeSlider->blockSignals(true);
        hsizeSlider->setValue(value);
        hsizeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Method combo
    connect(methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_method = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdeclip::updateFFmpegFlags() {
    QStringList methodNames = {"add", "save"};
    QString methodName = (m_method >= 0 && m_method < methodNames.size()) ? methodNames[m_method] : "add";
    
    ffmpegFlags = QString("adeclip=window=%1:overlap=%2:arorder=%3:threshold=%4:hsize=%5:method=%6")
                      .arg(m_window, 0, 'f', 1)
                      .arg(m_overlap, 0, 'f', 1)
                      .arg(m_arorder, 0, 'f', 1)
                      .arg(m_threshold, 0, 'f', 1)
                      .arg(m_hsize)
                      .arg(methodName);
}

QString FFAdeclip::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdeclip::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adeclip";
    json["window"] = m_window;
    json["overlap"] = m_overlap;
    json["arorder"] = m_arorder;
    json["threshold"] = m_threshold;
    json["hsize"] = m_hsize;
    json["method"] = m_method;
}

void FFAdeclip::fromJSON(const QJsonObject& json) {
    m_window = json["window"].toDouble(55.0);
    m_overlap = json["overlap"].toDouble(75.0);
    m_arorder = json["arorder"].toDouble(8.0);
    m_threshold = json["threshold"].toDouble(10.0);
    m_hsize = json["hsize"].toInt(1000);
    m_method = json["method"].toInt(0);
    updateFFmpegFlags();
}
