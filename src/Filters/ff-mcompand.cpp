#include "ff-mcompand.h"
#include "CompandCurveWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <QTabWidget>
#include <QPushButton>
#include <QScrollArea>

FFMcompand::FFMcompand() {
    position = Position::MIDDLE;

    // 5 default bands matching the classic mcompand example
    // Band 1: 0–100 Hz
    McompandBand b1;
    b1.attack = 0.005; b1.decay = 0.1; b1.crossoverFreq = 100;
    b1.curveData.softKnee = 6.0;
    b1.curveData.points = {{-47, -40}, {-34, -34}, {-17, -33}};
    m_bands.append(b1);

    // Band 2: 100–400 Hz
    McompandBand b2;
    b2.attack = 0.003; b2.decay = 0.05; b2.crossoverFreq = 400;
    b2.curveData.softKnee = 6.0;
    b2.curveData.points = {{-47, -40}, {-34, -34}, {-17, -33}};
    m_bands.append(b2);

    // Band 3: 400–1600 Hz
    McompandBand b3;
    b3.attack = 0.000625; b3.decay = 0.0125; b3.crossoverFreq = 1600;
    b3.curveData.softKnee = 6.0;
    b3.curveData.points = {{-47, -40}, {-34, -34}, {-15, -33}};
    m_bands.append(b3);

    // Band 4: 1600–6400 Hz
    McompandBand b4;
    b4.attack = 0.0001; b4.decay = 0.025; b4.crossoverFreq = 6400;
    b4.curveData.softKnee = 6.0;
    b4.curveData.points = {{-47, -40}, {-34, -34}, {-31, -31}, {0, -30}};
    m_bands.append(b4);

    // Band 5: 6400–22000 Hz
    McompandBand b5;
    b5.attack = 0.0; b5.decay = 0.025; b5.crossoverFreq = 22000;
    b5.curveData.softKnee = 6.0;
    b5.curveData.points = {{-38, -31}, {-28, -28}, {0, -25}};
    m_bands.append(b5);

    updateFFmpegFlags();
}

// ==================== FFMPEG FLAGS ====================

QString FFMcompand::bandToString(const McompandBand& band) const {
    // Format: attack,decay soft-knee pt1,pt2,...,ptN crossover [delay [volume [gain]]]
    QString attackDecay = QString("%1,%2")
        .arg(band.attack, 0, 'g', 6)
        .arg(band.decay, 0, 'g', 6);

    QString knee = QString::number(band.curveData.softKnee, 'g', 6);

    QStringList pts;
    for (const CompandPoint& pt : band.curveData.points) {
        pts.append(QString("%1/%2")
                   .arg(pt.inputDb, 0, 'g', 6)
                   .arg(pt.outputDb, 0, 'g', 6));
    }
    QString points = pts.join(",");

    QString crossover = QString::number(band.crossoverFreq, 'g', 6);

    QString result = QString("%1 %2 %3 %4").arg(attackDecay, knee, points, crossover);

    // Append optional params only if non-default
    if (band.delay != 0.0 || band.volume != 0.0 || band.gain != 0.0) {
        result += QString(" %1").arg(band.delay, 0, 'g', 6);
        if (band.volume != 0.0 || band.gain != 0.0) {
            result += QString(" %1").arg(band.volume, 0, 'g', 6);
            if (band.gain != 0.0) {
                result += QString(" %1").arg(band.gain, 0, 'g', 6);
            }
        }
    }

    return result;
}

void FFMcompand::updateFFmpegFlags() {
    if (m_bands.isEmpty()) {
        ffmpegFlags = "mcompand";
        return;
    }

    QStringList bandStrings;
    for (const McompandBand& band : m_bands) {
        bandStrings.append(bandToString(band));
    }

    QString args = bandStrings.join(" | ");
    // Escape single quotes for FFmpeg
    args.replace("'", "'\\''");
    ffmpegFlags = QString("mcompand='%1'").arg(args);
}

QString FFMcompand::buildFFmpegFlags() const {
    return ffmpegFlags;
}

// ==================== LEGACY ARGS PARSER ====================

void FFMcompand::parseLegacyArgs(const QString& args) {
    m_bands.clear();

    QStringList bandStrings = args.split("|");
    for (const QString& bandStr : bandStrings) {
        QString trimmed = bandStr.trimmed();
        if (trimmed.isEmpty()) continue;

        McompandBand band;

        // Split by whitespace to get tokens
        QStringList tokens = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (tokens.isEmpty()) continue;

        // Token 0: attack,decay
        if (tokens.size() >= 1) {
            QStringList ad = tokens[0].split(",");
            if (ad.size() >= 1) band.attack = ad[0].toDouble();
            if (ad.size() >= 2) band.decay = ad[1].toDouble();
        }

        // Token 1: soft-knee (dB)
        if (tokens.size() >= 2) {
            band.curveData.softKnee = tokens[1].toDouble();
        }

        // Token 2: transfer points (comma-separated "in/out" pairs)
        if (tokens.size() >= 3) {
            band.curveData.points.clear();
            QStringList ptPairs = tokens[2].split(",");
            for (const QString& pair : ptPairs) {
                QStringList vals = pair.split("/");
                if (vals.size() == 2) {
                    CompandPoint pt;
                    pt.inputDb = vals[0].toDouble();
                    pt.outputDb = vals[1].toDouble();
                    band.curveData.points.append(pt);
                }
            }
            band.curveData.sortPoints();
        }

        // Token 3: crossover frequency
        if (tokens.size() >= 4) {
            band.crossoverFreq = tokens[3].toDouble();
        }

        // Optional tokens
        if (tokens.size() >= 5) band.delay = tokens[4].toDouble();
        if (tokens.size() >= 6) band.volume = tokens[5].toDouble();
        if (tokens.size() >= 7) band.gain = tokens[6].toDouble();

        // Ensure minimum 2 points
        if (band.curveData.points.size() < 2) {
            band.curveData.points.clear();
            band.curveData.points.append({-70, -70});
            band.curveData.points.append({0, 0});
        }

        m_bands.append(band);
    }

    if (m_bands.isEmpty()) {
        // Fallback: one default band
        McompandBand def;
        def.curveData.points = {{-70, -70}, {0, 0}};
        def.crossoverFreq = 22000;
        m_bands.append(def);
    }
}

// ==================== UI ====================

QWidget* FFMcompand::createBandTab(int bandIndex) {
    McompandBand& band = m_bands[bandIndex];
    BandWidgets& bw = m_bandWidgets[bandIndex];

    // Scroll area so the tab content can scroll if panel is short
    auto scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto tabContent = new QWidget();
    auto layout = new QVBoxLayout(tabContent);
    layout->setSpacing(8);
    layout->setContentsMargins(4, 4, 4, 4);

    // ===== CURVE WIDGET =====
    bw.curve = new CompandCurveWidget(&band.curveData);
    bw.curve->setMinimumHeight(200);
    layout->addWidget(bw.curve, 1);

    connect(bw.curve, &CompandCurveWidget::dataChanged, this, [this, bandIndex]() {
        McompandBand& b = m_bands[bandIndex];
        BandWidgets& w = m_bandWidgets[bandIndex];
        if (w.kneeSlider) {
            w.kneeSlider->blockSignals(true);
            w.kneeSlider->setValue(static_cast<int>(b.curveData.softKnee * 100));
            w.kneeSlider->blockSignals(false);
        }
        if (w.kneeSpin) {
            w.kneeSpin->blockSignals(true);
            w.kneeSpin->setValue(b.curveData.softKnee);
            w.kneeSpin->blockSignals(false);
        }
        updateFFmpegFlags();
    });

    auto helpLabel = new QLabel(
        "<small><b>Controls:</b> Alt+Click: add/delete point | "
        "Drag: move point | Ctrl+Drag: adjust soft-knee</small>");
    helpLabel->setStyleSheet("color: #808080;");
    helpLabel->setWordWrap(true);
    layout->addWidget(helpLabel);

    // ===== ATTACK (ms) =====
    auto attackGroup = new QGroupBox("Attack (ms)");
    auto attackLayout = new QHBoxLayout(attackGroup);

    bw.attackSpin = new QDoubleSpinBox();
    bw.attackSpin->setMinimum(0.0);
    bw.attackSpin->setMaximum(5000.0);
    bw.attackSpin->setValue(band.attack * 1000.0);
    bw.attackSpin->setDecimals(2);
    bw.attackSpin->setSingleStep(0.5);
    bw.attackSpin->setSuffix(" ms");
    attackLayout->addWidget(bw.attackSpin);

    attackLayout->addWidget(new QLabel("0"));

    bw.attackSlider = new QSlider(Qt::Horizontal);
    bw.attackSlider->setMinimum(0);
    bw.attackSlider->setMaximum(5000);
    bw.attackSlider->setValue(static_cast<int>(band.attack * 1000.0));
    attackLayout->addWidget(bw.attackSlider, 1);

    attackLayout->addWidget(new QLabel("5000"));

    layout->addWidget(attackGroup);

    // ===== DECAY (ms) =====
    auto decayGroup = new QGroupBox("Decay (ms)");
    auto decayLayout = new QHBoxLayout(decayGroup);

    bw.decaySpin = new QDoubleSpinBox();
    bw.decaySpin->setMinimum(0.0);
    bw.decaySpin->setMaximum(10000.0);
    bw.decaySpin->setValue(band.decay * 1000.0);
    bw.decaySpin->setDecimals(2);
    bw.decaySpin->setSingleStep(1.0);
    bw.decaySpin->setSuffix(" ms");
    decayLayout->addWidget(bw.decaySpin);

    decayLayout->addWidget(new QLabel("0"));

    bw.decaySlider = new QSlider(Qt::Horizontal);
    bw.decaySlider->setMinimum(0);
    bw.decaySlider->setMaximum(10000);
    bw.decaySlider->setValue(static_cast<int>(band.decay * 1000.0));
    decayLayout->addWidget(bw.decaySlider, 1);

    decayLayout->addWidget(new QLabel("10000"));

    layout->addWidget(decayGroup);

    // ===== SOFT KNEE (dB) =====
    auto kneeGroup = new QGroupBox("Soft Knee (dB)");
    auto kneeLayout = new QHBoxLayout(kneeGroup);

    bw.kneeSpin = new QDoubleSpinBox();
    bw.kneeSpin->setMinimum(0.01);
    bw.kneeSpin->setMaximum(48.0);
    bw.kneeSpin->setValue(band.curveData.softKnee);
    bw.kneeSpin->setDecimals(2);
    bw.kneeSpin->setSingleStep(0.5);
    bw.kneeSpin->setSuffix(" dB");
    kneeLayout->addWidget(bw.kneeSpin);

    kneeLayout->addWidget(new QLabel("0"));

    bw.kneeSlider = new QSlider(Qt::Horizontal);
    bw.kneeSlider->setMinimum(1);
    bw.kneeSlider->setMaximum(4800);
    bw.kneeSlider->setValue(static_cast<int>(band.curveData.softKnee * 100));
    kneeLayout->addWidget(bw.kneeSlider, 1);

    kneeLayout->addWidget(new QLabel("48"));

    layout->addWidget(kneeGroup);

    // ===== CROSSOVER FREQUENCY (Hz) =====
    auto crossGroup = new QGroupBox("Crossover Frequency (Hz)");
    auto crossLayout = new QHBoxLayout(crossGroup);

    bw.crossoverSpin = new QDoubleSpinBox();
    bw.crossoverSpin->setMinimum(20.0);
    bw.crossoverSpin->setMaximum(22000.0);
    bw.crossoverSpin->setValue(band.crossoverFreq);
    bw.crossoverSpin->setDecimals(0);
    bw.crossoverSpin->setSingleStep(10.0);
    bw.crossoverSpin->setSuffix(" Hz");
    crossLayout->addWidget(bw.crossoverSpin);

    crossLayout->addWidget(new QLabel("20"));

    bw.crossoverSlider = new QSlider(Qt::Horizontal);
    bw.crossoverSlider->setMinimum(20);
    bw.crossoverSlider->setMaximum(22000);
    bw.crossoverSlider->setValue(static_cast<int>(band.crossoverFreq));
    crossLayout->addWidget(bw.crossoverSlider, 1);

    crossLayout->addWidget(new QLabel("22000"));

    layout->addWidget(crossGroup);

    // ===== OUTPUT GAIN (dB) =====
    auto gainGroup = new QGroupBox("Output Gain (dB)");
    auto gainLayout = new QHBoxLayout(gainGroup);

    bw.gainSpin = new QDoubleSpinBox();
    bw.gainSpin->setMinimum(-900.0);
    bw.gainSpin->setMaximum(900.0);
    bw.gainSpin->setValue(band.gain);
    bw.gainSpin->setDecimals(1);
    bw.gainSpin->setSingleStep(0.5);
    bw.gainSpin->setSuffix(" dB");
    gainLayout->addWidget(bw.gainSpin);

    gainLayout->addWidget(new QLabel("-30"));

    bw.gainSlider = new QSlider(Qt::Horizontal);
    bw.gainSlider->setMinimum(-3000);
    bw.gainSlider->setMaximum(3000);
    bw.gainSlider->setValue(static_cast<int>(band.gain * 100));
    gainLayout->addWidget(bw.gainSlider, 1);

    gainLayout->addWidget(new QLabel("+30"));

    layout->addWidget(gainGroup);

    // ===== INITIAL VOLUME (dB) =====
    auto volumeGroup = new QGroupBox("Initial Volume (dB)");
    auto volumeLayout = new QHBoxLayout(volumeGroup);

    bw.volumeSpin = new QDoubleSpinBox();
    bw.volumeSpin->setMinimum(-900.0);
    bw.volumeSpin->setMaximum(0.0);
    bw.volumeSpin->setValue(band.volume);
    bw.volumeSpin->setDecimals(1);
    bw.volumeSpin->setSingleStep(1.0);
    bw.volumeSpin->setSuffix(" dB");
    volumeLayout->addWidget(bw.volumeSpin);

    volumeLayout->addWidget(new QLabel("-90"));

    bw.volumeSlider = new QSlider(Qt::Horizontal);
    bw.volumeSlider->setMinimum(-9000);
    bw.volumeSlider->setMaximum(0);
    bw.volumeSlider->setValue(static_cast<int>(band.volume * 100));
    volumeLayout->addWidget(bw.volumeSlider, 1);

    volumeLayout->addWidget(new QLabel("0"));

    layout->addWidget(volumeGroup);

    // ===== DELAY (seconds) =====
    auto delayGroup = new QGroupBox("Delay (seconds)");
    auto delayLayout = new QHBoxLayout(delayGroup);

    bw.delaySpin = new QDoubleSpinBox();
    bw.delaySpin->setMinimum(0.0);
    bw.delaySpin->setMaximum(20.0);
    bw.delaySpin->setValue(band.delay);
    bw.delaySpin->setDecimals(2);
    bw.delaySpin->setSingleStep(0.1);
    bw.delaySpin->setSuffix(" s");
    delayLayout->addWidget(bw.delaySpin);

    delayLayout->addWidget(new QLabel("0"));

    bw.delaySlider = new QSlider(Qt::Horizontal);
    bw.delaySlider->setMinimum(0);
    bw.delaySlider->setMaximum(2000);
    bw.delaySlider->setValue(static_cast<int>(band.delay * 100));
    delayLayout->addWidget(bw.delaySlider, 1);

    delayLayout->addWidget(new QLabel("20"));

    layout->addWidget(delayGroup);

    layout->addStretch();

    // ===== SIGNAL CONNECTIONS =====

    // Attack
    connect(bw.attackSlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].attack = value / 1000.0;
        m_bandWidgets[bandIndex].attackSpin->blockSignals(true);
        m_bandWidgets[bandIndex].attackSpin->setValue(value);
        m_bandWidgets[bandIndex].attackSpin->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bw.attackSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].attack = value / 1000.0;
        m_bandWidgets[bandIndex].attackSlider->blockSignals(true);
        m_bandWidgets[bandIndex].attackSlider->setValue(static_cast<int>(value));
        m_bandWidgets[bandIndex].attackSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Decay
    connect(bw.decaySlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].decay = value / 1000.0;
        m_bandWidgets[bandIndex].decaySpin->blockSignals(true);
        m_bandWidgets[bandIndex].decaySpin->setValue(value);
        m_bandWidgets[bandIndex].decaySpin->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bw.decaySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].decay = value / 1000.0;
        m_bandWidgets[bandIndex].decaySlider->blockSignals(true);
        m_bandWidgets[bandIndex].decaySlider->setValue(static_cast<int>(value));
        m_bandWidgets[bandIndex].decaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Soft Knee
    connect(bw.kneeSlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].curveData.softKnee = value / 100.0;
        m_bandWidgets[bandIndex].kneeSpin->blockSignals(true);
        m_bandWidgets[bandIndex].kneeSpin->setValue(value / 100.0);
        m_bandWidgets[bandIndex].kneeSpin->blockSignals(false);
        updateFFmpegFlags();
        if (m_bandWidgets[bandIndex].curve) m_bandWidgets[bandIndex].curve->update();
    });
    connect(bw.kneeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].curveData.softKnee = value;
        m_bandWidgets[bandIndex].kneeSlider->blockSignals(true);
        m_bandWidgets[bandIndex].kneeSlider->setValue(static_cast<int>(value * 100));
        m_bandWidgets[bandIndex].kneeSlider->blockSignals(false);
        updateFFmpegFlags();
        if (m_bandWidgets[bandIndex].curve) m_bandWidgets[bandIndex].curve->update();
    });

    // Crossover Frequency
    connect(bw.crossoverSlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].crossoverFreq = value;
        m_bandWidgets[bandIndex].crossoverSpin->blockSignals(true);
        m_bandWidgets[bandIndex].crossoverSpin->setValue(value);
        m_bandWidgets[bandIndex].crossoverSpin->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bw.crossoverSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].crossoverFreq = value;
        m_bandWidgets[bandIndex].crossoverSlider->blockSignals(true);
        m_bandWidgets[bandIndex].crossoverSlider->setValue(static_cast<int>(value));
        m_bandWidgets[bandIndex].crossoverSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Output Gain
    connect(bw.gainSlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].gain = value / 100.0;
        m_bandWidgets[bandIndex].gainSpin->blockSignals(true);
        m_bandWidgets[bandIndex].gainSpin->setValue(value / 100.0);
        m_bandWidgets[bandIndex].gainSpin->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bw.gainSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].gain = value;
        m_bandWidgets[bandIndex].gainSlider->blockSignals(true);
        m_bandWidgets[bandIndex].gainSlider->setValue(static_cast<int>(value * 100));
        m_bandWidgets[bandIndex].gainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Initial Volume
    connect(bw.volumeSlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].volume = value / 100.0;
        m_bandWidgets[bandIndex].volumeSpin->blockSignals(true);
        m_bandWidgets[bandIndex].volumeSpin->setValue(value / 100.0);
        m_bandWidgets[bandIndex].volumeSpin->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bw.volumeSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].volume = value;
        m_bandWidgets[bandIndex].volumeSlider->blockSignals(true);
        m_bandWidgets[bandIndex].volumeSlider->setValue(static_cast<int>(value * 100));
        m_bandWidgets[bandIndex].volumeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Delay
    connect(bw.delaySlider, &QSlider::valueChanged, this, [this, bandIndex](int value) {
        m_bands[bandIndex].delay = value / 100.0;
        m_bandWidgets[bandIndex].delaySpin->blockSignals(true);
        m_bandWidgets[bandIndex].delaySpin->setValue(value / 100.0);
        m_bandWidgets[bandIndex].delaySpin->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(bw.delaySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this, bandIndex](double value) {
        m_bands[bandIndex].delay = value;
        m_bandWidgets[bandIndex].delaySlider->blockSignals(true);
        m_bandWidgets[bandIndex].delaySlider->setValue(static_cast<int>(value * 100));
        m_bandWidgets[bandIndex].delaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    scrollArea->setWidget(tabContent);
    return scrollArea;
}

void FFMcompand::rebuildUI() {
    if (!tabWidget) return;

    // Remove all existing tabs
    while (tabWidget->count() > 0) {
        QWidget* w = tabWidget->widget(0);
        tabWidget->removeTab(0);
        delete w;
    }
    m_bandWidgets.clear();

    // Create tabs for all bands
    for (int i = 0; i < m_bands.size(); ++i) {
        m_bandWidgets.append(BandWidgets{});
        QWidget* tab = createBandTab(i);
        tabWidget->addTab(tab, QString("Band %1").arg(i + 1));
    }
}

QWidget* FFMcompand::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Tab widget for bands
    tabWidget = new QTabWidget();
    mainLayout->addWidget(tabWidget, 1);

    // Build tabs for all bands
    for (int i = 0; i < m_bands.size(); ++i) {
        m_bandWidgets.append(BandWidgets{});
        QWidget* tab = createBandTab(i);
        tabWidget->addTab(tab, QString("Band %1").arg(i + 1));
    }

    // Add/Remove band buttons
    auto buttonLayout = new QHBoxLayout();

    auto addButton = new QPushButton("+ Add Band");
    connect(addButton, &QPushButton::clicked, this, [this]() {
        if (m_bands.size() >= MAX_BANDS) return;

        McompandBand newBand;
        newBand.curveData.softKnee = 6.0;
        newBand.curveData.points = {{-47, -40}, {-34, -34}, {0, -25}};
        // Default crossover above the last band's
        if (!m_bands.isEmpty()) {
            double lastCross = m_bands.last().crossoverFreq;
            newBand.crossoverFreq = qMin(22000.0, lastCross * 2.0);
        } else {
            newBand.crossoverFreq = 1000;
        }
        m_bands.append(newBand);

        m_bandWidgets.append(BandWidgets{});
        int idx = m_bands.size() - 1;
        QWidget* tab = createBandTab(idx);
        tabWidget->addTab(tab, QString("Band %1").arg(idx + 1));
        tabWidget->setCurrentIndex(idx);

        updateFFmpegFlags();
    });
    buttonLayout->addWidget(addButton);

    auto removeButton = new QPushButton("- Remove Band");
    connect(removeButton, &QPushButton::clicked, this, [this]() {
        if (m_bands.size() <= 1) return;

        int idx = tabWidget->currentIndex();
        if (idx < 0 || idx >= m_bands.size()) return;

        // Remove the tab and its widgets
        QWidget* w = tabWidget->widget(idx);
        tabWidget->removeTab(idx);
        delete w;

        m_bands.removeAt(idx);
        m_bandWidgets.removeAt(idx);

        // Rebuild all tabs to fix captured bandIndex in lambdas
        rebuildUI();

        updateFFmpegFlags();
    });
    buttonLayout->addWidget(removeButton);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    return parametersWidget;
}

// ==================== JSON ====================

void FFMcompand::toJSON(QJsonObject& json) const {
    json["type"] = "ff-mcompand";
    json["version"] = 2;

    QJsonArray bandsArray;
    for (const McompandBand& band : m_bands) {
        QJsonObject bandObj;
        bandObj["attack"] = band.attack;
        bandObj["decay"] = band.decay;
        bandObj["softKnee"] = band.curveData.softKnee;
        bandObj["crossoverFreq"] = band.crossoverFreq;
        bandObj["gain"] = band.gain;
        bandObj["volume"] = band.volume;
        bandObj["delay"] = band.delay;

        QJsonArray pointsArray;
        for (const CompandPoint& pt : band.curveData.points) {
            QJsonObject ptObj;
            ptObj["in"] = pt.inputDb;
            ptObj["out"] = pt.outputDb;
            pointsArray.append(ptObj);
        }
        bandObj["transferPoints"] = pointsArray;

        bandsArray.append(bandObj);
    }
    json["bands"] = bandsArray;
}

void FFMcompand::fromJSON(const QJsonObject& json) {
    int version = json["version"].toInt(1);

    if (version >= 2) {
        m_bands.clear();
        QJsonArray bandsArray = json["bands"].toArray();
        for (const QJsonValue& val : bandsArray) {
            QJsonObject bandObj = val.toObject();
            McompandBand band;
            band.attack = bandObj["attack"].toDouble(0.005);
            band.decay = bandObj["decay"].toDouble(0.1);
            band.curveData.softKnee = bandObj["softKnee"].toDouble(6.0);
            band.crossoverFreq = bandObj["crossoverFreq"].toDouble(1000);
            band.gain = bandObj["gain"].toDouble(0.0);
            band.volume = bandObj["volume"].toDouble(0.0);
            band.delay = bandObj["delay"].toDouble(0.0);

            band.curveData.points.clear();
            QJsonArray pointsArray = bandObj["transferPoints"].toArray();
            for (const QJsonValue& ptVal : pointsArray) {
                QJsonObject ptObj = ptVal.toObject();
                CompandPoint pt;
                pt.inputDb = ptObj["in"].toDouble(0.0);
                pt.outputDb = ptObj["out"].toDouble(0.0);
                band.curveData.points.append(pt);
            }
            band.curveData.sortPoints();

            if (band.curveData.points.size() < 2) {
                band.curveData.points.clear();
                band.curveData.points.append({-70, -70});
                band.curveData.points.append({0, 0});
            }

            m_bands.append(band);
        }
    } else {
        // Legacy v1: raw args string
        QString args = json["args"].toString();
        if (!args.isEmpty()) {
            parseLegacyArgs(args);
        }
    }

    if (m_bands.isEmpty()) {
        McompandBand def;
        def.curveData.softKnee = 6.0;
        def.curveData.points = {{-70, -70}, {0, 0}};
        def.crossoverFreq = 22000;
        m_bands.append(def);
    }

    updateFFmpegFlags();

    // If UI exists, rebuild tabs
    if (tabWidget) {
        rebuildUI();
    }
}
