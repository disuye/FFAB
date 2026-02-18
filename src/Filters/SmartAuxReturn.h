#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;

/**
 * SmartAuxReturn - Intelligently mixes insert chain back into main chain
 * 
 * This filter reads the current state of the insert chain at its PHYSICAL position
 * and mixes it with the main chain signal.
 * 
 * Use cases:
 * - After processing an IR with Volume/EQ, blend it back into main chain
 * - Add reverb to sidechain input, then mix back
 * - Process [1:a] through filters, then return to main
 * 
 * Behavior:
 * - If placed after Audio Input filters, it mixes the processed insert chain
 * - Mix level: 0.0 = only main chain, 1.0 = equal blend, 2.0 = double insert level
 * 
 * Example:
 *   Audio Input [1:a]
 *       ↓
 *   Volume (processes [1:a] → [0002])
 *       ↓
 *   Smart Aux Return ← takes [0002], mixes with main [0:a]
 *       ↓
 *   continues on main chain
 * 
 * Generated FFmpeg:
 *   [1:a]volume=...[0002];[0:a][0002]amix=inputs=2:weights=1.0 0.5[out]
 */
class SmartAuxReturn : public BaseFilter {
    Q_OBJECT
public:
    SmartAuxReturn();
    ~SmartAuxReturn() override = default;

    QString displayName() const override { 
        return QString("Smart Aux Return"); 
    }
    QString filterType() const override { return "SmartAuxReturn"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;
    
    // Mix level: how much of the insert chain to blend back
    double getMixLevel() const { return mixLevel; }
    void setMixLevel(double level) { mixLevel = level; }
    
    // Duration mode: how to determine end-of-stream
    enum class DurationMode {
        Longest,   // Duration of longest input (default)
        Shortest,  // Duration of shortest input
        First      // Duration of first input
    };
    DurationMode getDurationMode() const { return durationMode; }
    void setDurationMode(DurationMode mode) { durationMode = mode; }
    
    // Dropout transition time in seconds
    double getDropoutTransition() const { return dropoutTransition; }
    void setDropoutTransition(double seconds) { dropoutTransition = seconds; }
    
    // Normalize: scale inputs instead of simple summation
    bool getNormalize() const { return normalize; }
    void setNormalize(bool enabled) { normalize = enabled; }

private:
    void updateFFmpegFlags();

    // Parameters
    double mixLevel = 0.5;  // 0.0 = only main, 1.0 = equal, 2.0 = double insert
    DurationMode durationMode = DurationMode::Longest;
    double dropoutTransition = 2.0;  // seconds
    bool normalize = false;  // Enabled by default (FFmpeg default)

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* mixSpinBox = nullptr;
    QComboBox* durationCombo = nullptr;
    QDoubleSpinBox* dropoutSpinBox = nullptr;
    QCheckBox* normalizeCheckBox = nullptr;
    QLabel* infoLabel = nullptr;
};
