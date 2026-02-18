#include "ff-stereowiden.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFStereowiden::FFStereowiden() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFStereowiden::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Delay parameter (ms)
    auto delayGroup = new QGroupBox("Delay Time (ms)");
    auto delayLayout = new QVBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(1.0);
    delaySpinBox->setMaximum(100.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(2);
    delaySpinBox->setSingleStep(1.0);
    delaySpinBox->setSuffix(" ms");

    auto delayControlLayout = new QHBoxLayout();
    delayControlLayout->addWidget(delaySpinBox);
    delayControlLayout->addStretch();
    
    auto delayMinLabel = new QLabel("1 ms");
    delayControlLayout->addWidget(delayMinLabel);

    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(100);
    delaySlider->setMaximum(10000);
    delaySlider->setValue(static_cast<int>(m_delay * 100));
    delayControlLayout->addWidget(delaySlider, 1);

    auto delayMaxLabel = new QLabel("100 ms");
    delayControlLayout->addWidget(delayMaxLabel);

    delayLayout->addLayout(delayControlLayout);
    mainLayout->addWidget(delayGroup);

    // Feedback parameter
    auto feedbackGroup = new QGroupBox("Feedback");
    auto feedbackLayout = new QVBoxLayout(feedbackGroup);

    feedbackSpinBox = new QDoubleSpinBox();
    feedbackSpinBox->setMinimum(0.0);
    feedbackSpinBox->setMaximum(90.0);
    feedbackSpinBox->setValue(m_feedback * 100.0);
    feedbackSpinBox->setDecimals(2);
    feedbackSpinBox->setSingleStep(1.0);
    feedbackSpinBox->setSuffix(" %");

    auto feedbackControlLayout = new QHBoxLayout();
    feedbackControlLayout->addWidget(feedbackSpinBox);
    feedbackControlLayout->addStretch();
    
    auto feedbackMinLabel = new QLabel("0%");
    feedbackControlLayout->addWidget(feedbackMinLabel);

    feedbackSlider = new QSlider(Qt::Horizontal);
    feedbackSlider->setMinimum(0);
    feedbackSlider->setMaximum(90);
    feedbackSlider->setValue(static_cast<int>(m_feedback * 100));
    feedbackControlLayout->addWidget(feedbackSlider, 1);

    auto feedbackMaxLabel = new QLabel("90%");
    feedbackControlLayout->addWidget(feedbackMaxLabel);

    feedbackLayout->addLayout(feedbackControlLayout);
    mainLayout->addWidget(feedbackGroup);

    // Crossfeed parameter
    auto crossfeedGroup = new QGroupBox("Crossfeed");
    auto crossfeedLayout = new QVBoxLayout(crossfeedGroup);

    crossfeedSpinBox = new QDoubleSpinBox();
    crossfeedSpinBox->setMinimum(0.0);
    crossfeedSpinBox->setMaximum(80.0);
    crossfeedSpinBox->setValue(m_crossfeed * 100.0);
    crossfeedSpinBox->setDecimals(2);
    crossfeedSpinBox->setSingleStep(1.0);
    crossfeedSpinBox->setSuffix(" %");

    auto crossfeedControlLayout = new QHBoxLayout();
    crossfeedControlLayout->addWidget(crossfeedSpinBox);
    crossfeedControlLayout->addStretch();
    
    auto crossfeedMinLabel = new QLabel("0%");
    crossfeedControlLayout->addWidget(crossfeedMinLabel);

    crossfeedSlider = new QSlider(Qt::Horizontal);
    crossfeedSlider->setMinimum(0);
    crossfeedSlider->setMaximum(80);
    crossfeedSlider->setValue(static_cast<int>(m_crossfeed * 100));
    crossfeedControlLayout->addWidget(crossfeedSlider, 1);

    auto crossfeedMaxLabel = new QLabel("80%");
    crossfeedControlLayout->addWidget(crossfeedMaxLabel);

    crossfeedLayout->addLayout(crossfeedControlLayout);
    mainLayout->addWidget(crossfeedGroup);

    // Dry Mix parameter
    auto drymixGroup = new QGroupBox("Wet/Dry Mix");
    auto drymixLayout = new QVBoxLayout(drymixGroup);

    drymixSpinBox = new QDoubleSpinBox();
    drymixSpinBox->setMinimum(0.0);
    drymixSpinBox->setMaximum(100.0);
    drymixSpinBox->setValue(m_drymix * 100.0);
    drymixSpinBox->setDecimals(2);
    drymixSpinBox->setSingleStep(1.0);
    drymixSpinBox->setSuffix(" %");

    auto drymixControlLayout = new QHBoxLayout();
    drymixControlLayout->addWidget(drymixSpinBox);
    drymixControlLayout->addStretch();
    
    auto drymixMinLabel = new QLabel("0%");
    drymixControlLayout->addWidget(drymixMinLabel);

    drymixSlider = new QSlider(Qt::Horizontal);
    drymixSlider->setMinimum(0);
    drymixSlider->setMaximum(100);
    drymixSlider->setValue(static_cast<int>(m_drymix * 100));
    drymixControlLayout->addWidget(drymixSlider, 1);

    auto drymixMaxLabel = new QLabel("100%");
    drymixControlLayout->addWidget(drymixMaxLabel);

    drymixLayout->addLayout(drymixControlLayout);
    mainLayout->addWidget(drymixGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Delay
    connect(delaySlider, &QSlider::valueChanged, [this](int value) {
        m_delay = value / 100.0;
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_delay = value;
        delaySlider->blockSignals(true);
        delaySlider->setValue(static_cast<int>(value * 100));
        delaySlider->blockSignals(false);
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

    // Crossfeed
    connect(crossfeedSlider, &QSlider::valueChanged, [this](int value) {
        m_crossfeed = value / 100.0;
        crossfeedSpinBox->blockSignals(true);
        crossfeedSpinBox->setValue(value);
        crossfeedSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(crossfeedSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_crossfeed = value / 100.0;
        crossfeedSlider->blockSignals(true);
        crossfeedSlider->setValue(static_cast<int>(value));
        crossfeedSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Dry Mix
    connect(drymixSlider, &QSlider::valueChanged, [this](int value) {
        m_drymix = value / 100.0;
        drymixSpinBox->blockSignals(true);
        drymixSpinBox->setValue(value);
        drymixSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(drymixSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_drymix = value / 100.0;
        drymixSlider->blockSignals(true);
        drymixSlider->setValue(static_cast<int>(value));
        drymixSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFStereowiden::updateFFmpegFlags() {
    ffmpegFlags = QString("stereowiden=delay=%1:feedback=%2:crossfeed=%3:drymix=%4")
        .arg(m_delay, 0, 'f', 2)
        .arg(m_feedback, 0, 'f', 2)
        .arg(m_crossfeed, 0, 'f', 2)
        .arg(m_drymix, 0, 'f', 2);
}

QString FFStereowiden::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFStereowiden::toJSON(QJsonObject& json) const {
    json["type"] = "ff-stereowiden";
    json["delay"] = m_delay;
    json["feedback"] = m_feedback;
    json["crossfeed"] = m_crossfeed;
    json["drymix"] = m_drymix;
}

void FFStereowiden::fromJSON(const QJsonObject& json) {
    m_delay = json["delay"].toDouble(20.0);
    m_feedback = json["feedback"].toDouble(0.3);
    m_crossfeed = json["crossfeed"].toDouble(0.3);
    m_drymix = json["drymix"].toDouble(0.8);
    updateFFmpegFlags();
}
