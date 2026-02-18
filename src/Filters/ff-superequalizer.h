#pragma once

#include "BaseFilter.h"
#include "SnapSlider.h"
#include <QString>
#include <QWidget>
#include <array>

class QSlider;
class QDoubleSpinBox;
class QLabel;

/**
 * FFSuperequalizer - Wraps FFmpeg's 'superequalizer' audio filter
 * Apply 18-band equalization filter.
 * FFmpeg filter: superequalizer=1b=<g>:2b=<g>:...:18b=<g>
 * 
 * A graphic equalizer with 18 fixed frequency bands covering the
 * full audio spectrum. Each band can be boosted or cut.
 * 
 * Band center frequencies:
 * 65, 92, 131, 185, 262, 370, 523, 740, 1047,
 * 1480, 2093, 2960, 4186, 5920, 8372, 11840, 16744, 20000 Hz
 * 
 * FFmpeg uses linear gain (0-20, unity=1.0)
 * UI displays dB (-20 to +20, unity=0 dB)
 */
class FFSuperequalizer : public BaseFilter {
    Q_OBJECT
public:
    FFSuperequalizer();
    ~FFSuperequalizer() override = default;

    QString displayName() const override { return "18-Band EQ"; }
    QString filterType() const override { return "ff-superequalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    
    // Conversion helpers
    static double dbToLinear(double db);
    static double linearToDb(double linear);

    // Parameters stored as dB values (-20 to +20)
    std::array<double, 18> m_bandsDb;

    // Band center frequencies for display
    static constexpr std::array<const char*, 18> bandLabels = {
        "65", "92", "131", "185", "262", "370", "523", "740", "1k",
        "1.5k", "2.1k", "3k", "4.2k", "5.9k", "8.4k", "12k", "17k", "20k"
    };

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    std::array<SnapSlider*, 18> bandSliders;
    std::array<QDoubleSpinBox*, 18> bandSpinBoxes;
};