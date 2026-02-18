#include "ff-superequalizer.h"
#include "SnapSlider.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <QPushButton>
#include <QScrollArea>
#include <QAbstractSpinBox>
#include <cmath>

constexpr std::array<const char*, 18> FFSuperequalizer::bandLabels;

// Convert dB to linear gain for FFmpeg
double FFSuperequalizer::dbToLinear(double db) {
    // Clamp to reasonable range
    if (db <= -60.0) return 0.0;
    return std::pow(10.0, db / 20.0);
}

// Convert linear gain to dB for display
double FFSuperequalizer::linearToDb(double linear) {
    if (linear <= 0.0) return -60.0;
    return 20.0 * std::log10(linear);
}

FFSuperequalizer::FFSuperequalizer() {
    position = Position::MIDDLE;
    // Initialize all bands to 0 dB (unity)
    m_bandsDb.fill(0.0);
    updateFFmpegFlags();
}

QWidget* FFSuperequalizer::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // Compact header with reset button
    auto headerLayout = new QHBoxLayout();
    auto resetBtn = new QPushButton("Reset EQ");
    resetBtn->setToolTip("Reset all to 0 dB");
    headerLayout->addWidget(resetBtn);
    headerLayout->addStretch(1);
    
    auto infoLabel = new QLabel("EQ bands in Hz | Gain +/-20dB");
    headerLayout->addWidget(infoLabel);
    headerLayout->addStretch(20);
    
    mainLayout->addLayout(headerLayout);

    // dB scale reference
    // auto scaleLayout = new QHBoxLayout();
    // scaleLayout->addWidget(new QLabel("+20"));
    // scaleLayout->addStretch();
    // auto zeroLabel = new QLabel("0 dB");
    // zeroLabel->setStyleSheet("color: #808080;");
    // scaleLayout->addWidget(zeroLabel);
    // scaleLayout->addStretch();
    // scaleLayout->addWidget(new QLabel("-20"));
    // mainLayout->addLayout(scaleLayout);

    // Scrollable EQ area
    auto scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    auto eqWidget = new QWidget();
    auto eqLayout = new QHBoxLayout(eqWidget);
    eqLayout->setSpacing(2);
    eqLayout->setContentsMargins(0, 0, 0, 0);

    // Create 18 band controls (vertical sliders)
    for (int i = 0; i < 18; i++) {
        auto bandWidget = new QWidget();
        bandWidget->setObjectName("superEqualizerTarget");
        auto bandLayout = new QVBoxLayout(bandWidget);
        bandLayout->setSpacing(1);
        bandLayout->setContentsMargins(1, 1, 1, 1);

        // dB value spinbox at top (no buttons, compact)
        bandSpinBoxes[i] = new QDoubleSpinBox();
        bandSpinBoxes[i]->setMinimum(-20.0);
        bandSpinBoxes[i]->setMaximum(20.0);
        bandSpinBoxes[i]->setValue(m_bandsDb[i]);
        bandSpinBoxes[i]->setDecimals(1);
        bandSpinBoxes[i]->setSingleStep(0.5);
        bandSpinBoxes[i]->setButtonSymbols(QAbstractSpinBox::NoButtons);
        bandSpinBoxes[i]->setAlignment(Qt::AlignCenter);
        bandSpinBoxes[i]->setFixedWidth(44); // global styling is min-width 88px main.cpp
        bandSpinBoxes[i]->setStyleSheet("QDoubleSpinBox {padding: 1px;}");
        bandLayout->addWidget(bandSpinBoxes[i], 0, Qt::AlignHCenter);
        
        // Frequency label above slider
        auto freqLabel = new QLabel(bandLabels[i]);
        freqLabel->setStyleSheet("font-size: 11px;");
        freqLabel->setAlignment(Qt::AlignCenter);
        bandLayout->addWidget(freqLabel, 0, Qt::AlignHCenter);
        
        // Vertical slider (-20 to +20 dB, 0 in middle)
        bandSliders[i] = new SnapSlider(Qt::Vertical); // permits 'center detent'
        bandSliders[i]->setSnapValue(0);      // 0 = 0 dB
        bandSliders[i]->setSnapThreshold(3); // snap zone, keep it small
        bandSliders[i]->setMinimum(-200);  // -20.0 dB
        bandSliders[i]->setMaximum(200);   // +20.0 dB
        bandSliders[i]->setValue(static_cast<int>(m_bandsDb[i] * 10));
        bandSliders[i]->setMaximumHeight(390);
        bandSliders[i]->setMinimumHeight(130);
        bandSliders[i]->setMaximumWidth(40);
        bandSliders[i]->setTickPosition(QSlider::TicksBothSides);
        bandSliders[i]->setTickInterval(100);  // Tick at -10, 0, +10 dB
        bandLayout->addWidget(bandSliders[i], 10, Qt::AlignHCenter);
        bandLayout->addStretch(1); // with above value '2' = 2:1 content to stretch ratio
        eqLayout->addWidget(bandWidget);

        // Connect signals
        int bandIndex = i;  // Capture for lambda
        
        connect(bandSliders[i], &QSlider::valueChanged, [this, bandIndex](int value) {
            m_bandsDb[bandIndex] = value / 10.0;
            bandSpinBoxes[bandIndex]->blockSignals(true);
            bandSpinBoxes[bandIndex]->setValue(m_bandsDb[bandIndex]);
            bandSpinBoxes[bandIndex]->blockSignals(false);
            updateFFmpegFlags();
        });

        connect(bandSpinBoxes[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
                [this, bandIndex](double value) {
            m_bandsDb[bandIndex] = value;
            bandSliders[bandIndex]->blockSignals(true);
            bandSliders[bandIndex]->setValue(static_cast<int>(value * 10));
            bandSliders[bandIndex]->blockSignals(false);
            updateFFmpegFlags();
        });
    }

    eqLayout->addStretch();
    scrollArea->setWidget(eqWidget);
    mainLayout->addWidget(scrollArea, 1);

    // Reset button connection
    connect(resetBtn, &QPushButton::clicked, [this]() {
        for (int i = 0; i < 18; i++) {
            m_bandsDb[i] = 0.0;
            bandSliders[i]->blockSignals(true);
            bandSliders[i]->setValue(0);
            bandSliders[i]->blockSignals(false);
            bandSpinBoxes[i]->blockSignals(true);
            bandSpinBoxes[i]->setValue(0.0);
            bandSpinBoxes[i]->blockSignals(false);
        }
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFSuperequalizer::updateFFmpegFlags() {
    QStringList params;
    for (int i = 0; i < 18; i++) {
        // Convert dB to linear gain for FFmpeg
        double linearGain = dbToLinear(m_bandsDb[i]);
        // Clamp to FFmpeg's 0-20 range
        linearGain = std::max(0.0, std::min(20.0, linearGain));
        params << QString("%1b=%2").arg(i + 1).arg(linearGain, 0, 'f', 4);
    }
    ffmpegFlags = QString("superequalizer=%1").arg(params.join(":"));
}

QString FFSuperequalizer::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFSuperequalizer::toJSON(QJsonObject& json) const {
    json["type"] = "ff-superequalizer";
    QJsonArray bandsArray;
    for (int i = 0; i < 18; i++) {
        bandsArray.append(m_bandsDb[i]);
    }
    json["bands_db"] = bandsArray;
}

void FFSuperequalizer::fromJSON(const QJsonObject& json) {
    // Support both old format (linear) and new format (dB)
    if (json.contains("bands_db")) {
        QJsonArray bandsArray = json["bands_db"].toArray();
        for (int i = 0; i < 18 && i < bandsArray.size(); i++) {
            m_bandsDb[i] = bandsArray[i].toDouble(0.0);
        }
    } else if (json.contains("bands")) {
        // Legacy: convert from linear gain
        QJsonArray bandsArray = json["bands"].toArray();
        for (int i = 0; i < 18 && i < bandsArray.size(); i++) {
            double linear = bandsArray[i].toDouble(1.0);
            m_bandsDb[i] = linearToDb(linear);
        }
    }
    updateFFmpegFlags();
}