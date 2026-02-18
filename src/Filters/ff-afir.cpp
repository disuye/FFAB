#include "ff-afir.h"
#include "CollapsibleHelpSection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QJsonObject>
#include <QGroupBox>
#include <QFont>
#include <cmath>

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

FFAfir::FFAfir() {
    position = Position::MIDDLE;
}

QWidget* FFAfir::getParametersWidget() {
    if (parametersWidget) return parametersWidget;

    parametersWidget = new QWidget();
    // Target for global styling main.cpp
    // parametersWidget->setObjectName("filterParamsPaneTarget");
    parametersWidget->setObjectName("ffAfirTarget");
    auto mainLayout = new QVBoxLayout(parametersWidget);
    mainLayout->setSpacing(12);

    // Title
    auto titleLabel = new QLabel("Convolution (Impulse Response)");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Convolution Levels section
    auto levelsGroup = new QGroupBox("Convolution Levels");
    auto levelsLayout = new QFormLayout(levelsGroup);

    drySpinBox = new QDoubleSpinBox();
    drySpinBox->setRange(0.0, 10.0);
    drySpinBox->setSingleStep(0.1);
    drySpinBox->setDecimals(2);
    drySpinBox->setValue(dryLevel);
    drySpinBox->setToolTip("Input signal level fed into convolution");
    connect(drySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        dryLevel = value;
        updateFFmpegFlags();
    });
    levelsLayout->addRow("Input Level:", drySpinBox);

    wetSpinBox = new QDoubleSpinBox();
    wetSpinBox->setRange(0.0, 10.0);
    wetSpinBox->setSingleStep(0.1);
    wetSpinBox->setDecimals(2);
    wetSpinBox->setValue(wetLevel);
    wetSpinBox->setToolTip("IR level fed into convolution");
    connect(wetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        wetLevel = value;
        updateFFmpegFlags();
    });
    levelsLayout->addRow("IR Level:", wetSpinBox);

    mainLayout->addWidget(levelsGroup);

    // IR Processing section
    auto irGroup = new QGroupBox("Impulse Response Processing");
    auto irLayout = new QFormLayout(irGroup);

    lengthSpinBox = new QDoubleSpinBox();
    lengthSpinBox->setRange(0.0, 1.0);
    lengthSpinBox->setSingleStep(0.05);
    lengthSpinBox->setDecimals(2);
    lengthSpinBox->setValue(length);
    lengthSpinBox->setToolTip("IR length multiplier (1.0 = use full IR length)");
    connect(lengthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        length = value;
        updateFFmpegFlags();
    });
    irLayout->addRow("IR Length:", lengthSpinBox);

    mainLayout->addWidget(irGroup);

    // Reverb Tail (apad) section
    auto apadGroup = new QGroupBox("Reverb Tail (apad)");
    auto apadLayout = new QFormLayout(apadGroup);

    apadCheckBox = new QCheckBox("Add padding for full reverb decay");
    apadCheckBox->setChecked(useApad);
    apadCheckBox->setToolTip("Extends input duration to allow IR tail to fade naturally");
    connect(apadCheckBox, &QCheckBox::toggled, [this](bool checked) {
        useApad = checked;
        updateFFmpegFlags();
        if (apadDurationSpinBox) apadDurationSpinBox->setEnabled(checked);
    });
    apadLayout->addRow("", apadCheckBox);

    apadDurationSpinBox = new QDoubleSpinBox();
    apadDurationSpinBox->setRange(0.0, 60.0);
    apadDurationSpinBox->setSingleStep(0.5);
    apadDurationSpinBox->setDecimals(1);
    apadDurationSpinBox->setValue(apadDuration);
    apadDurationSpinBox->setSuffix(" seconds");
    apadDurationSpinBox->setEnabled(useApad);
    apadDurationSpinBox->setToolTip("Duration of silence to add (should match or exceed IR length)");
    connect(apadDurationSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double value) {
        apadDuration = value;
        updateFFmpegFlags();
    });
    apadLayout->addRow("Tail duration:", apadDurationSpinBox);

    mainLayout->addWidget(apadGroup);

    // Wet/Dry Mix (amix) section
    auto amixGroup = new QGroupBox("Wet/Dry Mix (amix)");
    auto amixLayout = new QVBoxLayout(amixGroup);

    amixCheckBox = new QCheckBox("Add inline wet/dry mixer");
    amixCheckBox->setChecked(useAmix);
    amixCheckBox->setToolTip("Mix the convolution output with the original dry signal");
    connect(amixCheckBox, &QCheckBox::toggled, [this](bool checked) {
        useAmix = checked;
        updateFFmpegFlags();
        if (mixSlider) mixSlider->setEnabled(checked);
        if (mixValueLabel) mixValueLabel->setEnabled(checked);
        if (mixCurveComboBox) mixCurveComboBox->setEnabled(checked);
    });
    amixLayout->addWidget(amixCheckBox);

    // Slider with Wet/Dry labels
    auto sliderLayout = new QHBoxLayout();
    
    auto wetLabel = new QLabel("Wet");
    wetLabel->setToolTip("100% convolution output");
    sliderLayout->addWidget(wetLabel);

    mixSlider = new QSlider(Qt::Horizontal);
    mixSlider->setRange(0, 100);
    mixSlider->setValue(mixBalance);
    mixSlider->setEnabled(useAmix);
    mixSlider->setToolTip("Balance between wet (convolution) and dry (original) signal");
    sliderLayout->addWidget(mixSlider, 1);

    auto dryLabel = new QLabel("Dry");
    dryLabel->setToolTip("100% original signal");
    sliderLayout->addWidget(dryLabel);

    amixLayout->addLayout(sliderLayout);

    // Value display and curve selector
    auto valueLayout = new QHBoxLayout();
    
    mixValueLabel = new QLabel(QString("Wet %1% / Dry %2%").arg(100 - mixBalance).arg(mixBalance));
    mixValueLabel->setEnabled(useAmix);
    valueLayout->addWidget(mixValueLabel);
    
    valueLayout->addStretch();
    
    auto curveLabel = new QLabel("Curve:");
    valueLayout->addWidget(curveLabel);
    
    mixCurveComboBox = new QComboBox();
    mixCurveComboBox->addItem("Equal Power", 0);
    mixCurveComboBox->addItem("Linear", 1);
    mixCurveComboBox->setCurrentIndex(mixCurve);
    mixCurveComboBox->setEnabled(useAmix);
    mixCurveComboBox->setToolTip("Equal Power maintains constant loudness through crossfade; Linear is simple percentage blend");
    connect(mixCurveComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        mixCurve = index;
        updateFFmpegFlags();
    });
    valueLayout->addWidget(mixCurveComboBox);
    
    amixLayout->addLayout(valueLayout);

    connect(mixSlider, &QSlider::valueChanged, [this](int value) {
        mixBalance = value;
        if (mixValueLabel) {
            mixValueLabel->setText(QString("Wet %1% / Dry %2%").arg(100 - value).arg(value));
        }
        updateFFmpegFlags();
    });

    mainLayout->addWidget(amixGroup);

    // Collapsible help section at bottom
    auto helpSection = new CollapsibleHelpSection(
        "<b>⚠️  Multi-input filter:</b> Requires an <b>Audio Input</b> before this filter. "
        "Load your IR file (reverb, cabinet simulation, etc.) into the Audio Input's file list.<br><br>"
        
        "<b>Convolution Levels:</b><br>"
        "These control the signal levels <i>into</i> the convolution math, not the output mix. "
        "For most uses, leave both at default (7.0).<br><br>"
        
        "<b>Reverb Tail (apad):</b><br>"
        "Enable this additional <b>apad</b> filter to allow reverb IRs to decay naturally. "
        "Set the duration to match or exceed your IR file length (up to 60 seconds).<br><br>"
        
        "<b>Wet/Dry Mix (amix):</b><br>"
        "Enable this to blend the convolution output with the original dry signal—the traditional "
        "wet/dry control musicians expect. Move slider toward <b>Wet</b> for more reverb/effect, "
        "toward <b>Dry</b> for more original signal.<br><br>"
        
        "<b>Mix Curve:</b><br>"
        "<b>Equal Power</b> (default) uses a sin/cos crossfade to maintain constant perceived loudness—"
        "no volume dip at 50%. <b>Linear</b> is a simple percentage blend.<br><br>"
        
        "<b>IR Length:</b><br>"
        "Use values less than 1.0 to truncate long IRs and reduce CPU usage. For example, 0.5 uses only the first half of the IR."
    );
    mainLayout->addWidget(helpSection);

    mainLayout->addStretch();

    return parametersWidget;
}

void FFAfir::updateFFmpegFlags() {
    // Flags are now generated dynamically in buildFFmpegFlags()
    // This method kept for UI update triggers but is now a no-op
    // since we generate fresh flags on every buildFFmpegFlags() call
}

QString FFAfir::buildFFmpegFlags() const {
    // Build afir parameter string
    QStringList afirParams;
    afirParams << QString("dry=%1").arg(dryLevel);
    afirParams << QString("wet=%1").arg(wetLevel);
    if (length < 1.0) {
        afirParams << QString("length=%1").arg(length);
    }
    QString afirString = afirParams.join(":");
    
    // Build filter chain incrementally
    QString result;
    QString afirInput;
    int idx = sidechainInputIndex;
    QString hexId = QString("%1").arg(getFilterId(), 4, 16, QChar('0')).toUpper();
    
    // Step 1: Optional amix
    if (useAmix) {
        result = QString("[0:a]asplit=2[%1-in][%1-thru];").arg(hexId);
        afirInput = QString("[%1-in]").arg(hexId);
    } else {
        afirInput = "[0:a]";
    }
    
    // Step 2: Optional apad
    if (useApad) {
        result += QString("%1apad=pad_dur=%2[%3-pad];")
            .arg(afirInput)
            .arg(apadDuration)
            .arg(hexId);
        afirInput = QString("[%1-pad]").arg(hexId);
    }
    
    // Step 3: afir (output label only needed if amix follows)
    QString afirOutput = useAmix ? QString("[%1-wet]").arg(hexId) : "";
    result += QString("%1[%2:a]afir=%3%4")
        .arg(afirInput)
        .arg(idx)
        .arg(afirString)
        .arg(afirOutput);
    
    // Step 4: Optional amix
    if (useAmix) {
        // Calculate mix weights
        double position = mixBalance / 100.0;
        double dryWeight, wetWeight;
        
        if (mixCurve == 0) {
            // Equal power crossfade (sin/cos)
            dryWeight = std::sin(position * M_PI_2);
            wetWeight = std::cos(position * M_PI_2);
        } else {
            // Linear
            dryWeight = position;
            wetWeight = 1.0 - position;
        }
        
        result += QString(";[%1-thru][%1-wet]amix=inputs=2:weights=%2 %3:normalize=0")
            .arg(hexId)
            .arg(dryWeight, 0, 'f', 4)
            .arg(wetWeight, 0, 'f', 4);
    }
    
    return result;
}

void FFAfir::toJSON(QJsonObject& json) const {
    json["type"] = "ff-afir";
    json["dryLevel"] = dryLevel;
    json["wetLevel"] = wetLevel;
    json["length"] = length;
    json["useApad"] = useApad;
    json["apadDuration"] = apadDuration;
    json["useAmix"] = useAmix;
    json["mixBalance"] = mixBalance;
    json["mixCurve"] = mixCurve;
}

void FFAfir::fromJSON(const QJsonObject& json) {
    dryLevel = json["dryLevel"].toDouble(7.0);
    wetLevel = json["wetLevel"].toDouble(7.0);
    length = json["length"].toDouble(1.0);
    useApad = json["useApad"].toBool(true);
    apadDuration = json["apadDuration"].toDouble(8.0);
    useAmix = json["useAmix"].toBool(false);
    mixBalance = json["mixBalance"].toInt(50);
    mixCurve = json["mixCurve"].toInt(0);
    // Note: sidechainInputIndex is set by FilterChain::updateMultiInputFilterIndices()
    // based on chain position, not from JSON
    
    // Update UI if it exists
    if (drySpinBox) drySpinBox->setValue(dryLevel);
    if (wetSpinBox) wetSpinBox->setValue(wetLevel);
    if (lengthSpinBox) lengthSpinBox->setValue(length);
    if (apadCheckBox) {
        apadCheckBox->setChecked(useApad);
        if (apadDurationSpinBox) {
            apadDurationSpinBox->setValue(apadDuration);
            apadDurationSpinBox->setEnabled(useApad);
        }
    }
    if (amixCheckBox) amixCheckBox->setChecked(useAmix);
    if (mixSlider) {
        mixSlider->setValue(mixBalance);
        mixSlider->setEnabled(useAmix);
    }
    if (mixValueLabel) {
        mixValueLabel->setText(QString("Wet %1% / Dry %2%").arg(100 - mixBalance).arg(mixBalance));
        mixValueLabel->setEnabled(useAmix);
    }
    if (mixCurveComboBox) {
        mixCurveComboBox->setCurrentIndex(mixCurve);
        mixCurveComboBox->setEnabled(useAmix);
    }
}