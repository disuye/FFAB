#include "ff-asubboost.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFAsubboost::FFAsubboost() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAsubboost::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: BOOST ==========
    auto boostTab = new QWidget();
    auto boostLayout = new QVBoxLayout(boostTab);

    // Boost parameter (1 - 12)
    auto boostGroup = new QGroupBox("Boost Amount");
    auto boostGroupLayout = new QVBoxLayout(boostGroup);

    boostSpinBox = new QDoubleSpinBox();
    boostSpinBox->setMinimum(1.0);
    boostSpinBox->setMaximum(12.0);
    boostSpinBox->setValue(m_boost);
    boostSpinBox->setDecimals(1);
    boostSpinBox->setSingleStep(0.1);
    boostSpinBox->setSuffix("x");

    auto boostControlLayout = new QHBoxLayout();
    boostControlLayout->addWidget(boostSpinBox);
    boostControlLayout->addStretch();
    boostControlLayout->addWidget(new QLabel("1x"));

    boostSlider = new QSlider(Qt::Horizontal);
    boostSlider->setMinimum(100);
    boostSlider->setMaximum(1200);
    boostSlider->setValue(static_cast<int>(m_boost * 100));
    boostControlLayout->addWidget(boostSlider, 1);
    boostControlLayout->addWidget(new QLabel("12x"));

    boostGroupLayout->addLayout(boostControlLayout);
    boostLayout->addWidget(boostGroup);

    // Cutoff parameter (50 - 900 Hz)
    auto cutoffGroup = new QGroupBox("Cutoff Frequency (Hz)");
    auto cutoffGroupLayout = new QVBoxLayout(cutoffGroup);

    cutoffSpinBox = new QDoubleSpinBox();
    cutoffSpinBox->setMinimum(50.0);
    cutoffSpinBox->setMaximum(900.0);
    cutoffSpinBox->setValue(m_cutoff);
    cutoffSpinBox->setDecimals(0);
    cutoffSpinBox->setSingleStep(10);
    cutoffSpinBox->setSuffix(" Hz");

    auto cutoffControlLayout = new QHBoxLayout();
    cutoffControlLayout->addWidget(cutoffSpinBox);
    cutoffControlLayout->addStretch();
    cutoffControlLayout->addWidget(new QLabel("50"));

    cutoffSlider = new QSlider(Qt::Horizontal);
    cutoffSlider->setMinimum(50);
    cutoffSlider->setMaximum(900);
    cutoffSlider->setValue(static_cast<int>(m_cutoff));
    cutoffControlLayout->addWidget(cutoffSlider, 1);
    cutoffControlLayout->addWidget(new QLabel("900"));

    cutoffGroupLayout->addLayout(cutoffControlLayout);
    boostLayout->addWidget(cutoffGroup);

    // Slope parameter (0.0001 - 1)
    auto slopeGroup = new QGroupBox("Filter Slope");
    auto slopeGroupLayout = new QVBoxLayout(slopeGroup);

    slopeSpinBox = new QDoubleSpinBox();
    slopeSpinBox->setMinimum(0.0001);
    slopeSpinBox->setMaximum(1.0);
    slopeSpinBox->setValue(m_slope);
    slopeSpinBox->setDecimals(4);
    slopeSpinBox->setSingleStep(0.01);

    auto slopeControlLayout = new QHBoxLayout();
    slopeControlLayout->addWidget(slopeSpinBox);
    slopeControlLayout->addStretch();
    slopeControlLayout->addWidget(new QLabel("0"));

    slopeSlider = new QSlider(Qt::Horizontal);
    slopeSlider->setMinimum(1);
    slopeSlider->setMaximum(10000);
    slopeSlider->setValue(static_cast<int>(m_slope * 10000));
    slopeControlLayout->addWidget(slopeSlider, 1);
    slopeControlLayout->addWidget(new QLabel("1"));

    slopeGroupLayout->addLayout(slopeControlLayout);
    boostLayout->addWidget(slopeGroup);

    boostLayout->addStretch();
    tabWidget->addTab(boostTab, "Boost");

    // ========== TAB 2: MIX ==========
    auto mixTab = new QWidget();
    auto mixLayout = new QVBoxLayout(mixTab);

    // Dry parameter (0 - 1)
    auto dryGroup = new QGroupBox("Dry Level");
    auto dryGroupLayout = new QVBoxLayout(dryGroup);

    drySpinBox = new QDoubleSpinBox();
    drySpinBox->setMinimum(0.0);
    drySpinBox->setMaximum(100.0);
    drySpinBox->setValue(m_dry * 100);
    drySpinBox->setDecimals(0);
    drySpinBox->setSingleStep(1);
    drySpinBox->setSuffix("%");

    auto dryControlLayout = new QHBoxLayout();
    dryControlLayout->addWidget(drySpinBox);
    dryControlLayout->addStretch();
    dryControlLayout->addWidget(new QLabel("0%"));

    drySlider = new QSlider(Qt::Horizontal);
    drySlider->setMinimum(0);
    drySlider->setMaximum(100);
    drySlider->setValue(static_cast<int>(m_dry * 100));
    dryControlLayout->addWidget(drySlider, 1);
    dryControlLayout->addWidget(new QLabel("100%"));

    dryGroupLayout->addLayout(dryControlLayout);
    mixLayout->addWidget(dryGroup);

    // Wet parameter (0 - 1)
    auto wetGroup = new QGroupBox("Wet Level");
    auto wetGroupLayout = new QVBoxLayout(wetGroup);

    wetSpinBox = new QDoubleSpinBox();
    wetSpinBox->setMinimum(0.0);
    wetSpinBox->setMaximum(100.0);
    wetSpinBox->setValue(m_wet * 100);
    wetSpinBox->setDecimals(0);
    wetSpinBox->setSingleStep(1);
    wetSpinBox->setSuffix("%");

    auto wetControlLayout = new QHBoxLayout();
    wetControlLayout->addWidget(wetSpinBox);
    wetControlLayout->addStretch();
    wetControlLayout->addWidget(new QLabel("0%"));

    wetSlider = new QSlider(Qt::Horizontal);
    wetSlider->setMinimum(0);
    wetSlider->setMaximum(100);
    wetSlider->setValue(static_cast<int>(m_wet * 100));
    wetControlLayout->addWidget(wetSlider, 1);
    wetControlLayout->addWidget(new QLabel("100%"));

    wetGroupLayout->addLayout(wetControlLayout);
    mixLayout->addWidget(wetGroup);

    mixLayout->addStretch();
    tabWidget->addTab(mixTab, "Mix");

    // ========== TAB 3: FEEDBACK ==========
    auto feedbackTab = new QWidget();
    auto feedbackLayout = new QVBoxLayout(feedbackTab);

    // Feedback parameter (0 - 1)
    auto feedbackGroup = new QGroupBox("Feedback");
    auto feedbackGroupLayout = new QVBoxLayout(feedbackGroup);

    feedbackSpinBox = new QDoubleSpinBox();
    feedbackSpinBox->setMinimum(0.0);
    feedbackSpinBox->setMaximum(100.0);
    feedbackSpinBox->setValue(m_feedback * 100);
    feedbackSpinBox->setDecimals(0);
    feedbackSpinBox->setSingleStep(1);
    feedbackSpinBox->setSuffix("%");

    auto feedbackControlLayout = new QHBoxLayout();
    feedbackControlLayout->addWidget(feedbackSpinBox);
    feedbackControlLayout->addStretch();
    feedbackControlLayout->addWidget(new QLabel("0%"));

    feedbackSlider = new QSlider(Qt::Horizontal);
    feedbackSlider->setMinimum(0);
    feedbackSlider->setMaximum(100);
    feedbackSlider->setValue(static_cast<int>(m_feedback * 100));
    feedbackControlLayout->addWidget(feedbackSlider, 1);
    feedbackControlLayout->addWidget(new QLabel("100%"));

    feedbackGroupLayout->addLayout(feedbackControlLayout);
    feedbackLayout->addWidget(feedbackGroup);

    // Decay parameter (0 - 1)
    auto decayGroup = new QGroupBox("Decay");
    auto decayGroupLayout = new QVBoxLayout(decayGroup);

    decaySpinBox = new QDoubleSpinBox();
    decaySpinBox->setMinimum(0.0);
    decaySpinBox->setMaximum(100.0);
    decaySpinBox->setValue(m_decay * 100);
    decaySpinBox->setDecimals(0);
    decaySpinBox->setSingleStep(1);
    decaySpinBox->setSuffix("%");

    auto decayControlLayout = new QHBoxLayout();
    decayControlLayout->addWidget(decaySpinBox);
    decayControlLayout->addStretch();
    decayControlLayout->addWidget(new QLabel("0%"));

    decaySlider = new QSlider(Qt::Horizontal);
    decaySlider->setMinimum(0);
    decaySlider->setMaximum(100);
    decaySlider->setValue(static_cast<int>(m_decay * 100));
    decayControlLayout->addWidget(decaySlider, 1);
    decayControlLayout->addWidget(new QLabel("100%"));

    decayGroupLayout->addLayout(decayControlLayout);
    feedbackLayout->addWidget(decayGroup);

    // Delay parameter (1 - 100 ms)
    auto delayGroup = new QGroupBox("Delay (ms)");
    auto delayGroupLayout = new QVBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(1.0);
    delaySpinBox->setMaximum(100.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(1);
    delaySpinBox->setSingleStep(1);
    delaySpinBox->setSuffix(" ms");

    auto delayControlLayout = new QHBoxLayout();
    delayControlLayout->addWidget(delaySpinBox);
    delayControlLayout->addStretch();
    delayControlLayout->addWidget(new QLabel("1"));

    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(10);
    delaySlider->setMaximum(1000);
    delaySlider->setValue(static_cast<int>(m_delay * 10));
    delayControlLayout->addWidget(delaySlider, 1);
    delayControlLayout->addWidget(new QLabel("100"));

    delayGroupLayout->addLayout(delayControlLayout);
    feedbackLayout->addWidget(delayGroup);

    feedbackLayout->addStretch();
    tabWidget->addTab(feedbackTab, "Feedback");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Boost
    connect(boostSlider, &QSlider::valueChanged, [this](int value) {
        m_boost = value / 100.0;
        boostSpinBox->blockSignals(true);
        boostSpinBox->setValue(m_boost);
        boostSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(boostSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_boost = value;
        boostSlider->blockSignals(true);
        boostSlider->setValue(static_cast<int>(value * 100));
        boostSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Cutoff
    connect(cutoffSlider, &QSlider::valueChanged, [this](int value) {
        m_cutoff = value;
        cutoffSpinBox->blockSignals(true);
        cutoffSpinBox->setValue(m_cutoff);
        cutoffSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(cutoffSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_cutoff = value;
        cutoffSlider->blockSignals(true);
        cutoffSlider->setValue(static_cast<int>(value));
        cutoffSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Slope
    connect(slopeSlider, &QSlider::valueChanged, [this](int value) {
        m_slope = value / 10000.0;
        slopeSpinBox->blockSignals(true);
        slopeSpinBox->setValue(m_slope);
        slopeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(slopeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_slope = value;
        slopeSlider->blockSignals(true);
        slopeSlider->setValue(static_cast<int>(value * 10000));
        slopeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Dry
    connect(drySlider, &QSlider::valueChanged, [this](int value) {
        m_dry = value / 100.0;
        drySpinBox->blockSignals(true);
        drySpinBox->setValue(value);
        drySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(drySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_dry = value / 100.0;
        drySlider->blockSignals(true);
        drySlider->setValue(static_cast<int>(value));
        drySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Wet
    connect(wetSlider, &QSlider::valueChanged, [this](int value) {
        m_wet = value / 100.0;
        wetSpinBox->blockSignals(true);
        wetSpinBox->setValue(value);
        wetSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(wetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_wet = value / 100.0;
        wetSlider->blockSignals(true);
        wetSlider->setValue(static_cast<int>(value));
        wetSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Feedback
    connect(feedbackSlider, &QSlider::valueChanged, [this](int value) {
        m_feedback = value / 100.0;
        feedbackSpinBox->blockSignals(true);
        feedbackSpinBox->setValue(value);
        feedbackSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(feedbackSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_feedback = value / 100.0;
        feedbackSlider->blockSignals(true);
        feedbackSlider->setValue(static_cast<int>(value));
        feedbackSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Decay
    connect(decaySlider, &QSlider::valueChanged, [this](int value) {
        m_decay = value / 100.0;
        decaySpinBox->blockSignals(true);
        decaySpinBox->setValue(value);
        decaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(decaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_decay = value / 100.0;
        decaySlider->blockSignals(true);
        decaySlider->setValue(static_cast<int>(value));
        decaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Delay
    connect(delaySlider, &QSlider::valueChanged, [this](int value) {
        m_delay = value / 10.0;
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_delay = value;
        delaySlider->blockSignals(true);
        delaySlider->setValue(static_cast<int>(value * 10));
        delaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAsubboost::updateFFmpegFlags() {
    ffmpegFlags = QString("asubboost=dry=%1:wet=%2:boost=%3:decay=%4:feedback=%5:cutoff=%6:slope=%7:delay=%8")
                      .arg(m_dry, 0, 'f', 2)
                      .arg(m_wet, 0, 'f', 2)
                      .arg(m_boost, 0, 'f', 2)
                      .arg(m_decay, 0, 'f', 2)
                      .arg(m_feedback, 0, 'f', 2)
                      .arg(m_cutoff, 0, 'f', 0)
                      .arg(m_slope, 0, 'f', 4)
                      .arg(m_delay, 0, 'f', 1);
}

QString FFAsubboost::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAsubboost::toJSON(QJsonObject& json) const {
    json["type"] = "ff-asubboost";
    json["dry"] = m_dry;
    json["wet"] = m_wet;
    json["boost"] = m_boost;
    json["decay"] = m_decay;
    json["feedback"] = m_feedback;
    json["cutoff"] = m_cutoff;
    json["slope"] = m_slope;
    json["delay"] = m_delay;
}

void FFAsubboost::fromJSON(const QJsonObject& json) {
    m_dry = json["dry"].toDouble(1.0);
    m_wet = json["wet"].toDouble(1.0);
    m_boost = json["boost"].toDouble(2.0);
    m_decay = json["decay"].toDouble(0.0);
    m_feedback = json["feedback"].toDouble(0.9);
    m_cutoff = json["cutoff"].toDouble(100.0);
    m_slope = json["slope"].toDouble(0.5);
    m_delay = json["delay"].toDouble(20.0);
    updateFFmpegFlags();
}
