#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLabel;

/**
 * FFHdcd - Wraps FFmpeg's 'hdcd' audio filter
 * Apply High Definition Compatible Digital (HDCD) decoding.
 * FFmpeg filter: hdcd=disable_autoconvert=<b>:process_stereo=<b>:cdt_ms=<i>:force_pe=<b>:analyze_mode=<m>:bits_per_sample=<b>
 * 
 * HDCD is an encoding/decoding scheme that extends 16-bit audio
 * to achieve approximately 20-bit dynamic range using peak extension
 * and low-level gain adjustment encoded in the least significant bits.
 */
class FFHdcd : public BaseFilter {
    Q_OBJECT
public:
    FFHdcd();
    ~FFHdcd() override = default;

    QString displayName() const override { return "HDCD Decoder"; }
    QString filterType() const override { return "ff-hdcd"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters
    bool m_disableAutoconvert = true;  // Disable format conversion, default true
    bool m_processStereo = true;       // Process stereo channels together, default true
    int m_cdtMs = 2000;                // Code detect timer period (100-60000 ms), default 2000
    bool m_forcePe = false;            // Always extend peaks above -3dBFS, default false
    int m_analyzeMode = 0;             // 0=off, 1=lle, 2=pe, 3=cdt, 4=tgm, default off
    int m_bitsPerSample = 16;          // Valid bits per sample (16-24), default 16

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QCheckBox* disableAutoconvertCheck = nullptr;
    QCheckBox* processStereoCheck = nullptr;
    QSlider* cdtMsSlider = nullptr;
    QSpinBox* cdtMsSpinBox = nullptr;
    QCheckBox* forcePeCheck = nullptr;
    QComboBox* analyzeModeCombo = nullptr;
    QComboBox* bitsPerSampleCombo = nullptr;
};
