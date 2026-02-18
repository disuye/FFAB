#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QSlider;
class QDoubleSpinBox;
class QCheckBox;
class QLineEdit;

/**
 * FFDynaudnorm - Wraps FFmpeg's 'dynaudnorm' audio filter
 * Dynamic Audio Normalizer
 * FFmpeg filter: dynaudnorm with 13 unique parameters
 */
class FFDynaudnorm : public BaseFilter {
    Q_OBJECT
public:
    FFDynaudnorm();
    ~FFDynaudnorm() override = default;

    QString displayName() const override { return "Dynamic Normalizer"; }
    QString filterType() const override { return "ff-dynaudnorm"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();

    // Parameters (13 unique, ignoring short aliases)
    int m_framelen = 500;        // 10 - 8000 ms, default 500
    int m_gausssize = 31;        // 3 - 301, default 31
    double m_peak = 0.95;        // 0 - 1, default 0.95
    double m_maxgain = 10.0;     // 1 - 100, default 10
    double m_targetrms = 0.0;    // 0 - 1, default 0
    bool m_coupling = true;      // default true
    bool m_correctdc = false;    // default false
    bool m_altboundary = false;  // default false
    double m_compress = 0.0;     // 0 - 30, default 0
    double m_threshold = 0.0;    // 0 - 1, default 0
    QString m_channels = "all";  // default "all"
    double m_overlap = 0.0;      // 0 - 1, default 0
    QString m_curve = "";        // default null/empty

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QSlider* framelenSlider = nullptr;
    QDoubleSpinBox* framelenSpinBox = nullptr;
    QSlider* gausssizeSlider = nullptr;
    QDoubleSpinBox* gausssizeSpinBox = nullptr;
    QSlider* peakSlider = nullptr;
    QDoubleSpinBox* peakSpinBox = nullptr;
    QSlider* maxgainSlider = nullptr;
    QDoubleSpinBox* maxgainSpinBox = nullptr;
    QSlider* targetrmsSlider = nullptr;
    QDoubleSpinBox* targetrmsSpinBox = nullptr;
    QCheckBox* couplingCheckBox = nullptr;
    QCheckBox* correctdcCheckBox = nullptr;
    QCheckBox* altboundaryCheckBox = nullptr;
    QSlider* compressSlider = nullptr;
    QDoubleSpinBox* compressSpinBox = nullptr;
    QSlider* thresholdSlider = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QLineEdit* channelsEdit = nullptr;
    QSlider* overlapSlider = nullptr;
    QDoubleSpinBox* overlapSpinBox = nullptr;
    QLineEdit* curveEdit = nullptr;
};
