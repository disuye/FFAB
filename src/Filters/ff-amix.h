#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;

/**
 * FFAmix - Wraps FFmpeg's 'amix' audio filter
 * Mix multiple audio streams together with individual weights
 * 
 * This is a MULTI-INPUT filter that requires:
 * - Input [0:a]: First audio stream (primary)
 * - Input [1:a], [2:a], etc.: Additional streams (from AudioInput filters)
 * 
 * Use cases:
 * - Mix multiple audio tracks together
 * - Layer sounds with different volumes
 * - Combine voice with background music
 * - Mix multiple instrument stems
 * 
 * FFmpeg filter: [0:a][1:a][2:a]amix=inputs=3:duration=longest
 */
class FFAmix : public BaseFilter {
    Q_OBJECT
public:
    FFAmix();
    ~FFAmix() override = default;

    QString displayName() const override { return "Audio Mix (amix)"; }
    QString filterType() const override { return "ff-amix"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    int inputCount = 2;             // Number of inputs (2-32)
    QString duration = "longest";   // Duration mode: longest, shortest, first
    double dropoutTransition = 2.0; // Transition time when input ends (0-9000)
    QString weights = "0.7";    // Space-separated weight values (e.g., "1 0.5 0.25")
    bool normalize = false;          // Normalize output

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSpinBox* inputCountSpinBox = nullptr;
    QComboBox* durationCombo = nullptr;
    QDoubleSpinBox* dropoutSpinBox = nullptr;
    QLineEdit* weightsEdit = nullptr;
    QCheckBox* normalizeCheckBox = nullptr;
};
