#include "ff-surround.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>

FFSurround::FFSurround() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFSurround::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: MAIN ==========
    auto mainTab = new QWidget();
    auto mainTabLayout = new QVBoxLayout(mainTab);

    // Channel layouts
    auto layoutGroup = new QGroupBox("Channel Configuration");
    auto layoutLayout = new QVBoxLayout(layoutGroup);

    auto inLayoutLayout = new QHBoxLayout();
    inLayoutLayout->addWidget(new QLabel("Input:"));
    chlInCombo = new QComboBox();
    chlInCombo->addItem("Mono", "mono");
    chlInCombo->addItem("Stereo", "stereo");
    chlInCombo->addItem("3.0", "3.0");
    chlInCombo->setCurrentIndex(1);
    inLayoutLayout->addWidget(chlInCombo);
    inLayoutLayout->addStretch();
    layoutLayout->addLayout(inLayoutLayout);

    auto outLayoutLayout = new QHBoxLayout();
    outLayoutLayout->addWidget(new QLabel("Output:"));
    chlOutCombo = new QComboBox();
    chlOutCombo->addItem("3.0", "3.0");
    chlOutCombo->addItem("4.0 (Quad)", "quad");
    chlOutCombo->addItem("5.0", "5.0");
    chlOutCombo->addItem("5.1", "5.1");
    chlOutCombo->addItem("6.1", "6.1");
    chlOutCombo->addItem("7.1", "7.1");
    chlOutCombo->setCurrentIndex(2); // 5.1
    outLayoutLayout->addWidget(chlOutCombo);
    outLayoutLayout->addStretch();
    layoutLayout->addLayout(outLayoutLayout);

    mainTabLayout->addWidget(layoutGroup);

    // Master levels
    auto levelGroup = new QGroupBox("Master Levels");
    auto levelLayout = new QVBoxLayout(levelGroup);

    auto levelInLayout = new QHBoxLayout();
    levelInLayout->addWidget(new QLabel("Input:"));
    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(0.0);
    levelInSpinBox->setMaximum(10.0);
    levelInSpinBox->setValue(m_levelIn);
    levelInSpinBox->setDecimals(2);
    levelInSpinBox->setSingleStep(0.1);
    levelInLayout->addWidget(levelInSpinBox);
    levelInLayout->addStretch();
    levelLayout->addLayout(levelInLayout);

    auto levelOutLayout = new QHBoxLayout();
    levelOutLayout->addWidget(new QLabel("Output:"));
    levelOutSpinBox = new QDoubleSpinBox();
    levelOutSpinBox->setMinimum(0.0);
    levelOutSpinBox->setMaximum(10.0);
    levelOutSpinBox->setValue(m_levelOut);
    levelOutSpinBox->setDecimals(2);
    levelOutSpinBox->setSingleStep(0.1);
    levelOutLayout->addWidget(levelOutSpinBox);
    levelOutLayout->addStretch();
    levelLayout->addLayout(levelOutLayout);

    mainTabLayout->addWidget(levelGroup);

    // Soundfield controls
    auto soundfieldGroup = new QGroupBox("Soundfield");
    auto soundfieldLayout = new QVBoxLayout(soundfieldGroup);

    // Angle
    auto angleLayout = new QHBoxLayout();
    angleLayout->addWidget(new QLabel("Angle:"));
    angleSpinBox = new QDoubleSpinBox();
    angleSpinBox->setMinimum(0.0);
    angleSpinBox->setMaximum(360.0);
    angleSpinBox->setValue(m_angle);
    angleSpinBox->setDecimals(1);
    angleSpinBox->setSuffix("°");
    angleLayout->addWidget(angleSpinBox);
    angleSlider = new QSlider(Qt::Horizontal);
    angleSlider->setMinimum(0);
    angleSlider->setMaximum(3600);
    angleSlider->setValue(static_cast<int>(m_angle * 10));
    angleLayout->addWidget(angleSlider, 1);
    soundfieldLayout->addLayout(angleLayout);

    // Focus
    auto focusLayout = new QHBoxLayout();
    focusLayout->addWidget(new QLabel("Focus:"));
    focusSpinBox = new QDoubleSpinBox();
    focusSpinBox->setMinimum(-1.0);
    focusSpinBox->setMaximum(1.0);
    focusSpinBox->setValue(m_focus);
    focusSpinBox->setDecimals(2);
    focusSpinBox->setSingleStep(0.05);
    focusLayout->addWidget(focusSpinBox);
    focusSlider = new QSlider(Qt::Horizontal);
    focusSlider->setMinimum(-100);
    focusSlider->setMaximum(100);
    focusSlider->setValue(static_cast<int>(m_focus * 100));
    focusLayout->addWidget(focusSlider, 1);
    soundfieldLayout->addLayout(focusLayout);

    // Smooth
    auto smoothLayout = new QHBoxLayout();
    smoothLayout->addWidget(new QLabel("Smooth:"));
    smoothSpinBox = new QDoubleSpinBox();
    smoothSpinBox->setMinimum(0.0);
    smoothSpinBox->setMaximum(1.0);
    smoothSpinBox->setValue(m_smooth);
    smoothSpinBox->setDecimals(2);
    smoothSpinBox->setSingleStep(0.05);
    smoothLayout->addWidget(smoothSpinBox);
    smoothSlider = new QSlider(Qt::Horizontal);
    smoothSlider->setMinimum(0);
    smoothSlider->setMaximum(100);
    smoothSlider->setValue(static_cast<int>(m_smooth * 100));
    smoothLayout->addWidget(smoothSlider, 1);
    soundfieldLayout->addLayout(smoothLayout);

    auto soundfieldHint = new QLabel("Angle rotates the soundfield, Focus adjusts width, Smooth reduces flutter.");
    soundfieldHint->setStyleSheet("color: gray; font-size: 11px;");
    soundfieldHint->setWordWrap(true);
    soundfieldLayout->addWidget(soundfieldHint);

    mainTabLayout->addWidget(soundfieldGroup);

    mainTabLayout->addStretch();
    tabWidget->addTab(mainTab, "Main");

    // ========== TAB 2: LFE ==========
    auto lfeTab = new QWidget();
    auto lfeTabLayout = new QVBoxLayout(lfeTab);

    auto lfeGroup = new QGroupBox("LFE (Subwoofer) Settings");
    auto lfeLayout = new QVBoxLayout(lfeGroup);

    lfeCheck = new QCheckBox("Enable LFE output");
    lfeCheck->setChecked(m_lfe);
    lfeLayout->addWidget(lfeCheck);

    auto lfeLowLayout = new QHBoxLayout();
    lfeLowLayout->addWidget(new QLabel("Low cutoff (Hz):"));
    lfeLowSpinBox = new QSpinBox();
    lfeLowSpinBox->setMinimum(0);
    lfeLowSpinBox->setMaximum(256);
    lfeLowSpinBox->setValue(m_lfeLow);
    lfeLowSpinBox->setSuffix(" Hz");
    lfeLowLayout->addWidget(lfeLowSpinBox);
    lfeLowLayout->addStretch();
    lfeLayout->addLayout(lfeLowLayout);

    auto lfeHighLayout = new QHBoxLayout();
    lfeHighLayout->addWidget(new QLabel("High cutoff (Hz):"));
    lfeHighSpinBox = new QSpinBox();
    lfeHighSpinBox->setMinimum(0);
    lfeHighSpinBox->setMaximum(512);
    lfeHighSpinBox->setValue(m_lfeHigh);
    lfeHighSpinBox->setSuffix(" Hz");
    lfeHighLayout->addWidget(lfeHighSpinBox);
    lfeHighLayout->addStretch();
    lfeLayout->addLayout(lfeHighLayout);

    auto lfeModeLayout = new QHBoxLayout();
    lfeModeLayout->addWidget(new QLabel("LFE mode:"));
    lfeModeCombo = new QComboBox();
    lfeModeCombo->addItem("Add (boost bass)", 0);
    lfeModeCombo->addItem("Sub (redirect bass)", 1);
    lfeModeCombo->setCurrentIndex(m_lfeMode);
    lfeModeLayout->addWidget(lfeModeCombo);
    lfeModeLayout->addStretch();
    lfeLayout->addLayout(lfeModeLayout);

    auto lfeHint = new QLabel(
        "Add mode: LFE content is added while keeping bass in main channels.\n"
        "Sub mode: Bass is extracted and redirected to LFE only."
    );
    lfeHint->setStyleSheet("color: gray; font-size: 11px;");
    lfeHint->setWordWrap(true);
    lfeLayout->addWidget(lfeHint);

    lfeTabLayout->addWidget(lfeGroup);
    lfeTabLayout->addStretch();
    tabWidget->addTab(lfeTab, "LFE");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Layouts
    connect(chlInCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_chlIn = chlInCombo->currentData().toString();
        updateFFmpegFlags();
    });

    connect(chlOutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int) {
        m_chlOut = chlOutCombo->currentData().toString();
        updateFFmpegFlags();
    });

    // Levels
    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelIn = value;
        updateFFmpegFlags();
    });

    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_levelOut = value;
        updateFFmpegFlags();
    });

    // Angle
    connect(angleSlider, &QSlider::valueChanged, [this](int value) {
        m_angle = value / 10.0;
        angleSpinBox->blockSignals(true);
        angleSpinBox->setValue(m_angle);
        angleSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(angleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_angle = value;
        angleSlider->blockSignals(true);
        angleSlider->setValue(static_cast<int>(value * 10));
        angleSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Focus
    connect(focusSlider, &QSlider::valueChanged, [this](int value) {
        m_focus = value / 100.0;
        focusSpinBox->blockSignals(true);
        focusSpinBox->setValue(m_focus);
        focusSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(focusSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_focus = value;
        focusSlider->blockSignals(true);
        focusSlider->setValue(static_cast<int>(value * 100));
        focusSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Smooth
    connect(smoothSlider, &QSlider::valueChanged, [this](int value) {
        m_smooth = value / 100.0;
        smoothSpinBox->blockSignals(true);
        smoothSpinBox->setValue(m_smooth);
        smoothSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(smoothSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_smooth = value;
        smoothSlider->blockSignals(true);
        smoothSlider->setValue(static_cast<int>(value * 100));
        smoothSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // LFE
    connect(lfeCheck, &QCheckBox::toggled, [this](bool checked) {
        m_lfe = checked;
        updateFFmpegFlags();
    });

    connect(lfeLowSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_lfeLow = value;
        updateFFmpegFlags();
    });

    connect(lfeHighSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_lfeHigh = value;
        updateFFmpegFlags();
    });

    connect(lfeModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_lfeMode = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFSurround::updateFFmpegFlags() {
    QStringList lfeModeNames = {"add", "sub"};
    QString lfeModeName = (m_lfeMode >= 0 && m_lfeMode < lfeModeNames.size()) ? lfeModeNames[m_lfeMode] : "add";
    
    ffmpegFlags = QString("surround=chl_out=%1:chl_in=%2:level_in=%3:level_out=%4:lfe=%5:lfe_low=%6:lfe_high=%7:lfe_mode=%8:smooth=%9:angle=%10:focus=%11")
                      .arg(m_chlOut)
                      .arg(m_chlIn)
                      .arg(m_levelIn, 0, 'f', 2)
                      .arg(m_levelOut, 0, 'f', 2)
                      .arg(m_lfe ? "true" : "false")
                      .arg(m_lfeLow)
                      .arg(m_lfeHigh)
                      .arg(lfeModeName)
                      .arg(m_smooth, 0, 'f', 2)
                      .arg(m_angle, 0, 'f', 1)
                      .arg(m_focus, 0, 'f', 2);
}

QString FFSurround::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFSurround::toJSON(QJsonObject& json) const {
    json["type"] = "ff-surround";
    json["chl_out"] = m_chlOut;
    json["chl_in"] = m_chlIn;
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["lfe"] = m_lfe;
    json["lfe_low"] = m_lfeLow;
    json["lfe_high"] = m_lfeHigh;
    json["lfe_mode"] = m_lfeMode;
    json["smooth"] = m_smooth;
    json["angle"] = m_angle;
    json["focus"] = m_focus;
}

void FFSurround::fromJSON(const QJsonObject& json) {
    m_chlOut = json["chl_out"].toString("5.1");
    m_chlIn = json["chl_in"].toString("stereo");
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_lfe = json["lfe"].toBool(true);
    m_lfeLow = json["lfe_low"].toInt(128);
    m_lfeHigh = json["lfe_high"].toInt(256);
    m_lfeMode = json["lfe_mode"].toInt(0);
    m_smooth = json["smooth"].toDouble(0.0);
    m_angle = json["angle"].toDouble(90.0);
    m_focus = json["focus"].toDouble(0.0);
    updateFFmpegFlags();
}
