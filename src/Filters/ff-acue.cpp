#include "ff-acue.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QGroupBox>
#include <QLineEdit>

FFAcue::FFAcue() {
    position = Position::MIDDLE;
    updateFFmpegFlags();
}

QWidget* FFAcue::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    parametersWidget->setObjectName("filterParamsPaneTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Info group
    auto infoGroup = new QGroupBox("Audio Cue Synchronization");
    auto infoLayout = new QVBoxLayout(infoGroup);

    auto descLabel = new QLabel(
        "Delays audio output until the system clock matches the specified "
        "cue timestamp. Used for synchronized playback with external events "
        "or other media streams."
    );
    descLabel->setWordWrap(true);
    infoLayout->addWidget(descLabel);

    mainLayout->addWidget(infoGroup);

    // Cue timestamp parameter
    auto cueGroup = new QGroupBox("Cue Timestamp");
    auto cueLayout = new QVBoxLayout(cueGroup);

    auto cueInputLayout = new QHBoxLayout();
    auto cueEdit = new QLineEdit();
    cueEdit->setText(QString::number(m_cue));
    cueEdit->setPlaceholderText("Unix timestamp in microseconds");
    cueInputLayout->addWidget(cueEdit);
    cueInputLayout->addWidget(new QLabel("µs"));

    cueLayout->addLayout(cueInputLayout);

    auto cueHint = new QLabel(
        "Unix timestamp in microseconds when playback should begin. "
        "Use 0 to start immediately."
    );
    cueHint->setStyleSheet("color: gray; font-size: 11px;");
    cueHint->setWordWrap(true);
    cueLayout->addWidget(cueHint);

    mainLayout->addWidget(cueGroup);

    // Preroll parameter
    auto prerollGroup = new QGroupBox("Preroll Duration");
    auto prerollLayout = new QVBoxLayout(prerollGroup);

    prerollSpinBox = new QDoubleSpinBox();
    prerollSpinBox->setMinimum(0.0);
    prerollSpinBox->setMaximum(60.0);
    prerollSpinBox->setValue(m_preroll);
    prerollSpinBox->setDecimals(3);
    prerollSpinBox->setSingleStep(0.1);
    prerollSpinBox->setSuffix(" s");

    prerollLayout->addWidget(prerollSpinBox);

    auto prerollHint = new QLabel(
        "How long before the cue point to start processing. "
        "Allows time for filter initialization and buffering."
    );
    prerollHint->setStyleSheet("color: gray; font-size: 11px;");
    prerollHint->setWordWrap(true);
    prerollLayout->addWidget(prerollHint);

    mainLayout->addWidget(prerollGroup);

    // Buffer parameter
    auto bufferGroup = new QGroupBox("Buffer Duration");
    auto bufferLayout = new QVBoxLayout(bufferGroup);

    bufferSpinBox = new QDoubleSpinBox();
    bufferSpinBox->setMinimum(0.0);
    bufferSpinBox->setMaximum(60.0);
    bufferSpinBox->setValue(m_buffer);
    bufferSpinBox->setDecimals(3);
    bufferSpinBox->setSingleStep(0.1);
    bufferSpinBox->setSuffix(" s");

    bufferLayout->addWidget(bufferSpinBox);

    auto bufferHint = new QLabel(
        "Amount of audio to buffer before the cue point. "
        "Helps ensure smooth playback at the cue time."
    );
    bufferHint->setStyleSheet("color: gray; font-size: 11px;");
    bufferHint->setWordWrap(true);
    bufferLayout->addWidget(bufferHint);

    mainLayout->addWidget(bufferGroup);

    // Usage notes
    auto notesGroup = new QGroupBox("Usage Notes");
    auto notesLayout = new QVBoxLayout(notesGroup);

    auto notesLabel = new QLabel(
        "This filter is typically used for:\n\n"
        "• Synchronized multi-track playback\n"
        "• Starting playback at a specific wall-clock time\n"
        "• Coordinating audio with video or external events\n"
        "• Live broadcast applications"
    );
    notesLabel->setWordWrap(true);
    notesLayout->addWidget(notesLabel);

    mainLayout->addWidget(notesGroup);

    mainLayout->addStretch();

    // ========== SIGNAL CONNECTIONS ==========

    // Cue timestamp (using QLineEdit for large numbers)
    connect(cueEdit, &QLineEdit::textChanged, [this](const QString& text) {
        bool ok;
        qint64 value = text.toLongLong(&ok);
        if (ok) {
            m_cue = value;
            updateFFmpegFlags();
        }
    });

    // Preroll
    connect(prerollSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_preroll = value;
        updateFFmpegFlags();
    });

    // Buffer
    connect(bufferSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        m_buffer = value;
        updateFFmpegFlags();
    });

    return parametersWidget;
}

void FFAcue::updateFFmpegFlags() {
    ffmpegFlags = QString("acue=cue=%1:preroll=%2:buffer=%3")
                      .arg(m_cue)
                      .arg(m_preroll, 0, 'f', 3)
                      .arg(m_buffer, 0, 'f', 3);
}

QString FFAcue::buildFFmpegFlags() const {
    return ffmpegFlags;
}

void FFAcue::toJSON(QJsonObject& json) const {
    json["type"] = "ff-acue";
    json["cue"] = m_cue;
    json["preroll"] = m_preroll;
    json["buffer"] = m_buffer;
}

void FFAcue::fromJSON(const QJsonObject& json) {
    m_cue = json["cue"].toVariant().toLongLong();
    m_preroll = json["preroll"].toDouble(0.0);
    m_buffer = json["buffer"].toDouble(0.0);
    updateFFmpegFlags();
}
