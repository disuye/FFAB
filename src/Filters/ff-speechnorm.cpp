#include "ff-speechnorm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFSpeechnorm::FFSpeechnorm() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFSpeechnorm::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: LEVELS ==========
    auto levelsTab = new QWidget();
    auto levelsLayout = new QVBoxLayout(levelsTab);

    // Peak level (0 - 100%)
    auto peakGroup = new QGroupBox("Target Peak Level");
    auto peakLayout = new QVBoxLayout(peakGroup);

    peakSpinBox = new QDoubleSpinBox();
    peakSpinBox->setMinimum(0.0);
    peakSpinBox->setMaximum(100.0);
    peakSpinBox->setValue(m_peak * 100.0);
    peakSpinBox->setDecimals(1);
    peakSpinBox->setSingleStep(1.0);
    peakSpinBox->setSuffix(" %");

    auto peakControlLayout = new QHBoxLayout();
    peakControlLayout->addWidget(peakSpinBox);
    peakControlLayout->addStretch();
    peakControlLayout->addWidget(new QLabel("0%"));
    peakSlider = new QSlider(Qt::Horizontal);
    peakSlider->setMinimum(0);
    peakSlider->setMaximum(100);
    peakSlider->setValue(static_cast<int>(m_peak * 100));
    peakControlLayout->addWidget(peakSlider, 1);
    peakControlLayout->addWidget(new QLabel("100%"));

    auto peakHint = new QLabel("Target maximum amplitude (95% recommended)");
    peakHint->setStyleSheet("color: gray; font-size: 11px;");
    peakLayout->addLayout(peakControlLayout);
    peakLayout->addWidget(peakHint);
    levelsLayout->addWidget(peakGroup);

    // Threshold (0 - 100%)
    auto thresholdGroup = new QGroupBox("Silence Threshold");
    auto thresholdLayout = new QVBoxLayout(thresholdGroup);

    thresholdSpinBox = new QDoubleSpinBox();
    thresholdSpinBox->setMinimum(0.0);
    thresholdSpinBox->setMaximum(100.0);
    thresholdSpinBox->setValue(m_threshold * 100.0);
    thresholdSpinBox->setDecimals(1);
    thresholdSpinBox->setSingleStep(0.5);
    thresholdSpinBox->setSuffix(" %");

    auto thresholdControlLayout = new QHBoxLayout();
    thresholdControlLayout->addWidget(thresholdSpinBox);
    thresholdControlLayout->addStretch();
    thresholdControlLayout->addWidget(new QLabel("0%"));
    thresholdSlider = new QSlider(Qt::Horizontal);
    thresholdSlider->setMinimum(0);
    thresholdSlider->setMaximum(100);
    thresholdSlider->setValue(static_cast<int>(m_threshold * 100));
    thresholdControlLayout->addWidget(thresholdSlider, 1);
    thresholdControlLayout->addWidget(new QLabel("100%"));

    auto thresholdHint = new QLabel("Below this level is treated as silence (no processing)");
    thresholdHint->setStyleSheet("color: gray; font-size: 11px;");
    thresholdLayout->addLayout(thresholdControlLayout);
    thresholdLayout->addWidget(thresholdHint);
    levelsLayout->addWidget(thresholdGroup);

    levelsLayout->addStretch();
    tabWidget->addTab(levelsTab, "Levels");

    // ========== TAB 2: DYNAMICS ==========
    auto dynamicsTab = new QWidget();
    auto dynamicsLayout = new QVBoxLayout(dynamicsTab);

    // Expansion (1 - 50x)
    auto expansionGroup = new QGroupBox("Max Expansion");
    auto expansionLayout = new QVBoxLayout(expansionGroup);

    expansionSpinBox = new QDoubleSpinBox();
    expansionSpinBox->setMinimum(1.0);
    expansionSpinBox->setMaximum(50.0);
    expansionSpinBox->setValue(m_expansion);
    expansionSpinBox->setDecimals(1);
    expansionSpinBox->setSingleStep(0.5);
    expansionSpinBox->setSuffix("x");

    auto expansionControlLayout = new QHBoxLayout();
    expansionControlLayout->addWidget(expansionSpinBox);
    expansionControlLayout->addStretch();
    expansionControlLayout->addWidget(new QLabel("1x"));
    expansionSlider = new QSlider(Qt::Horizontal);
    expansionSlider->setMinimum(10);
    expansionSlider->setMaximum(500);
    expansionSlider->setValue(static_cast<int>(m_expansion * 10));
    expansionControlLayout->addWidget(expansionSlider, 1);
    expansionControlLayout->addWidget(new QLabel("50x"));

    auto expansionHint = new QLabel("Maximum gain increase for quiet passages");
    expansionHint->setStyleSheet("color: gray; font-size: 11px;");
    expansionLayout->addLayout(expansionControlLayout);
    expansionLayout->addWidget(expansionHint);
    dynamicsLayout->addWidget(expansionGroup);

    // Compression (1 - 50x)
    auto compressionGroup = new QGroupBox("Max Compression");
    auto compressionLayout = new QVBoxLayout(compressionGroup);

    compressionSpinBox = new QDoubleSpinBox();
    compressionSpinBox->setMinimum(1.0);
    compressionSpinBox->setMaximum(50.0);
    compressionSpinBox->setValue(m_compression);
    compressionSpinBox->setDecimals(1);
    compressionSpinBox->setSingleStep(0.5);
    compressionSpinBox->setSuffix("x");

    auto compressionControlLayout = new QHBoxLayout();
    compressionControlLayout->addWidget(compressionSpinBox);
    compressionControlLayout->addStretch();
    compressionControlLayout->addWidget(new QLabel("1x"));
    compressionSlider = new QSlider(Qt::Horizontal);
    compressionSlider->setMinimum(10);
    compressionSlider->setMaximum(500);
    compressionSlider->setValue(static_cast<int>(m_compression * 10));
    compressionControlLayout->addWidget(compressionSlider, 1);
    compressionControlLayout->addWidget(new QLabel("50x"));

    auto compressionHint = new QLabel("Maximum gain reduction for loud passages");
    compressionHint->setStyleSheet("color: gray; font-size: 11px;");
    compressionLayout->addLayout(compressionControlLayout);
    compressionLayout->addWidget(compressionHint);
    dynamicsLayout->addWidget(compressionGroup);

    dynamicsLayout->addStretch();
    tabWidget->addTab(dynamicsTab, "Dynamics");

    // ========== TAB 3: TIMING ==========
    auto timingTab = new QWidget();
    auto timingLayout = new QVBoxLayout(timingTab);

    // Raise rate (0 - 100%)
    auto raiseGroup = new QGroupBox("Expansion Rate");
    auto raiseLayout = new QVBoxLayout(raiseGroup);

    raiseSpinBox = new QDoubleSpinBox();
    raiseSpinBox->setMinimum(0.0);
    raiseSpinBox->setMaximum(100.0);
    raiseSpinBox->setValue(m_raise * 100.0);
    raiseSpinBox->setDecimals(2);
    raiseSpinBox->setSingleStep(0.1);
    raiseSpinBox->setSuffix(" %");

    auto raiseControlLayout = new QHBoxLayout();
    raiseControlLayout->addWidget(raiseSpinBox);
    raiseControlLayout->addStretch();
    raiseControlLayout->addWidget(new QLabel("0%"));
    raiseSlider = new QSlider(Qt::Horizontal);
    raiseSlider->setMinimum(0);
    raiseSlider->setMaximum(1000);
    raiseSlider->setValue(static_cast<int>(m_raise * 1000));
    raiseControlLayout->addWidget(raiseSlider, 1);
    raiseControlLayout->addWidget(new QLabel("100%"));

    auto raiseHint = new QLabel("How quickly gain increases (lower = smoother)");
    raiseHint->setStyleSheet("color: gray; font-size: 11px;");
    raiseLayout->addLayout(raiseControlLayout);
    raiseLayout->addWidget(raiseHint);
    timingLayout->addWidget(raiseGroup);

    // Fall rate (0 - 100%)
    auto fallGroup = new QGroupBox("Compression Rate");
    auto fallLayout = new QVBoxLayout(fallGroup);

    fallSpinBox = new QDoubleSpinBox();
    fallSpinBox->setMinimum(0.0);
    fallSpinBox->setMaximum(100.0);
    fallSpinBox->setValue(m_fall * 100.0);
    fallSpinBox->setDecimals(2);
    fallSpinBox->setSingleStep(0.1);
    fallSpinBox->setSuffix(" %");

    auto fallControlLayout = new QHBoxLayout();
    fallControlLayout->addWidget(fallSpinBox);
    fallControlLayout->addStretch();
    fallControlLayout->addWidget(new QLabel("0%"));
    fallSlider = new QSlider(Qt::Horizontal);
    fallSlider->setMinimum(0);
    fallSlider->setMaximum(1000);
    fallSlider->setValue(static_cast<int>(m_fall * 1000));
    fallControlLayout->addWidget(fallSlider, 1);
    fallControlLayout->addWidget(new QLabel("100%"));

    auto fallHint = new QLabel("How quickly gain decreases (lower = smoother)");
    fallHint->setStyleSheet("color: gray; font-size: 11px;");
    fallLayout->addLayout(fallControlLayout);
    fallLayout->addWidget(fallHint);
    timingLayout->addWidget(fallGroup);

    // Invert checkbox
    auto invertGroup = new QGroupBox("Mode");
    auto invertLayout = new QVBoxLayout(invertGroup);

    invertCheck = new QCheckBox("Invert filtering");
    invertCheck->setChecked(m_invert);
    auto invertHint = new QLabel("Invert the gain changes (for creative effects)");
    invertHint->setStyleSheet("color: gray; font-size: 11px;");
    invertLayout->addWidget(invertCheck);
    invertLayout->addWidget(invertHint);
    timingLayout->addWidget(invertGroup);

    timingLayout->addStretch();
    tabWidget->addTab(timingTab, "Timing");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Peak
    connect(peakSlider, &QSlider::valueChanged, [this](int value) {
        m_peak = value / 100.0;
        peakSpinBox->blockSignals(true);
        peakSpinBox->setValue(value);
        peakSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(peakSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_peak = value / 100.0;
        peakSlider->blockSignals(true);
        peakSlider->setValue(static_cast<int>(value));
        peakSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Threshold
    connect(thresholdSlider, &QSlider::valueChanged, [this](int value) {
        m_threshold = value / 100.0;
        thresholdSpinBox->blockSignals(true);
        thresholdSpinBox->setValue(value);
        thresholdSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(thresholdSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_threshold = value / 100.0;
        thresholdSlider->blockSignals(true);
        thresholdSlider->setValue(static_cast<int>(value));
        thresholdSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Expansion
    connect(expansionSlider, &QSlider::valueChanged, [this](int value) {
        m_expansion = value / 10.0;
        expansionSpinBox->blockSignals(true);
        expansionSpinBox->setValue(m_expansion);
        expansionSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(expansionSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_expansion = value;
        expansionSlider->blockSignals(true);
        expansionSlider->setValue(static_cast<int>(value * 10));
        expansionSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Compression
    connect(compressionSlider, &QSlider::valueChanged, [this](int value) {
        m_compression = value / 10.0;
        compressionSpinBox->blockSignals(true);
        compressionSpinBox->setValue(m_compression);
        compressionSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(compressionSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_compression = value;
        compressionSlider->blockSignals(true);
        compressionSlider->setValue(static_cast<int>(value * 10));
        compressionSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Raise
    connect(raiseSlider, &QSlider::valueChanged, [this](int value) {
        m_raise = value / 1000.0;
        raiseSpinBox->blockSignals(true);
        raiseSpinBox->setValue(m_raise * 100.0);
        raiseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(raiseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_raise = value / 100.0;
        raiseSlider->blockSignals(true);
        raiseSlider->setValue(static_cast<int>(m_raise * 1000));
        raiseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Fall
    connect(fallSlider, &QSlider::valueChanged, [this](int value) {
        m_fall = value / 1000.0;
        fallSpinBox->blockSignals(true);
        fallSpinBox->setValue(m_fall * 100.0);
        fallSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(fallSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_fall = value / 100.0;
        fallSlider->blockSignals(true);
        fallSlider->setValue(static_cast<int>(m_fall * 1000));
        fallSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Invert
    connect(invertCheck, &QCheckBox::toggled, [this](bool checked) {
        m_invert = checked;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFSpeechnorm::updateFFmpegFlags() {
    ffmpegFlags = QString("speechnorm=peak=%1:expansion=%2:compression=%3:threshold=%4:raise=%5:fall=%6:invert=%7")
                      .arg(m_peak, 0, 'f', 3)
                      .arg(m_expansion, 0, 'f', 1)
                      .arg(m_compression, 0, 'f', 1)
                      .arg(m_threshold, 0, 'f', 3)
                      .arg(m_raise, 0, 'f', 4)
                      .arg(m_fall, 0, 'f', 4)
                      .arg(m_invert ? "true" : "false");
}

QString FFSpeechnorm::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFSpeechnorm::toJSON(QJsonObject& json) const {
    json["type"] = "ff-speechnorm";
    json["peak"] = m_peak;
    json["expansion"] = m_expansion;
    json["compression"] = m_compression;
    json["threshold"] = m_threshold;
    json["raise"] = m_raise;
    json["fall"] = m_fall;
    json["invert"] = m_invert;
}

void FFSpeechnorm::fromJSON(const QJsonObject& json) {
    m_peak = json["peak"].toDouble(0.95);
    m_expansion = json["expansion"].toDouble(2.0);
    m_compression = json["compression"].toDouble(2.0);
    m_threshold = json["threshold"].toDouble(0.0);
    m_raise = json["raise"].toDouble(0.001);
    m_fall = json["fall"].toDouble(0.001);
    m_invert = json["invert"].toBool(false);
    updateFFmpegFlags();
}
