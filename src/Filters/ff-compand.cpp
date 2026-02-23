#include "ff-compand.h"
#include "CompandCurveWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <algorithm>

FFCompand::FFCompand() {
    position = Position::MIDDLE;

    m_points.append({-70.0, -70.0});
    m_points.append({-60.0, -20.0});
    m_points.append({1.0, 0.0});

    updateFFmpegFlags();
}

// ==================== POINT MANAGEMENT ====================

void FFCompand::sortPoints() {
    std::sort(m_points.begin(), m_points.end(),
              [](const CompandPoint& a, const CompandPoint& b) {
                  return a.inputDb < b.inputDb;
              });
}

void FFCompand::addPoint(double inputDb, double outputDb) {
    if (m_points.size() >= MAX_POINTS) return;

    m_points.append({inputDb, outputDb});
    sortPoints();
    updateFFmpegFlags();
    if (curveWidget) curveWidget->update();
}

void FFCompand::removePoint(int index) {
    if (index < 0 || index >= m_points.size()) return;
    if (m_points.size() <= 2) return;

    m_points.removeAt(index);
    updateFFmpegFlags();
    if (curveWidget) curveWidget->update();
}

int FFCompand::updatePoint(int index, double inputDb, double outputDb) {
    if (index < 0 || index >= m_points.size()) return index;

    m_points[index].inputDb = inputDb;
    m_points[index].outputDb = outputDb;

    CompandPoint edited = m_points[index];
    sortPoints();

    // Find where the edited point ended up after sorting
    int newIndex = index;
    for (int i = 0; i < m_points.size(); ++i) {
        if (qFuzzyCompare(m_points[i].inputDb, edited.inputDb) &&
            qFuzzyCompare(m_points[i].outputDb, edited.outputDb)) {
            newIndex = i;
            break;
        }
    }

    updateFFmpegFlags();
    return newIndex;
}

void FFCompand::setSoftKnee(double knee) {
    m_softKnee = qBound(0.01, knee, 48.0);

    if (softKneeSlider) {
        softKneeSlider->blockSignals(true);
        softKneeSlider->setValue(static_cast<int>(m_softKnee * 100));
        softKneeSlider->blockSignals(false);
    }
    if (softKneeSpinBox) {
        softKneeSpinBox->blockSignals(true);
        softKneeSpinBox->setValue(m_softKnee);
        softKneeSpinBox->blockSignals(false);
    }

    updateFFmpegFlags();
}

// ==================== STRING HELPERS ====================

QString FFCompand::pointsToString() const {
    QStringList parts;
    for (const CompandPoint& pt : m_points) {
        parts.append(QString("%1/%2")
                     .arg(pt.inputDb, 0, 'g', 6)
                     .arg(pt.outputDb, 0, 'g', 6));
    }
    return parts.join("|");
}

void FFCompand::parsePointsString(const QString& str) {
    m_points.clear();
    QStringList pairs = str.split("|", Qt::SkipEmptyParts);
    for (const QString& pair : pairs) {
        QStringList values = pair.split("/");
        if (values.size() == 2) {
            CompandPoint pt;
            pt.inputDb = values[0].toDouble();
            pt.outputDb = values[1].toDouble();
            m_points.append(pt);
        }
    }
    sortPoints();
}

// ==================== FFMPEG FLAGS ====================

void FFCompand::updateFFmpegFlags() {
    ffmpegFlags = QString("compand=attacks=%1:decays=%2:points=%3:soft-knee=%4:gain=%5:volume=%6:delay=%7")
                      .arg(m_attack, 0, 'f', 4)
                      .arg(m_decay, 0, 'f', 4)
                      .arg(pointsToString())
                      .arg(m_softKnee, 0, 'f', 2)
                      .arg(m_gain, 0, 'f', 1)
                      .arg(m_volume, 0, 'f', 1)
                      .arg(m_delay, 0, 'f', 2);
}

QString FFCompand::buildFFmpegFlags() const {
    return ffmpegFlags;
}

// ==================== UI ====================

QWidget* FFCompand::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ===== CURVE WIDGET =====
    curveWidget = new CompandCurveWidget(this);
    curveWidget->setMinimumHeight(250);
    mainLayout->addWidget(curveWidget, 1);

    auto helpLabel = new QLabel(
        "<small><b>Controls:</b> Alt+Click: add/delete point | "
        "Drag: move point | Ctrl+Drag: adjust soft-knee</small>");
    helpLabel->setStyleSheet("color: #808080;");
    helpLabel->setWordWrap(true);
    mainLayout->addWidget(helpLabel);

    // ===== ATTACK (ms) =====
    auto attackGroup = new QGroupBox("Attack (ms)");
    auto attackLayout = new QHBoxLayout(attackGroup);

    attackSpinBox = new QDoubleSpinBox();
    attackSpinBox->setMinimum(0.0);
    attackSpinBox->setMaximum(5000.0);
    attackSpinBox->setValue(m_attack * 1000.0);
    attackSpinBox->setDecimals(1);
    attackSpinBox->setSingleStep(1.0);
    attackSpinBox->setSuffix(" ms");
    attackLayout->addWidget(attackSpinBox);

    attackLayout->addWidget(new QLabel("0"));

    attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setMinimum(0);
    attackSlider->setMaximum(5000);
    attackSlider->setValue(static_cast<int>(m_attack * 1000.0));
    attackLayout->addWidget(attackSlider, 1);

    attackLayout->addWidget(new QLabel("5000"));

    mainLayout->addWidget(attackGroup);

    // ===== DECAY (ms) =====
    auto decayGroup = new QGroupBox("Decay (ms)");
    auto decayLayout = new QHBoxLayout(decayGroup);

    decaySpinBox = new QDoubleSpinBox();
    decaySpinBox->setMinimum(0.0);
    decaySpinBox->setMaximum(10000.0);
    decaySpinBox->setValue(m_decay * 1000.0);
    decaySpinBox->setDecimals(1);
    decaySpinBox->setSingleStep(1.0);
    decaySpinBox->setSuffix(" ms");
    decayLayout->addWidget(decaySpinBox);

    decayLayout->addWidget(new QLabel("0"));

    decaySlider = new QSlider(Qt::Horizontal);
    decaySlider->setMinimum(0);
    decaySlider->setMaximum(10000);
    decaySlider->setValue(static_cast<int>(m_decay * 1000.0));
    decayLayout->addWidget(decaySlider, 1);

    decayLayout->addWidget(new QLabel("10000"));

    mainLayout->addWidget(decayGroup);

    // ===== SOFT KNEE (dB) =====
    auto kneeGroup = new QGroupBox("Soft Knee (dB)");
    auto kneeLayout = new QHBoxLayout(kneeGroup);

    softKneeSpinBox = new QDoubleSpinBox();
    softKneeSpinBox->setMinimum(0.01);
    softKneeSpinBox->setMaximum(48.0);
    softKneeSpinBox->setValue(m_softKnee);
    softKneeSpinBox->setDecimals(2);
    softKneeSpinBox->setSingleStep(0.5);
    softKneeSpinBox->setSuffix(" dB");
    kneeLayout->addWidget(softKneeSpinBox);

    kneeLayout->addWidget(new QLabel("0"));

    softKneeSlider = new QSlider(Qt::Horizontal);
    softKneeSlider->setMinimum(1);
    softKneeSlider->setMaximum(4800);
    softKneeSlider->setValue(static_cast<int>(m_softKnee * 100));
    kneeLayout->addWidget(softKneeSlider, 1);

    kneeLayout->addWidget(new QLabel("48"));

    mainLayout->addWidget(kneeGroup);

    // ===== OUTPUT GAIN (dB) =====
    auto gainGroup = new QGroupBox("Output Gain (dB)");
    auto gainLayout2 = new QHBoxLayout(gainGroup);

    gainSpinBox = new QDoubleSpinBox();
    gainSpinBox->setMinimum(-900.0);
    gainSpinBox->setMaximum(900.0);
    gainSpinBox->setValue(m_gain);
    gainSpinBox->setDecimals(1);
    gainSpinBox->setSingleStep(0.5);
    gainSpinBox->setSuffix(" dB");
    gainLayout2->addWidget(gainSpinBox);

    gainLayout2->addWidget(new QLabel("-30"));

    gainSlider = new QSlider(Qt::Horizontal);
    gainSlider->setMinimum(-3000);
    gainSlider->setMaximum(3000);
    gainSlider->setValue(static_cast<int>(m_gain * 100));
    gainLayout2->addWidget(gainSlider, 1);

    gainLayout2->addWidget(new QLabel("+30"));

    mainLayout->addWidget(gainGroup);

    // ===== INITIAL VOLUME (dB) =====
    auto volumeGroup = new QGroupBox("Initial Volume (dB)");
    auto volumeLayout2 = new QHBoxLayout(volumeGroup);

    volumeSpinBox = new QDoubleSpinBox();
    volumeSpinBox->setMinimum(-900.0);
    volumeSpinBox->setMaximum(0.0);
    volumeSpinBox->setValue(m_volume);
    volumeSpinBox->setDecimals(1);
    volumeSpinBox->setSingleStep(1.0);
    volumeSpinBox->setSuffix(" dB");
    volumeLayout2->addWidget(volumeSpinBox);

    volumeLayout2->addWidget(new QLabel("-90"));

    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setMinimum(-9000);
    volumeSlider->setMaximum(0);
    volumeSlider->setValue(static_cast<int>(m_volume * 100));
    volumeLayout2->addWidget(volumeSlider, 1);

    volumeLayout2->addWidget(new QLabel("0"));

    mainLayout->addWidget(volumeGroup);

    // ===== DELAY (seconds) =====
    auto delayGroup = new QGroupBox("Delay (seconds)");
    auto delayLayout2 = new QHBoxLayout(delayGroup);

    delaySpinBox = new QDoubleSpinBox();
    delaySpinBox->setMinimum(0.0);
    delaySpinBox->setMaximum(20.0);
    delaySpinBox->setValue(m_delay);
    delaySpinBox->setDecimals(2);
    delaySpinBox->setSingleStep(0.1);
    delaySpinBox->setSuffix(" s");
    delayLayout2->addWidget(delaySpinBox);

    delayLayout2->addWidget(new QLabel("0"));

    delaySlider = new QSlider(Qt::Horizontal);
    delaySlider->setMinimum(0);
    delaySlider->setMaximum(2000);
    delaySlider->setValue(static_cast<int>(m_delay * 100));
    delayLayout2->addWidget(delaySlider, 1);

    delayLayout2->addWidget(new QLabel("20"));

    mainLayout->addWidget(delayGroup);

    // ===== SIGNAL CONNECTIONS =====

    // Attack: ms in UI, seconds in storage
    connect(attackSlider, &QSlider::valueChanged, [this](int value) {
        m_attack = value / 1000.0;
        attackSpinBox->blockSignals(true);
        attackSpinBox->setValue(value);
        attackSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(attackSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        m_attack = value / 1000.0;
        attackSlider->blockSignals(true);
        attackSlider->setValue(static_cast<int>(value));
        attackSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Decay: ms in UI, seconds in storage
    connect(decaySlider, &QSlider::valueChanged, [this](int value) {
        m_decay = value / 1000.0;
        decaySpinBox->blockSignals(true);
        decaySpinBox->setValue(value);
        decaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(decaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        m_decay = value / 1000.0;
        decaySlider->blockSignals(true);
        decaySlider->setValue(static_cast<int>(value));
        decaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Soft Knee
    connect(softKneeSlider, &QSlider::valueChanged, [this](int value) {
        m_softKnee = value / 100.0;
        softKneeSpinBox->blockSignals(true);
        softKneeSpinBox->setValue(m_softKnee);
        softKneeSpinBox->blockSignals(false);
        updateFFmpegFlags();
        if (curveWidget) curveWidget->update();
    });
    connect(softKneeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        m_softKnee = value;
        softKneeSlider->blockSignals(true);
        softKneeSlider->setValue(static_cast<int>(value * 100));
        softKneeSlider->blockSignals(false);
        updateFFmpegFlags();
        if (curveWidget) curveWidget->update();
    });

    // Output Gain
    connect(gainSlider, &QSlider::valueChanged, [this](int value) {
        m_gain = value / 100.0;
        gainSpinBox->blockSignals(true);
        gainSpinBox->setValue(m_gain);
        gainSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(gainSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        m_gain = value;
        gainSlider->blockSignals(true);
        gainSlider->setValue(static_cast<int>(value * 100));
        gainSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Initial Volume
    connect(volumeSlider, &QSlider::valueChanged, [this](int value) {
        m_volume = value / 100.0;
        volumeSpinBox->blockSignals(true);
        volumeSpinBox->setValue(m_volume);
        volumeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(volumeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        m_volume = value;
        volumeSlider->blockSignals(true);
        volumeSlider->setValue(static_cast<int>(value * 100));
        volumeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Delay
    connect(delaySlider, &QSlider::valueChanged, [this](int value) {
        m_delay = value / 100.0;
        delaySpinBox->blockSignals(true);
        delaySpinBox->setValue(m_delay);
        delaySpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(delaySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        m_delay = value;
        delaySlider->blockSignals(true);
        delaySlider->setValue(static_cast<int>(value * 100));
        delaySlider->blockSignals(false);
        updateFFmpegFlags();
    });

    mainLayout->addStretch();

    return parametersWidget;
}

// ==================== JSON ====================

void FFCompand::toJSON(QJsonObject& json) const {
    json["type"] = "ff-compand";
    json["version"] = 2;

    json["attack"] = m_attack;
    json["decay"] = m_decay;
    json["softKnee"] = m_softKnee;
    json["gain"] = m_gain;
    json["volume"] = m_volume;
    json["delay"] = m_delay;

    QJsonArray pointsArray;
    for (const CompandPoint& pt : m_points) {
        QJsonObject ptObj;
        ptObj["in"] = pt.inputDb;
        ptObj["out"] = pt.outputDb;
        pointsArray.append(ptObj);
    }
    json["transferPoints"] = pointsArray;
}

void FFCompand::fromJSON(const QJsonObject& json) {
    int version = json["version"].toInt(1);

    if (version >= 2) {
        m_attack = json["attack"].toDouble(0.05);
        m_decay = json["decay"].toDouble(0.01);
        m_softKnee = json["softKnee"].toDouble(0.01);
        m_gain = json["gain"].toDouble(-12.0);
        m_volume = json["volume"].toDouble(-90.0);
        m_delay = json["delay"].toDouble(0.0);

        m_points.clear();
        QJsonArray pointsArray = json["transferPoints"].toArray();
        for (const QJsonValue& val : pointsArray) {
            QJsonObject ptObj = val.toObject();
            CompandPoint pt;
            pt.inputDb = ptObj["in"].toDouble(0.0);
            pt.outputDb = ptObj["out"].toDouble(0.0);
            m_points.append(pt);
        }
        sortPoints();
    } else {
        // Legacy v1 format
        QString attacksStr = json["attacks"].toString("0.05");
        m_attack = attacksStr.toDouble();
        if (m_attack == 0.0) m_attack = 0.05;

        QString decaysStr = json["decays"].toString("0.25");
        m_decay = decaysStr.toDouble();

        QString pointsStr = json["points"].toString("-70/-70|-60/-20|1/0");
        parsePointsString(pointsStr);

        m_softKnee = 0.01;
        m_gain = json["gain"].toDouble(0.0);
        m_volume = json["volume"].toDouble(0.0);
        m_delay = json["delay"].toDouble(0.0);
    }

    if (m_points.size() < 2) {
        m_points.clear();
        m_points.append({-70.0, -70.0});
        m_points.append({-60.0, -20.0});
        m_points.append({1.0, 0.0});
    }

    updateFFmpegFlags();
    if (curveWidget) curveWidget->update();
}
