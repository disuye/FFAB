#include "ff-rubberband.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <cmath>

FFRubberband::FFRubberband() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

double FFRubberband::semitonesToPitchFactor(double semitones) const {
    return std::pow(2.0, semitones / 12.0);
}

double FFRubberband::pitchFactorToSemitones(double factor) const {
    if (factor <= 0.0) return 0.0;
    return 12.0 * std::log2(factor);
}

QWidget* FFRubberband::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: MAIN (Tempo & Pitch) ==========
    auto mainTab = new QWidget();
    auto mainTabLayout = new QVBoxLayout(mainTab);

    // Tempo
    auto tempoGroup = new QGroupBox("Tempo Scale");
    auto tempoLayout = new QVBoxLayout(tempoGroup);

    tempoSpinBox = new QDoubleSpinBox();
    tempoSpinBox->setMinimum(0.25);
    tempoSpinBox->setMaximum(4.0);
    tempoSpinBox->setValue(m_tempo);
    tempoSpinBox->setDecimals(3);
    tempoSpinBox->setSingleStep(0.05);
    tempoSpinBox->setSuffix("x");

    auto tempoControlLayout = new QHBoxLayout();
    tempoControlLayout->addWidget(tempoSpinBox);
    tempoControlLayout->addWidget(new QLabel("0.25x"));
    tempoSlider = new QSlider(Qt::Horizontal);
    tempoSlider->setMinimum(250);
    tempoSlider->setMaximum(4000);
    tempoSlider->setValue(static_cast<int>(m_tempo * 1000));
    tempoControlLayout->addWidget(tempoSlider, 1);
    tempoControlLayout->addWidget(new QLabel("4x"));
    tempoLayout->addLayout(tempoControlLayout);

    auto tempoHint = new QLabel(
        "Speed up or slow down playback.\n"
        "< 1.0 = slower, > 1.0 = faster\n"
        "0.5x = half speed, 2.0x = double speed"
    );
    tempoHint->setStyleSheet("color: gray; font-size: 11px;");
    tempoHint->setWordWrap(true);
    tempoLayout->addWidget(tempoHint);

    mainTabLayout->addWidget(tempoGroup);

    // Pitch
    auto pitchGroup = new QGroupBox("Pitch Shift");
    auto pitchLayout = new QVBoxLayout(pitchGroup);

    // Semitones control (more intuitive)
    auto semitonesLayout = new QHBoxLayout();
    semitonesLayout->addWidget(new QLabel("Semitones:"));
    semitonesSpinBox = new QDoubleSpinBox();
    semitonesSpinBox->setMinimum(-24.0);
    semitonesSpinBox->setMaximum(24.0);
    semitonesSpinBox->setValue(pitchFactorToSemitones(m_pitch));
    semitonesSpinBox->setDecimals(2);
    semitonesSpinBox->setSingleStep(1.0);
    semitonesSpinBox->setSuffix(" st");
    semitonesLayout->addWidget(semitonesSpinBox);
    semitonesLayout->addStretch();
    pitchLayout->addLayout(semitonesLayout);

    // Pitch factor (raw)
    pitchSpinBox = new QDoubleSpinBox();
    pitchSpinBox->setMinimum(0.25);
    pitchSpinBox->setMaximum(4.0);
    pitchSpinBox->setValue(m_pitch);
    pitchSpinBox->setDecimals(4);
    pitchSpinBox->setSingleStep(0.01);
    pitchSpinBox->setPrefix("Factor: ");

    auto pitchControlLayout = new QHBoxLayout();
    pitchControlLayout->addWidget(pitchSpinBox);
    pitchControlLayout->addWidget(new QLabel("0.25"));
    pitchSlider = new QSlider(Qt::Horizontal);
    pitchSlider->setMinimum(250);
    pitchSlider->setMaximum(4000);
    pitchSlider->setValue(static_cast<int>(m_pitch * 1000));
    pitchControlLayout->addWidget(pitchSlider, 1);
    pitchControlLayout->addWidget(new QLabel("4.0"));
    pitchLayout->addLayout(pitchControlLayout);

    auto pitchHint = new QLabel(
        "Shift pitch without changing tempo.\n"
        "±12 semitones = ±1 octave\n"
        "Factor 2.0 = +12 semitones (octave up)"
    );
    pitchHint->setStyleSheet("color: gray; font-size: 11px;");
    pitchHint->setWordWrap(true);
    pitchLayout->addWidget(pitchHint);

    mainTabLayout->addWidget(pitchGroup);

    mainTabLayout->addStretch();
    tabWidget->addTab(mainTab, "Tempo / Pitch");

    // ========== TAB 2: QUALITY ==========
    auto qualityTab = new QWidget();
    auto qualityLayout = new QVBoxLayout(qualityTab);

    // Transients
    auto transientsGroup = new QGroupBox("Transient Handling");
    auto transientsLayout = new QVBoxLayout(transientsGroup);

    transientsCombo = new QComboBox();
    transientsCombo->addItem("Crisp (preserve attacks)", 0);
    transientsCombo->addItem("Mixed (balanced)", 1);
    transientsCombo->addItem("Smooth (reduce artifacts)", 2);
    transientsCombo->setCurrentIndex(m_transients);
    transientsLayout->addWidget(transientsCombo);

    qualityLayout->addWidget(transientsGroup);

    // Detector
    auto detectorGroup = new QGroupBox("Transient Detector");
    auto detectorLayout = new QVBoxLayout(detectorGroup);

    detectorCombo = new QComboBox();
    detectorCombo->addItem("Compound (general purpose)", 0);
    detectorCombo->addItem("Percussive (drums/beats)", 1);
    detectorCombo->addItem("Soft (vocals/strings)", 2);
    detectorCombo->setCurrentIndex(m_detector);
    detectorLayout->addWidget(detectorCombo);

    qualityLayout->addWidget(detectorGroup);

    // Pitch quality
    auto pitchqGroup = new QGroupBox("Pitch Shift Quality");
    auto pitchqLayout = new QVBoxLayout(pitchqGroup);

    pitchqCombo = new QComboBox();
    pitchqCombo->addItem("Quality (best, slower)", 0);
    pitchqCombo->addItem("Speed (faster processing)", 1);
    pitchqCombo->addItem("Consistency (stable timing)", 2);
    pitchqCombo->setCurrentIndex(m_pitchq);
    pitchqLayout->addWidget(pitchqCombo);

    qualityLayout->addWidget(pitchqGroup);

    // Formant preservation
    auto formantGroup = new QGroupBox("Formant Handling");
    auto formantLayout = new QVBoxLayout(formantGroup);

    formantCombo = new QComboBox();
    formantCombo->addItem("Shifted (natural for instruments)", 0);
    formantCombo->addItem("Preserved (natural for vocals)", 1);
    formantCombo->setCurrentIndex(m_formant);
    formantLayout->addWidget(formantCombo);

    auto formantHint = new QLabel(
        "Formant preservation keeps vocal character when pitch-shifting. "
        "Use 'Preserved' for voice to avoid chipmunk/monster effects."
    );
    formantHint->setStyleSheet("color: gray; font-size: 11px;");
    formantHint->setWordWrap(true);
    formantLayout->addWidget(formantHint);

    qualityLayout->addWidget(formantGroup);

    qualityLayout->addStretch();
    tabWidget->addTab(qualityTab, "Quality");

    // ========== TAB 3: ADVANCED ==========
    auto advancedTab = new QWidget();
    auto advancedLayout = new QVBoxLayout(advancedTab);

    // Phase
    auto phaseGroup = new QGroupBox("Phase Mode");
    auto phaseLayout = new QVBoxLayout(phaseGroup);

    phaseCombo = new QComboBox();
    phaseCombo->addItem("Laminar (coherent phase)", 0);
    phaseCombo->addItem("Independent (per-channel)", 1);
    phaseCombo->setCurrentIndex(m_phase);
    phaseLayout->addWidget(phaseCombo);

    advancedLayout->addWidget(phaseGroup);

    // Window
    auto windowGroup = new QGroupBox("Analysis Window");
    auto windowLayout = new QVBoxLayout(windowGroup);

    windowCombo = new QComboBox();
    windowCombo->addItem("Standard", 0);
    windowCombo->addItem("Short (better transients)", 1);
    windowCombo->addItem("Long (better frequency resolution)", 2);
    windowCombo->setCurrentIndex(m_window);
    windowLayout->addWidget(windowCombo);

    advancedLayout->addWidget(windowGroup);

    // Smoothing
    auto smoothingGroup = new QGroupBox("Smoothing");
    auto smoothingLayout = new QVBoxLayout(smoothingGroup);

    smoothingCombo = new QComboBox();
    smoothingCombo->addItem("Off", 0);
    smoothingCombo->addItem("On (reduce flutter)", 1);
    smoothingCombo->setCurrentIndex(m_smoothing);
    smoothingLayout->addWidget(smoothingCombo);

    advancedLayout->addWidget(smoothingGroup);

    // Channels
    auto channelsGroup = new QGroupBox("Channel Processing");
    auto channelsLayout = new QVBoxLayout(channelsGroup);

    channelsCombo = new QComboBox();
    channelsCombo->addItem("Apart (independent channels)", 0);
    channelsCombo->addItem("Together (linked processing)", 1);
    channelsCombo->setCurrentIndex(m_channels);
    channelsLayout->addWidget(channelsCombo);

    advancedLayout->addWidget(channelsGroup);

    advancedLayout->addStretch();
    tabWidget->addTab(advancedTab, "Advanced");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Tempo
    connect(tempoSlider, &QSlider::valueChanged, [this](int value) {
        m_tempo = value / 1000.0;
        tempoSpinBox->blockSignals(true);
        tempoSpinBox->setValue(m_tempo);
        tempoSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(tempoSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_tempo = value;
        tempoSlider->blockSignals(true);
        tempoSlider->setValue(static_cast<int>(value * 1000));
        tempoSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Pitch (slider and factor spinbox)
    connect(pitchSlider, &QSlider::valueChanged, [this](int value) {
        m_pitch = value / 1000.0;
        pitchSpinBox->blockSignals(true);
        pitchSpinBox->setValue(m_pitch);
        pitchSpinBox->blockSignals(false);
        semitonesSpinBox->blockSignals(true);
        semitonesSpinBox->setValue(pitchFactorToSemitones(m_pitch));
        semitonesSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(pitchSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_pitch = value;
        pitchSlider->blockSignals(true);
        pitchSlider->setValue(static_cast<int>(value * 1000));
        pitchSlider->blockSignals(false);
        semitonesSpinBox->blockSignals(true);
        semitonesSpinBox->setValue(pitchFactorToSemitones(m_pitch));
        semitonesSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    // Semitones (linked to pitch)
    connect(semitonesSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_pitch = semitonesToPitchFactor(value);
        pitchSlider->blockSignals(true);
        pitchSlider->setValue(static_cast<int>(m_pitch * 1000));
        pitchSlider->blockSignals(false);
        pitchSpinBox->blockSignals(true);
        pitchSpinBox->setValue(m_pitch);
        pitchSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    // Quality options
    connect(transientsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_transients = index;
        updateFFmpegFlags();
    });
    connect(detectorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_detector = index;
        updateFFmpegFlags();
    });
    connect(pitchqCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_pitchq = index;
        updateFFmpegFlags();
    });
    connect(formantCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_formant = index;
        updateFFmpegFlags();
    });

    // Advanced options
    connect(phaseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_phase = index;
        updateFFmpegFlags();
    });
    connect(windowCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_window = index;
        updateFFmpegFlags();
    });
    connect(smoothingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_smoothing = index;
        updateFFmpegFlags();
    });
    connect(channelsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_channels = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFRubberband::updateFFmpegFlags() {
    QStringList transientsNames = {"crisp", "mixed", "smooth"};
    QStringList detectorNames = {"compound", "percussive", "soft"};
    QStringList phaseNames = {"laminar", "independent"};
    QStringList windowNames = {"standard", "short", "long"};
    QStringList smoothingNames = {"off", "on"};
    QStringList formantNames = {"shifted", "preserved"};
    QStringList pitchqNames = {"quality", "speed", "consistency"};
    QStringList channelsNames = {"apart", "together"};

    QString transients = (m_transients >= 0 && m_transients < transientsNames.size()) ? transientsNames[m_transients] : "crisp";
    QString detector = (m_detector >= 0 && m_detector < detectorNames.size()) ? detectorNames[m_detector] : "compound";
    QString phase = (m_phase >= 0 && m_phase < phaseNames.size()) ? phaseNames[m_phase] : "laminar";
    QString window = (m_window >= 0 && m_window < windowNames.size()) ? windowNames[m_window] : "standard";
    QString smoothing = (m_smoothing >= 0 && m_smoothing < smoothingNames.size()) ? smoothingNames[m_smoothing] : "off";
    QString formant = (m_formant >= 0 && m_formant < formantNames.size()) ? formantNames[m_formant] : "shifted";
    QString pitchq = (m_pitchq >= 0 && m_pitchq < pitchqNames.size()) ? pitchqNames[m_pitchq] : "speed";
    QString channels = (m_channels >= 0 && m_channels < channelsNames.size()) ? channelsNames[m_channels] : "apart";

    ffmpegFlags = QString("rubberband=tempo=%1:pitch=%2:transients=%3:detector=%4:phase=%5:window=%6:smoothing=%7:formant=%8:pitchq=%9:channels=%10")
                      .arg(m_tempo, 0, 'f', 4)
                      .arg(m_pitch, 0, 'f', 4)
                      .arg(transients)
                      .arg(detector)
                      .arg(phase)
                      .arg(window)
                      .arg(smoothing)
                      .arg(formant)
                      .arg(pitchq)
                      .arg(channels);
}

QString FFRubberband::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFRubberband::toJSON(QJsonObject& json) const {
    json["type"] = "ff-rubberband";
    json["tempo"] = m_tempo;
    json["pitch"] = m_pitch;
    json["transients"] = m_transients;
    json["detector"] = m_detector;
    json["phase"] = m_phase;
    json["window"] = m_window;
    json["smoothing"] = m_smoothing;
    json["formant"] = m_formant;
    json["pitchq"] = m_pitchq;
    json["channels"] = m_channels;
}

void FFRubberband::fromJSON(const QJsonObject& json) {
    m_tempo = json["tempo"].toDouble(1.0);
    m_pitch = json["pitch"].toDouble(1.0);
    m_transients = json["transients"].toInt(0);
    m_detector = json["detector"].toInt(0);
    m_phase = json["phase"].toInt(0);
    m_window = json["window"].toInt(0);
    m_smoothing = json["smoothing"].toInt(0);
    m_formant = json["formant"].toInt(0);
    m_pitchq = json["pitchq"].toInt(0);
    m_channels = json["channels"].toInt(0);
    updateFFmpegFlags();
}
