#include "ff-biquad.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QTabWidget>
#include <QGridLayout>

FFBiquad::FFBiquad() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFBiquad::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Create tab widget
    auto tabWidget = new QTabWidget();

    // ========== TAB 1: COEFFICIENTS ==========
    auto coeffTab = new QWidget();
    auto coeffLayout = new QVBoxLayout(coeffTab);

    // Info label
    auto infoLabel = new QLabel(
        "Transfer function: H(z) = (b₀ + b₁z⁻¹ + b₂z⁻²) / (a₀ + a₁z⁻¹ + a₂z⁻²)"
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    coeffLayout->addWidget(infoLabel);

    // Denominator coefficients (a)
    auto denomGroup = new QGroupBox("Denominator (a coefficients)");
    auto denomLayout = new QGridLayout(denomGroup);

    denomLayout->addWidget(new QLabel("a₀:"), 0, 0);
    a0SpinBox = new QDoubleSpinBox();
    a0SpinBox->setRange(-100.0, 100.0);
    a0SpinBox->setValue(m_a0);
    a0SpinBox->setDecimals(6);
    a0SpinBox->setSingleStep(0.01);
    denomLayout->addWidget(a0SpinBox, 0, 1);

    denomLayout->addWidget(new QLabel("a₁:"), 1, 0);
    a1SpinBox = new QDoubleSpinBox();
    a1SpinBox->setRange(-100.0, 100.0);
    a1SpinBox->setValue(m_a1);
    a1SpinBox->setDecimals(6);
    a1SpinBox->setSingleStep(0.01);
    denomLayout->addWidget(a1SpinBox, 1, 1);

    denomLayout->addWidget(new QLabel("a₂:"), 2, 0);
    a2SpinBox = new QDoubleSpinBox();
    a2SpinBox->setRange(-100.0, 100.0);
    a2SpinBox->setValue(m_a2);
    a2SpinBox->setDecimals(6);
    a2SpinBox->setSingleStep(0.01);
    denomLayout->addWidget(a2SpinBox, 2, 1);

    coeffLayout->addWidget(denomGroup);

    // Numerator coefficients (b)
    auto numerGroup = new QGroupBox("Numerator (b coefficients)");
    auto numerLayout = new QGridLayout(numerGroup);

    numerLayout->addWidget(new QLabel("b₀:"), 0, 0);
    b0SpinBox = new QDoubleSpinBox();
    b0SpinBox->setRange(-100.0, 100.0);
    b0SpinBox->setValue(m_b0);
    b0SpinBox->setDecimals(6);
    b0SpinBox->setSingleStep(0.01);
    numerLayout->addWidget(b0SpinBox, 0, 1);

    numerLayout->addWidget(new QLabel("b₁:"), 1, 0);
    b1SpinBox = new QDoubleSpinBox();
    b1SpinBox->setRange(-100.0, 100.0);
    b1SpinBox->setValue(m_b1);
    b1SpinBox->setDecimals(6);
    b1SpinBox->setSingleStep(0.01);
    numerLayout->addWidget(b1SpinBox, 1, 1);

    numerLayout->addWidget(new QLabel("b₂:"), 2, 0);
    b2SpinBox = new QDoubleSpinBox();
    b2SpinBox->setRange(-100.0, 100.0);
    b2SpinBox->setValue(m_b2);
    b2SpinBox->setDecimals(6);
    b2SpinBox->setSingleStep(0.01);
    numerLayout->addWidget(b2SpinBox, 2, 1);

    coeffLayout->addWidget(numerGroup);

    coeffLayout->addStretch();
    tabWidget->addTab(coeffTab, "Coefficients");

    // ========== TAB 2: OPTIONS ==========
    auto optionsTab = new QWidget();
    auto optionsLayout = new QVBoxLayout(optionsTab);

    // Mix (0 - 100%)
    auto mixGroup = new QGroupBox("Mix (Dry/Wet)");
    auto mixLayout = new QVBoxLayout(mixGroup);

    mixSpinBox = new QDoubleSpinBox();
    mixSpinBox->setMinimum(0.0);
    mixSpinBox->setMaximum(100.0);
    mixSpinBox->setValue(m_mix * 100.0);
    mixSpinBox->setDecimals(1);
    mixSpinBox->setSingleStep(5.0);
    mixSpinBox->setSuffix(" %");

    auto mixControlLayout = new QHBoxLayout();
    mixControlLayout->addWidget(mixSpinBox);
    mixControlLayout->addStretch();
    mixControlLayout->addWidget(new QLabel("0%"));
    mixSlider = new QSlider(Qt::Horizontal);
    mixSlider->setMinimum(0);
    mixSlider->setMaximum(100);
    mixSlider->setValue(static_cast<int>(m_mix * 100));
    mixControlLayout->addWidget(mixSlider, 1);
    mixControlLayout->addWidget(new QLabel("100%"));

    mixLayout->addLayout(mixControlLayout);
    optionsLayout->addWidget(mixGroup);

    // Normalize checkbox
    auto normalizeGroup = new QGroupBox("Normalization");
    auto normalizeLayout = new QVBoxLayout(normalizeGroup);

    normalizeCheck = new QCheckBox("Normalize coefficients");
    normalizeCheck->setChecked(m_normalize);
    auto normalizeHint = new QLabel("Normalize a0 to 1.0");
    normalizeHint->setStyleSheet("color: gray; font-size: 11px;");
    normalizeLayout->addWidget(normalizeCheck);
    normalizeLayout->addWidget(normalizeHint);
    optionsLayout->addWidget(normalizeGroup);

    // Transform type
    auto transformGroup = new QGroupBox("Filter Structure");
    auto transformLayout = new QVBoxLayout(transformGroup);

    transformCombo = new QComboBox();
    transformCombo->addItem("Direct Form I", 0);
    transformCombo->addItem("Direct Form II", 1);
    transformCombo->addItem("Transposed Direct Form I", 2);
    transformCombo->addItem("Transposed Direct Form II", 3);
    transformCombo->addItem("Lattice", 4);
    transformCombo->addItem("State Variable Filter", 5);
    transformCombo->addItem("Zero Delay Feedback", 6);
    transformCombo->setCurrentIndex(m_transform);

    auto transformHint = new QLabel("Internal filter implementation structure");
    transformHint->setStyleSheet("color: gray; font-size: 11px;");
    transformLayout->addWidget(transformCombo);
    transformLayout->addWidget(transformHint);
    optionsLayout->addWidget(transformGroup);

    optionsLayout->addStretch();
    tabWidget->addTab(optionsTab, "Options");

    mainLayout->addWidget(tabWidget);

    // ========== SIGNAL CONNECTIONS ==========

    // Coefficients
    connect(a0SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_a0 = value;
        updateFFmpegFlags();
    });
    connect(a1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_a1 = value;
        updateFFmpegFlags();
    });
    connect(a2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_a2 = value;
        updateFFmpegFlags();
    });
    connect(b0SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_b0 = value;
        updateFFmpegFlags();
    });
    connect(b1SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_b1 = value;
        updateFFmpegFlags();
    });
    connect(b2SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_b2 = value;
        updateFFmpegFlags();
    });

    // Mix
    connect(mixSlider, &QSlider::valueChanged, [this](int value) {
        m_mix = value / 100.0;
        mixSpinBox->blockSignals(true);
        mixSpinBox->setValue(value);
        mixSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(mixSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_mix = value / 100.0;
        mixSlider->blockSignals(true);
        mixSlider->setValue(static_cast<int>(value));
        mixSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Normalize
    connect(normalizeCheck, &QCheckBox::toggled, [this](bool checked) {
        m_normalize = checked;
        updateFFmpegFlags();
    });

    // Transform
    connect(transformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_transform = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFBiquad::updateFFmpegFlags() {
    QStringList transformNames = {"di", "dii", "tdi", "tdii", "latt", "svf", "zdf"};
    QString transformName = (m_transform >= 0 && m_transform < transformNames.size()) ? transformNames[m_transform] : "di";
    
    ffmpegFlags = QString("biquad=a0=%1:a1=%2:a2=%3:b0=%4:b1=%5:b2=%6:mix=%7:normalize=%8:transform=%9")
                      .arg(m_a0, 0, 'f', 6)
                      .arg(m_a1, 0, 'f', 6)
                      .arg(m_a2, 0, 'f', 6)
                      .arg(m_b0, 0, 'f', 6)
                      .arg(m_b1, 0, 'f', 6)
                      .arg(m_b2, 0, 'f', 6)
                      .arg(m_mix, 0, 'f', 2)
                      .arg(m_normalize ? "true" : "false")
                      .arg(transformName);
}

QString FFBiquad::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFBiquad::toJSON(QJsonObject& json) const {
    json["type"] = "ff-biquad";
    json["a0"] = m_a0;
    json["a1"] = m_a1;
    json["a2"] = m_a2;
    json["b0"] = m_b0;
    json["b1"] = m_b1;
    json["b2"] = m_b2;
    json["mix"] = m_mix;
    json["normalize"] = m_normalize;
    json["transform"] = m_transform;
}

void FFBiquad::fromJSON(const QJsonObject& json) {
    m_a0 = json["a0"].toDouble(1.0);
    m_a1 = json["a1"].toDouble(0.0);
    m_a2 = json["a2"].toDouble(0.0);
    m_b0 = json["b0"].toDouble(1.0);
    m_b1 = json["b1"].toDouble(0.0);
    m_b2 = json["b2"].toDouble(0.0);
    m_mix = json["mix"].toDouble(1.0);
    m_normalize = json["normalize"].toBool(false);
    m_transform = json["transform"].toInt(0);
    updateFFmpegFlags();
}
