#include "ff-adecorrelate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAdecorrelate::FFAdecorrelate() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdecorrelate::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "Decorrelates stereo channels to create a wider stereo image.\n"
        "Uses allpass filters to introduce phase differences between channels."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Stages parameter (1 - 16)
    auto stagesGroup = new QGroupBox("Filter Stages");
    auto stagesLayout = new QVBoxLayout(stagesGroup);

    stagesSpinBox = new QSpinBox();
    stagesSpinBox->setMinimum(1);
    stagesSpinBox->setMaximum(16);
    stagesSpinBox->setValue(m_stages);

    auto stagesControlLayout = new QHBoxLayout();
    stagesControlLayout->addWidget(stagesSpinBox);
    stagesControlLayout->addStretch();
    
    auto stagesMinLabel = new QLabel("1");
    stagesControlLayout->addWidget(stagesMinLabel);

    stagesSlider = new QSlider(Qt::Horizontal);
    stagesSlider->setMinimum(1);
    stagesSlider->setMaximum(16);
    stagesSlider->setValue(m_stages);
    stagesControlLayout->addWidget(stagesSlider, 1);

    auto stagesMaxLabel = new QLabel("16");
    stagesControlLayout->addWidget(stagesMaxLabel);

    auto stagesHint = new QLabel("More stages = more decorrelation (wider stereo)");
    stagesHint->setStyleSheet("color: gray; font-size: 11px;");
    stagesLayout->addLayout(stagesControlLayout);
    stagesLayout->addWidget(stagesHint);
    mainLayout->addWidget(stagesGroup);

    // Seed parameter
    auto seedGroup = new QGroupBox("Random Seed");
    auto seedLayout = new QVBoxLayout(seedGroup);

    seedSpinBox = new QSpinBox();
    seedSpinBox->setMinimum(-1);
    seedSpinBox->setMaximum(999999);
    seedSpinBox->setValue(m_seed);
    seedSpinBox->setSpecialValueText("Random");

    auto seedHint = new QLabel("-1 = random seed each time, or set fixed value for reproducibility");
    seedHint->setStyleSheet("color: gray; font-size: 11px;");
    seedLayout->addWidget(seedSpinBox);
    seedLayout->addWidget(seedHint);
    mainLayout->addWidget(seedGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Stages
    connect(stagesSlider, &QSlider::valueChanged, [this](int value) {
        m_stages = value;
        stagesSpinBox->blockSignals(true);
        stagesSpinBox->setValue(m_stages);
        stagesSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(stagesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_stages = value;
        stagesSlider->blockSignals(true);
        stagesSlider->setValue(value);
        stagesSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Seed
    connect(seedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_seed = value;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdecorrelate::updateFFmpegFlags() {
    ffmpegFlags = QString("adecorrelate=stages=%1:seed=%2")
                      .arg(m_stages)
                      .arg(m_seed);
}

QString FFAdecorrelate::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdecorrelate::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adecorrelate";
    json["stages"] = m_stages;
    json["seed"] = m_seed;
}

void FFAdecorrelate::fromJSON(const QJsonObject& json) {
    m_stages = json["stages"].toInt(6);
    m_seed = json["seed"].toInt(-1);
    updateFFmpegFlags();
}
