#include "ff-adrc.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFAdrc::FFAdrc() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAdrc::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Transfer function expression
    auto transferGroup = new QGroupBox("Transfer Function");
    auto transferLayout = new QVBoxLayout(transferGroup);

    transferEdit = new QLineEdit();
    transferEdit->setText(m_transfer);
    transferEdit->setPlaceholderText("e.g. p, p^2, if(gt(p,0.5),p,p*2)");

    auto transferHint = new QLabel(
        "FFmpeg expression using 'p' (power), 'f' (freq Hz), 'ch' (channel).\n"
        "Examples: 'p' = passthrough, 'p^0.5' = expand, 'p^2' = compress"
    );
    transferHint->setStyleSheet("color: gray; font-size: 11px;");
    transferHint->setWordWrap(true);

    transferLayout->addWidget(transferEdit);
    transferLayout->addWidget(transferHint);
    mainLayout->addWidget(transferGroup);

    // Attack parameter (1 - 1000 ms)
    auto attackGroup = new QGroupBox("Attack (ms)");
    auto attackLayout = new QVBoxLayout(attackGroup);

    attackSpinBox = new QDoubleSpinBox();
    attackSpinBox->setMinimum(1.0);
    attackSpinBox->setMaximum(1000.0);
    attackSpinBox->setValue(m_attack);
    attackSpinBox->setDecimals(1);
    attackSpinBox->setSingleStep(5.0);
    attackSpinBox->setSuffix(" ms");

    auto attackControlLayout = new QHBoxLayout();
    attackControlLayout->addWidget(attackSpinBox);
    attackControlLayout->addStretch();
    
    auto attackMinLabel = new QLabel("1");
    attackControlLayout->addWidget(attackMinLabel);

    attackSlider = new QSlider(Qt::Horizontal);
    attackSlider->setMinimum(10);
    attackSlider->setMaximum(10000);
    attackSlider->setValue(static_cast<int>(m_attack * 10));
    attackControlLayout->addWidget(attackSlider, 1);

    auto attackMaxLabel = new QLabel("1000");
    attackControlLayout->addWidget(attackMaxLabel);

    auto attackHint = new QLabel("How quickly gain changes respond to increases");
    attackHint->setStyleSheet("color: gray; font-size: 11px;");
    attackLayout->addLayout(attackControlLayout);
    attackLayout->addWidget(attackHint);
    mainLayout->addWidget(attackGroup);

    // Release parameter (5 - 2000 ms)
    auto releaseGroup = new QGroupBox("Release (ms)");
    auto releaseLayout = new QVBoxLayout(releaseGroup);

    releaseSpinBox = new QDoubleSpinBox();
    releaseSpinBox->setMinimum(5.0);
    releaseSpinBox->setMaximum(2000.0);
    releaseSpinBox->setValue(m_release);
    releaseSpinBox->setDecimals(1);
    releaseSpinBox->setSingleStep(10.0);
    releaseSpinBox->setSuffix(" ms");

    auto releaseControlLayout = new QHBoxLayout();
    releaseControlLayout->addWidget(releaseSpinBox);
    releaseControlLayout->addStretch();
    
    auto releaseMinLabel = new QLabel("5");
    releaseControlLayout->addWidget(releaseMinLabel);

    releaseSlider = new QSlider(Qt::Horizontal);
    releaseSlider->setMinimum(50);
    releaseSlider->setMaximum(20000);
    releaseSlider->setValue(static_cast<int>(m_release * 10));
    releaseControlLayout->addWidget(releaseSlider, 1);

    auto releaseMaxLabel = new QLabel("2000");
    releaseControlLayout->addWidget(releaseMaxLabel);

    auto releaseHint = new QLabel("How quickly gain changes recover after decreases");
    releaseHint->setStyleSheet("color: gray; font-size: 11px;");
    releaseLayout->addLayout(releaseControlLayout);
    releaseLayout->addWidget(releaseHint);
    mainLayout->addWidget(releaseGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Transfer function
    connect(transferEdit, &QLineEdit::textChanged, [this](const QString& text) {
        m_transfer = text;
        updateFFmpegFlags();
    });

    // Attack
    connect(attackSlider, &QSlider::valueChanged, [this](int value) {
        m_attack = value / 10.0;
        attackSpinBox->blockSignals(true);
        attackSpinBox->setValue(m_attack);
        attackSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(attackSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_attack = value;
        attackSlider->blockSignals(true);
        attackSlider->setValue(static_cast<int>(value * 10));
        attackSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Release
    connect(releaseSlider, &QSlider::valueChanged, [this](int value) {
        m_release = value / 10.0;
        releaseSpinBox->blockSignals(true);
        releaseSpinBox->setValue(m_release);
        releaseSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(releaseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_release = value;
        releaseSlider->blockSignals(true);
        releaseSlider->setValue(static_cast<int>(value * 10));
        releaseSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAdrc::updateFFmpegFlags() {
    // Escape single quotes in transfer expression for FFmpeg
    QString escapedTransfer = m_transfer;
    escapedTransfer.replace("'", "\\'");
    
    ffmpegFlags = QString("adrc=transfer='%1':attack=%2:release=%3")
                      .arg(escapedTransfer)
                      .arg(m_attack, 0, 'f', 1)
                      .arg(m_release, 0, 'f', 1);
}

QString FFAdrc::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAdrc::toJSON(QJsonObject& json) const {
    json["type"] = "ff-adrc";
    json["transfer"] = m_transfer;
    json["attack"] = m_attack;
    json["release"] = m_release;
}

void FFAdrc::fromJSON(const QJsonObject& json) {
    m_transfer = json["transfer"].toString("p");
    m_attack = json["attack"].toDouble(50.0);
    m_release = json["release"].toDouble(100.0);
    updateFFmpegFlags();
}
