#include "ff-bs2b.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>

FFBs2b::FFBs2b() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFBs2b::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info label
    auto infoLabel = new QLabel(
        "BS2B (Bauer Stereophonic-to-Binaural) improves headphone listening\n"
        "by simulating speaker crossfeed. Requires libbs2b in FFmpeg build."
    );
    infoLabel->setStyleSheet("color: gray; font-size: 11px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // Profile preset combo
    auto profileGroup = new QGroupBox("Profile Preset");
    auto profileLayout = new QVBoxLayout(profileGroup);

    profileCombo = new QComboBox();
    profileCombo->addItem("Default (700 Hz, 4.5 dB)", 0);
    profileCombo->addItem("C-Moy (700 Hz, 6.0 dB)", 1);
    profileCombo->addItem("Jan Meier (650 Hz, 9.5 dB)", 2);
    profileCombo->setCurrentIndex(m_profile);

    auto profileHint = new QLabel("Select a preset or customize below");
    profileHint->setStyleSheet("color: gray; font-size: 11px;");
    profileLayout->addWidget(profileCombo);
    profileLayout->addWidget(profileHint);
    mainLayout->addWidget(profileGroup);

    // Cutoff frequency parameter (300 - 2000 Hz)
    auto fcutGroup = new QGroupBox("Crossfeed Cutoff Frequency (Hz)");
    auto fcutLayout = new QVBoxLayout(fcutGroup);

    fcutSpinBox = new QSpinBox();
    fcutSpinBox->setMinimum(300);
    fcutSpinBox->setMaximum(2000);
    fcutSpinBox->setValue(m_fcut);
    fcutSpinBox->setSingleStep(10);
    fcutSpinBox->setSuffix(" Hz");

    auto fcutControlLayout = new QHBoxLayout();
    fcutControlLayout->addWidget(fcutSpinBox);
    fcutControlLayout->addStretch();
    
    auto fcutMinLabel = new QLabel("300");
    fcutControlLayout->addWidget(fcutMinLabel);

    fcutSlider = new QSlider(Qt::Horizontal);
    fcutSlider->setMinimum(300);
    fcutSlider->setMaximum(2000);
    fcutSlider->setValue(m_fcut);
    fcutControlLayout->addWidget(fcutSlider, 1);

    auto fcutMaxLabel = new QLabel("2000");
    fcutControlLayout->addWidget(fcutMaxLabel);

    auto fcutHint = new QLabel("Low frequencies below this are crossfed between channels");
    fcutHint->setStyleSheet("color: gray; font-size: 11px;");
    fcutLayout->addLayout(fcutControlLayout);
    fcutLayout->addWidget(fcutHint);
    mainLayout->addWidget(fcutGroup);

    // Feed level parameter (10 - 150 representing 1.0 - 15.0 dB)
    auto feedGroup = new QGroupBox("Crossfeed Level (dB × 10)");
    auto feedLayout = new QVBoxLayout(feedGroup);

    feedSpinBox = new QSpinBox();
    feedSpinBox->setMinimum(10);
    feedSpinBox->setMaximum(150);
    feedSpinBox->setValue(m_feed);
    feedSpinBox->setSingleStep(5);

    auto feedControlLayout = new QHBoxLayout();
    feedControlLayout->addWidget(feedSpinBox);
    feedControlLayout->addStretch();
    
    auto feedMinLabel = new QLabel("1.0 dB");
    feedControlLayout->addWidget(feedMinLabel);

    feedSlider = new QSlider(Qt::Horizontal);
    feedSlider->setMinimum(10);
    feedSlider->setMaximum(150);
    feedSlider->setValue(m_feed);
    feedControlLayout->addWidget(feedSlider, 1);

    auto feedMaxLabel = new QLabel("15.0 dB");
    feedControlLayout->addWidget(feedMaxLabel);

    auto feedHint = new QLabel("Amount of crossfeed (45 = 4.5 dB). Higher = more blending");
    feedHint->setStyleSheet("color: gray; font-size: 11px;");
    feedLayout->addLayout(feedControlLayout);
    feedLayout->addWidget(feedHint);
    mainLayout->addWidget(feedGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Profile combo - updates fcut and feed based on preset
    connect(profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        m_profile = index;
        
        // Update fcut/feed based on preset
        switch (index) {
            case 0:  // Default
                m_fcut = 700;
                m_feed = 45;
                break;
            case 1:  // C-Moy
                m_fcut = 700;
                m_feed = 60;
                break;
            case 2:  // Jan Meier
                m_fcut = 650;
                m_feed = 95;
                break;
        }
        
        // Update UI
        fcutSpinBox->blockSignals(true);
        fcutSlider->blockSignals(true);
        feedSpinBox->blockSignals(true);
        feedSlider->blockSignals(true);
        
        fcutSpinBox->setValue(m_fcut);
        fcutSlider->setValue(m_fcut);
        feedSpinBox->setValue(m_feed);
        feedSlider->setValue(m_feed);
        
        fcutSpinBox->blockSignals(false);
        fcutSlider->blockSignals(false);
        feedSpinBox->blockSignals(false);
        feedSlider->blockSignals(false);
        
        updateFFmpegFlags();
    });

    // Cutoff frequency
    connect(fcutSlider, &QSlider::valueChanged, [this](int value) {
        m_fcut = value;
        fcutSpinBox->blockSignals(true);
        fcutSpinBox->setValue(m_fcut);
        fcutSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(fcutSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_fcut = value;
        fcutSlider->blockSignals(true);
        fcutSlider->setValue(value);
        fcutSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    // Feed level
    connect(feedSlider, &QSlider::valueChanged, [this](int value) {
        m_feed = value;
        feedSpinBox->blockSignals(true);
        feedSpinBox->setValue(m_feed);
        feedSpinBox->blockSignals(false);
        updateFFmpegFlags();
    });
    connect(feedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        m_feed = value;
        feedSlider->blockSignals(true);
        feedSlider->setValue(value);
        feedSlider->blockSignals(false);
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFBs2b::updateFFmpegFlags() {
    QStringList profileNames = {"default", "cmoy", "jmeier"};
    QString profileName = (m_profile >= 0 && m_profile < profileNames.size()) ? profileNames[m_profile] : "default";
    
    ffmpegFlags = QString("bs2b=profile=%1:fcut=%2:feed=%3")
                      .arg(profileName)
                      .arg(m_fcut)
                      .arg(m_feed);
}

QString FFBs2b::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFBs2b::toJSON(QJsonObject& json) const {
    json["type"] = "ff-bs2b";
    json["profile"] = m_profile;
    json["fcut"] = m_fcut;
    json["feed"] = m_feed;
}

void FFBs2b::fromJSON(const QJsonObject& json) {
    m_profile = json["profile"].toInt(0);
    m_fcut = json["fcut"].toInt(700);
    m_feed = json["feed"].toInt(45);
    updateFFmpegFlags();
}
