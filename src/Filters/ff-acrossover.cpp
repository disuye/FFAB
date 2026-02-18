#include "ff-acrossover.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAcrossover::FFAcrossover() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcrossover::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Convert linear gain to dB for display
    auto linearToDb = [](double linear) -> double {
        if (linear <= 0.0) return -96.0;
        return 20.0 * std::log10(linear);
    };
    auto dbToLinear = [](double db) -> double {
        return std::pow(10.0, db / 20.0);
    };

    // Multi-output warning
    auto warningGroup = new QGroupBox("⚠️ Multi-Output Filter");
    auto warningLayout = new QVBoxLayout(warningGroup);
    warningGroup->setStyleSheet("QGroupBox { color: #ff9944; font-weight: bold; }");

    auto warningLabel = new QLabel(
        "This filter produces MULTIPLE output streams!\n\n"
        "For N crossover frequencies, you get N+1 output bands:\n"
        "• 1 frequency (e.g., 500Hz) → 2 outputs (low, high)\n"
        "• 2 frequencies (e.g., 200|2000) → 3 outputs (low, mid, high)\n\n"
        "Each output needs to be routed appropriately in the filter chain."
    );
    warningLabel->setWordWrap(true);
    warningLayout->addWidget(warningLabel);

    mainLayout->addWidget(warningGroup);

    // Split frequencies
    auto splitGroup = new QGroupBox("Crossover Frequencies (Hz)");
    auto splitLayout = new QVBoxLayout(splitGroup);

    splitEdit = new QLineEdit();
    splitEdit->setPlaceholderText("500 or 200 2000 or 80|800|5000");
    splitEdit->setText(m_split);
    splitLayout->addWidget(splitEdit);

    auto splitHint = new QLabel(
        "Enter frequencies separated by spaces or | characters.\n"
        "Examples:\n"
        "  500 — 2-way split (sub 500Hz, above 500Hz)\n"
        "  200 2000 — 3-way (low, mid, high)\n"
        "  80 500 2000 8000 — 5-way"
    );
    splitHint->setStyleSheet("color: gray; font-size: 11px;");
    splitHint->setWordWrap(true);
    splitLayout->addWidget(splitHint);

    mainLayout->addWidget(splitGroup);

    // Filter order
    auto orderGroup = new QGroupBox("Filter Order");
    auto orderLayout = new QVBoxLayout(orderGroup);

    orderCombo = new QComboBox();
    orderCombo->addItem("2nd order (12 dB/oct)", 0);
    orderCombo->addItem("4th order (24 dB/oct)", 1);
    orderCombo->addItem("6th order (36 dB/oct)", 2);
    orderCombo->addItem("8th order (48 dB/oct)", 3);
    orderCombo->addItem("10th order (60 dB/oct)", 4);
    orderCombo->addItem("12th order (72 dB/oct)", 5);
    orderCombo->addItem("14th order (84 dB/oct)", 6);
    orderCombo->addItem("16th order (96 dB/oct)", 7);
    orderCombo->addItem("18th order (108 dB/oct)", 8);
    orderCombo->addItem("20th order (120 dB/oct)", 9);
    orderCombo->setCurrentIndex(m_order);
    orderLayout->addWidget(orderCombo);

    auto orderHint = new QLabel(
        "Higher orders give steeper crossover slopes. "
        "Uses Linkwitz-Riley alignment for flat summed response."
    );
    orderHint->setStyleSheet("color: gray; font-size: 11px;");
    orderHint->setWordWrap(true);
    orderLayout->addWidget(orderHint);

    mainLayout->addWidget(orderGroup);

    // Input level (-96 to 0 dB)
    auto levelGroup = new QGroupBox("Input Level (dB)");
    auto levelLayout = new QVBoxLayout(levelGroup);

    levelSpinBox = new QDoubleSpinBox();
    levelSpinBox->setMinimum(-96.0);
    levelSpinBox->setMaximum(0.0);
    levelSpinBox->setValue(linearToDb(m_level));
    levelSpinBox->setDecimals(1);
    levelSpinBox->setSingleStep(0.1);
    levelSpinBox->setSuffix(" dB");

    auto levelControlLayout = new QHBoxLayout();
    levelControlLayout->addWidget(levelSpinBox);
    levelControlLayout->addWidget(new QLabel("-96"));
    levelSlider = new QSlider(Qt::Horizontal);
    levelSlider->setMinimum(0);
    levelSlider->setMaximum(960);
    levelSlider->setValue(static_cast<int>((linearToDb(m_level) + 96.0) * 10.0));
    levelControlLayout->addWidget(levelSlider, 1);
    levelControlLayout->addWidget(new QLabel("0"));
    levelLayout->addLayout(levelControlLayout);

    mainLayout->addWidget(levelGroup);

    // Output band gains (optional)
    auto gainGroup = new QGroupBox("Output Band Gains (Optional)");
    auto gainLayout = new QVBoxLayout(gainGroup);

    gainEdit = new QLineEdit();
    gainEdit->setPlaceholderText("1.0 1.0 1.0 (one per band)");
    gainEdit->setText(m_gain);
    gainLayout->addWidget(gainEdit);

    auto gainHint = new QLabel(
        "Gain for each output band, separated by spaces. "
        "Use 'f' suffix for float values (e.g., 1.0f 0.5f 1.0f)."
    );
    gainHint->setStyleSheet("color: gray; font-size: 11px;");
    gainHint->setWordWrap(true);
    gainLayout->addWidget(gainHint);

    mainLayout->addWidget(gainGroup);

    // Precision
    auto precisionGroup = new QGroupBox("Processing Precision");
    auto precisionLayout = new QVBoxLayout(precisionGroup);

    precisionCombo = new QComboBox();
    precisionCombo->addItem("Auto", 0);
    precisionCombo->addItem("Float (32-bit)", 1);
    precisionCombo->addItem("Double (64-bit)", 2);
    precisionCombo->setCurrentIndex(m_precision);
    precisionLayout->addWidget(precisionCombo);

    mainLayout->addWidget(precisionGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Split frequencies
    connect(splitEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_split = text.trimmed();
        updateFFmpegFlags();
    });

    // Order
    connect(orderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_order = index;
        updateFFmpegFlags();
    });

    // Level (dB conversion)
    connect(levelSlider, &QSlider::valueChanged, [this, dbToLinear](int value) {
        double db = (value / 10.0) - 96.0;
        m_level = dbToLinear(db);
        levelSpinBox->blockSignals(true);
        levelSpinBox->setValue(db);
        levelSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(levelSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, dbToLinear](double db) {
        m_level = dbToLinear(db);
        levelSlider->blockSignals(true);
        levelSlider->setValue(static_cast<int>((db + 96.0) * 10.0));
        levelSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Gain
    connect(gainEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_gain = text.trimmed();
        updateFFmpegFlags();
    });

    // Precision
    connect(precisionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_precision = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAcrossover::updateFFmpegFlags() {
    QStringList orderNames = {"2nd", "4th", "6th", "8th", "10th", "12th", "14th", "16th", "18th", "20th"};
    QStringList precisionNames = {"auto", "float", "double"};
    
    QString orderName = (m_order >= 0 && m_order < orderNames.size()) ? orderNames[m_order] : "4th";
    QString precisionName = (m_precision >= 0 && m_precision < precisionNames.size()) ? precisionNames[m_precision] : "auto";
    
    // Replace spaces with | in split frequencies
    QString splitFormatted = m_split;
    splitFormatted.replace(" ", " ");  // Normalize whitespace
    
    ffmpegFlags = QString("acrossover=split='%1':order=%2:level=%3:gain='%4':precision=%5")
                      .arg(splitFormatted)
                      .arg(orderName)
                      .arg(m_level, 0, 'f', 2)
                      .arg(m_gain)
                      .arg(precisionName);
}

QString FFAcrossover::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcrossover::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acrossover";
    json["split"] = m_split;
    json["order"] = m_order;
    json["level"] = m_level;
    json["gain"] = m_gain;
    json["precision"] = m_precision;
}

void FFAcrossover::fromJSON(const QJsonObject& json) {
    m_split = json["split"].toString("500");
    m_order = json["order"].toInt(4);
    m_level = json["level"].toDouble(1.0);
    m_gain = json["gain"].toString("1.f");
    m_precision = json["precision"].toInt(0);
    updateFFmpegFlags();
}
