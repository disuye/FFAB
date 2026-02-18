#include "ff-aemphasis.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAemphasis::FFAemphasis() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAemphasis::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // dB conversion helpers
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -60.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Mode parameter
    auto modeGroup = new QGroupBox("Mode");
    auto modeLayout = new QVBoxLayout(modeGroup);

    modeCombo = new QComboBox();
    modeCombo->addItem("Reproduction (De-emphasis)", 0);
    modeCombo->addItem("Production (Pre-emphasis)", 1);
    modeCombo->setCurrentIndex(m_mode);
    modeLayout->addWidget(modeCombo);

    auto modeHint = new QLabel(
        "De-emphasis: Apply to already-emphasized audio to restore flat response.\n"
        "Pre-emphasis: Apply before recording/transmission to boost highs."
    );
    modeHint->setStyleSheet("color: gray; font-size: 11px;");
    modeHint->setWordWrap(true);
    modeLayout->addWidget(modeHint);

    mainLayout->addWidget(modeGroup);

    // Type parameter
    auto typeGroup = new QGroupBox("Emphasis Curve");
    auto typeLayout = new QVBoxLayout(typeGroup);

    typeCombo = new QComboBox();
    typeCombo->addItem("CD (IEC 908)", 0);
    typeCombo->addItem("50µs FM (Europe)", 1);
    typeCombo->addItem("75µs FM (USA)", 2);
    typeCombo->addItem("50µs KF", 3);
    typeCombo->addItem("75µs KF", 4);
    typeCombo->addItem("RIAA (Vinyl)", 5);
    typeCombo->addItem("EMI 78 RPM", 6);
    typeCombo->addItem("EMI 2 (Columbia)", 7);
    typeCombo->addItem("Columbia LP", 8);
    typeCombo->setCurrentIndex(m_type);
    typeLayout->addWidget(typeCombo);

    auto typeHint = new QLabel(
        "Select the emphasis curve matching your source material."
    );
    typeHint->setStyleSheet("color: gray; font-size: 11px;");
    typeLayout->addWidget(typeHint);

    mainLayout->addWidget(typeGroup);

    // Input Level parameter (displayed in dB)
    auto levelInGroup = new QGroupBox("Input Level (dB)");
    auto levelInLayout = new QVBoxLayout(levelInGroup);

    double levelInDb = linearToDb(m_levelIn);
    levelInSpinBox = new QDoubleSpinBox();
    levelInSpinBox->setMinimum(-60.0);
    levelInSpinBox->setMaximum(36.0);
    levelInSpinBox->setValue(levelInDb);
    levelInSpinBox->setDecimals(1);
    levelInSpinBox->setSingleStep(0.5);
    levelInSpinBox->setSuffix(" dB");

    auto levelInControlLayout = new QHBoxLayout();
    levelInControlLayout->addWidget(levelInSpinBox);
    levelInControlLayout->addStretch();
    
    auto levelInMinLabel = new QLabel("-60");
    levelInControlLayout->addWidget(levelInMinLabel);

    levelInSlider = new QSlider(Qt::Horizontal);
    levelInSlider->setMinimum(-600);
    levelInSlider->setMaximum(360);
    levelInSlider->setValue(static_cast<int>(levelInDb * 10));
    levelInControlLayout->addWidget(levelInSlider, 1);

    auto levelInMaxLabel = new QLabel("+36");
    levelInControlLayout->addWidget(levelInMaxLabel);

    levelInLayout->addLayout(levelInControlLayout);
    mainLayout->addWidget(levelInGroup);

    // Output Level parameter (displayed in dB)
    auto levelOutGroup = new QGroupBox("Output Level (dB)");
    auto levelOutLayout = new QVBoxLayout(levelOutGroup);

    double levelOutDb = linearToDb(m_levelOut);
    levelOutSpinBox = new QDoubleSpinBox();
    levelOutSpinBox->setMinimum(-60.0);
    levelOutSpinBox->setMaximum(36.0);
    levelOutSpinBox->setValue(levelOutDb);
    levelOutSpinBox->setDecimals(1);
    levelOutSpinBox->setSingleStep(0.5);
    levelOutSpinBox->setSuffix(" dB");

    auto levelOutControlLayout = new QHBoxLayout();
    levelOutControlLayout->addWidget(levelOutSpinBox);
    levelOutControlLayout->addStretch();
    
    auto levelOutMinLabel = new QLabel("-60");
    levelOutControlLayout->addWidget(levelOutMinLabel);

    levelOutSlider = new QSlider(Qt::Horizontal);
    levelOutSlider->setMinimum(-600);
    levelOutSlider->setMaximum(360);
    levelOutSlider->setValue(static_cast<int>(levelOutDb * 10));
    levelOutControlLayout->addWidget(levelOutSlider, 1);

    auto levelOutMaxLabel = new QLabel("+36");
    levelOutControlLayout->addWidget(levelOutMaxLabel);

    levelOutLayout->addLayout(levelOutControlLayout);
    mainLayout->addWidget(levelOutGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Mode
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_mode = index;
        updateFFmpegFlags();
    });

    // Type
    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_type = index;
        updateFFmpegFlags();
    });

    // Input Level (dB to linear conversion)
    connect(levelInSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_levelIn = dbToLinear(db);
        levelInSpinBox->blockSignals(true);
        levelInSpinBox->setValue(db);
        levelInSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(levelInSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_levelIn = dbToLinear(db);
        levelInSlider->blockSignals(true);
        levelInSlider->setValue(static_cast<int>(db * 10));
        levelInSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Output Level (dB to linear conversion)
    connect(levelOutSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = value / 10.0;
        m_levelOut = dbToLinear(db);
        levelOutSpinBox->blockSignals(true);
        levelOutSpinBox->setValue(db);
        levelOutSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(levelOutSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_levelOut = dbToLinear(db);
        levelOutSlider->blockSignals(true);
        levelOutSlider->setValue(static_cast<int>(db * 10));
        levelOutSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAemphasis::updateFFmpegFlags() {
    QStringList modeNames = {"reproduction", "production"};
    QStringList typeNames = {"cd", "50fm", "75fm", "50kf", "75kf", "riaa", "emi", "emi2", "col"};
    
    QString modeName = (m_mode >= 0 && m_mode < modeNames.size()) ? modeNames[m_mode] : "reproduction";
    QString typeName = (m_type >= 0 && m_type < typeNames.size()) ? typeNames[m_type] : "cd";
    
    ffmpegFlags = QString("aemphasis=level_in=%1:level_out=%2:mode=%3:type=%4")
                      .arg(m_levelIn, 0, 'f', 6)
                      .arg(m_levelOut, 0, 'f', 6)
                      .arg(modeName)
                      .arg(typeName);
}

QString FFAemphasis::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAemphasis::toJSON(QJsonObject& json) const {
    json["type"] = "ff-aemphasis";
    json["level_in"] = m_levelIn;
    json["level_out"] = m_levelOut;
    json["mode"] = m_mode;
    json["emphasis_type"] = m_type;
}

void FFAemphasis::fromJSON(const QJsonObject& json) {
    m_levelIn = json["level_in"].toDouble(1.0);
    m_levelOut = json["level_out"].toDouble(1.0);
    m_mode = json["mode"].toInt(0);
    m_type = json["emphasis_type"].toInt(0);
    updateFFmpegFlags();
}
