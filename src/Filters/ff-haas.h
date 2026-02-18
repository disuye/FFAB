#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QComboBox;
class QCheckBox;

/**
 * FFHaas - Wraps FFmpeg's 'haas' audio filter
 * Apply Haas Stereo Enhancer effect
 * FFmpeg filter: haas=level_in=<li>:level_out=<lo>:side_gain=<sg>:middle_source=<ms>:...
 * 
 * The Haas effect uses slight delays between left and right channels to create
 * a wider stereo image while maintaining mono compatibility.
 */
class FFHaas : public BaseFilter {
    Q_OBJECT
public:
    FFHaas();
    ~FFHaas() override = default;

    QString displayName() const override { return "Haas Effect"; }
    QString filterType() const override { return "ff-haas"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    double dbToLinear(double db) const;
    double linearToDb(double linear) const;

    // Parameters - stored as linear values internally
    double m_levelIn = 1.0;         // 0.015625 - 64 (display as -36 to +36 dB)
    double m_levelOut = 1.0;        // 0.015625 - 64 (display as -36 to +36 dB)
    double m_sideGain = 1.0;        // 0.015625 - 64 (display as -36 to +36 dB)
    int m_middleSource = 2;         // 0=left, 1=right, 2=mid, 3=side
    bool m_middlePhase = false;     // invert middle phase
    
    double m_leftDelay = 2.05;      // 0 - 40 ms
    double m_leftBalance = -1.0;    // -1 to 1
    double m_leftGain = 1.0;        // 0.015625 - 64 (display as dB)
    bool m_leftPhase = false;       // invert left phase
    
    double m_rightDelay = 2.12;     // 0 - 40 ms
    double m_rightBalance = 1.0;    // -1 to 1
    double m_rightGain = 1.0;       // 0.015625 - 64 (display as dB)
    bool m_rightPhase = true;       // invert right phase

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    
    // Tab 1 - Levels
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QSlider* levelInSlider = nullptr;
    QDoubleSpinBox* levelOutSpinBox = nullptr;
    QSlider* levelOutSlider = nullptr;
    QDoubleSpinBox* sideGainSpinBox = nullptr;
    QSlider* sideGainSlider = nullptr;
    QComboBox* middleSourceCombo = nullptr;
    QCheckBox* middlePhaseCheck = nullptr;
    
    // Tab 2 - Left/Right Channels
    QDoubleSpinBox* leftDelaySpinBox = nullptr;
    QSlider* leftDelaySlider = nullptr;
    QDoubleSpinBox* leftBalanceSpinBox = nullptr;
    QSlider* leftBalanceSlider = nullptr;
    QDoubleSpinBox* leftGainSpinBox = nullptr;
    QSlider* leftGainSlider = nullptr;
    QCheckBox* leftPhaseCheck = nullptr;
    
    QDoubleSpinBox* rightDelaySpinBox = nullptr;
    QSlider* rightDelaySlider = nullptr;
    QDoubleSpinBox* rightBalanceSpinBox = nullptr;
    QSlider* rightBalanceSlider = nullptr;
    QDoubleSpinBox* rightGainSpinBox = nullptr;
    QSlider* rightGainSlider = nullptr;
    QCheckBox* rightPhaseCheck = nullptr;
};
