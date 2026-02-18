#include "ff-axcorrelate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <cmath>

FFAxcorrelate::FFAxcorrelate() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAxcorrelate::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Multi-input warning
    auto warningGroup = new QGroupBox("⚠️ Multi-Input Filter");
    auto warningLayout = new QVBoxLayout(warningGroup);
    warningGroup->setStyleSheet("QGroupBox { color: #ff9944; font-weight: bold; }");

    auto warningLabel = new QLabel(
        "This filter requires TWO audio inputs:\n"
        "• [0:a] Main input (first signal)\n"
        "• [1:a] Audio Input (second signal)\n\n"
        "Place an Audio Input filter before this filter in the chain.\n"
        "Output is the cross-correlation signal (not a pass-through)."
    );
    warningLabel->setWordWrap(true);
    warningLayout->addWidget(warningLabel);

    mainLayout->addWidget(warningGroup);

    // Info group
    auto infoGroup = new QGroupBox("Audio Cross-Correlation");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Computes the cross-correlation between two audio streams. "
        "Cross-correlation measures the similarity of two signals as a "
        "function of time lag, useful for finding delays or detecting patterns."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Segment size parameter
    auto sizeGroup = new QGroupBox("Segment Size (samples)");
    auto sizeLayout = new QVBoxLayout(sizeGroup);

    sizeSpinBox = new QSpinBox();
    sizeSpinBox->setMinimum(2);
    sizeSpinBox->setMaximum(131072);
    sizeSpinBox->setValue(m_size);

    auto sizeControlLayout = new QHBoxLayout();
    sizeControlLayout->addWidget(sizeSpinBox);
    sizeControlLayout->addStretch();
    
    auto sizeMinLabel = new QLabel("2");
    sizeControlLayout->addWidget(sizeMinLabel);

    // Log scale slider for size (powers of 2)
    sizeSlider = new QSlider(Qt::Horizontal);
    sizeSlider->setMinimum(1);  // 2^1 = 2
    sizeSlider->setMaximum(17); // 2^17 = 131072
    sizeSlider->setValue(static_cast<int>(std::log2(m_size)));
    sizeControlLayout->addWidget(sizeSlider, 1);

    auto sizeMaxLabel = new QLabel("131072");
    sizeControlLayout->addWidget(sizeMaxLabel);

    sizeLayout->addLayout(sizeControlLayout);

    auto sizeHint = new QLabel(
        "Size of the segment used for correlation. Larger segments provide "
        "better frequency resolution but increase latency and computation."
    );
    sizeHint->setStyleSheet("color: gray; font-size: 11px;");
    sizeHint->setWordWrap(true);
    sizeLayout->addWidget(sizeHint);

    mainLayout->addWidget(sizeGroup);

    // Algorithm parameter
    auto algoGroup = new QGroupBox("Algorithm");
    auto algoLayout = new QVBoxLayout(algoGroup);

    algoCombo = new QComboBox();
    algoCombo->addItem("Slow (Direct)", 0);
    algoCombo->addItem("Fast (FFT-based)", 1);
    algoCombo->addItem("Best (Auto-select)", 2);
    algoCombo->setCurrentIndex(m_algo);
    algoLayout->addWidget(algoCombo);

    auto algoHint = new QLabel(
        "• Slow: Direct computation, accurate but O(n²)\n"
        "• Fast: FFT-based, faster for larger segments O(n log n)\n"
        "• Best: Automatically chooses based on segment size"
    );
    algoHint->setStyleSheet("color: gray; font-size: 11px;");
    algoHint->setWordWrap(true);
    algoLayout->addWidget(algoHint);

    mainLayout->addWidget(algoGroup);

    // Use cases group
    auto usesGroup = new QGroupBox("Use Cases");
    auto usesLayout = new QVBoxLayout(usesGroup);

    auto usesLabel = new QLabel(
        "• Finding time delay between two recordings\n"
        "• Audio fingerprinting and matching\n"
        "• Echo detection and measurement\n"
        "• Phase alignment analysis\n"
        "• Template matching in audio"
    );
    usesLabel->setWordWrap(true);
    usesLayout->addWidget(usesLabel);

    mainLayout->addWidget(usesGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Size (log scale slider)
    connect(sizeSlider, &QSlider::valueChanged, [this](int value) {
        m_size = 1 << value;  // 2^value
        sizeSpinBox->blockSignals(true);
        sizeSpinBox->setValue(m_size);
        sizeSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });

    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_size = value;
        sizeSlider->blockSignals(true);
        sizeSlider->setValue(static_cast<int>(std::log2(value)));
        sizeSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Algorithm
    connect(algoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_algo = index;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAxcorrelate::updateFFmpegFlags() {
    QStringList algoNames = {"slow", "fast", "best"};
    QString algoName = (m_algo >= 0 && m_algo < algoNames.size()) ? algoNames[m_algo] : "best";
    
    ffmpegFlags = QString("axcorrelate=size=%1:algo=%2")
                      .arg(m_size)
                      .arg(algoName);
}

QString FFAxcorrelate::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAxcorrelate::toJSON(QJsonObject& json) const {
    json["type"] = "ff-axcorrelate";
    json["size"] = m_size;
    json["algo"] = m_algo;
}

void FFAxcorrelate::fromJSON(const QJsonObject& json) {
    m_size = json["size"].toInt(256);
    m_algo = json["algo"].toInt(2);
    updateFFmpegFlags();
}
