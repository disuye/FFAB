#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QDoubleSpinBox;
class QComboBox;
class QLabel;
class QSlider;

/**
 * FFSidechaingate - Wraps FFmpeg's 'sidechaingate' audio filter
 * Noise gate with sidechain input for triggered gating
 *
 * This is a MULTI-INPUT filter that requires:
 * - Input [0:a]: Audio to be gated
 * - Input [1:a]: Sidechain signal (triggers gate open/close)
 *
 * Use cases:
 * - Triggered drums: Use kick to gate bass synth
 * - Noise removal: Use clean reference to gate noisy signal
 * - Creative effects: Rhythmic gating triggered by another source
 *
 * FFmpeg filter: [0:a][1:a]sidechaingate=...
 */
class FFSidechaingate : public BaseFilter {
    Q_OBJECT
public:
    FFSidechaingate();
    ~FFSidechaingate() override = default;

    QString displayName() const override {
        return QString("Sidechain Gate");
    }
    QString filterType() const override { return "ff-sidechaingate"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    // Track which sidechain input this filter uses
    int getSidechainInputIndex() const { return sidechainInputIndex; }
    void setSidechainInputIndex(int index) { sidechainInputIndex = index; }

private:
    void updateFFmpegFlags();

    // Parameters
    double levelIn = 1.0;       // Input gain (0.015625 to 64)
    double threshold = 0.125;   // Threshold for gate to open (0.00097563 to 1)
    double range = 0.06125;     // Gain reduction when closed (0 to 1, 0 = full mute)
    double ratio = 2.0;         // Expansion ratio (1 to 9000)
    double attack = 20.0;       // Attack time in ms (0.01 to 9000)
    double release = 250.0;     // Release time in ms (0.01 to 9000)
    double makeup = 1.0;        // Makeup gain (1 to 64)
    double knee = 2.82843;      // Knee width (1 to 8)
    QString detection = "rms";  // Detection mode: peak, rms
    QString link = "average";   // Link mode: average, maximum
    int sidechainInputIndex = 1;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QDoubleSpinBox* rangeSpinBox = nullptr;
    QDoubleSpinBox* ratioSpinBox = nullptr;
    QDoubleSpinBox* attackSpinBox = nullptr;
    QDoubleSpinBox* releaseSpinBox = nullptr;
    QDoubleSpinBox* makeupSpinBox = nullptr;
    QDoubleSpinBox* kneeSpinBox = nullptr;
    QComboBox* detectionCombo = nullptr;
    QComboBox* linkCombo = nullptr;
    QSlider* levelInSlider = nullptr;
    QSlider* thresholdSlider = nullptr;
    QSlider* rangeSlider = nullptr;
    QSlider* ratioSlider = nullptr;
    QSlider* kneeSlider = nullptr;
    QSlider* attackSlider = nullptr;
    QSlider* releaseSlider = nullptr;
    QSlider* makeupSlider = nullptr;
};
