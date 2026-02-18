#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>

class QDoubleSpinBox;
class QComboBox;
class QLabel;
class QSlider;

/**
 * FFSidechaincompress - Wraps FFmpeg's 'sidechaincompress' audio filter
 * Compressor with sidechain input for ducking/pumping effects
 * 
 * This is a MULTI-INPUT filter that requires:
 * - Input [0:a]: Audio to be compressed
 * - Input [1:a]: Sidechain signal (triggers compression)
 * 
 * Use cases:
 * - Ducking: Lower music when voice is present
 * - Pumping effects: Rhythmic compression triggered by kick drum
 * - Podcast production: Auto-duck background music
 * 
 * FFmpeg filter: [0:a][1:a]sidechaincompress=...
 */
class FFSidechaincompress : public BaseFilter {
    Q_OBJECT
public:
    FFSidechaincompress();
    ~FFSidechaincompress() override = default;

    QString displayName() const override { 
        return QString("Sidechain Compressor");
    }
    QString filterType() const override { return "ff-sidechaincompress"; }
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
    double threshold = 0.125;   // Threshold level (0.00097563 to 1)
    double ratio = 2.0;         // Compression ratio (1 to 20)
    double attack = 20.0;       // Attack time in ms (0.01 to 2000)
    double release = 250.0;     // Release time in ms (0.01 to 9000)
    double makeup = 1.0;        // Makeup gain (1 to 64)
    double knee = 2.82843;      // Knee width (1 to 8)
    QString link = "average";   // Link mode: average, maximum
    QString detection = "rms";  // Detection mode: peak, rms
    double mix = 1.0;           // Dry/wet mix (0 to 1)
    int sidechainInputIndex = 1;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* levelInSpinBox = nullptr;
    QDoubleSpinBox* thresholdSpinBox = nullptr;
    QDoubleSpinBox* ratioSpinBox = nullptr;
    QDoubleSpinBox* attackSpinBox = nullptr;
    QDoubleSpinBox* releaseSpinBox = nullptr;
    QDoubleSpinBox* makeupSpinBox = nullptr;
    QDoubleSpinBox* kneeSpinBox = nullptr;
    QComboBox* linkCombo = nullptr;
    QComboBox* detectionCombo = nullptr;
    QDoubleSpinBox* mixSpinBox = nullptr;
    QSlider* levelInSlider = nullptr;
    QSlider* thresholdSlider = nullptr;
    QSlider* ratioSlider = nullptr;
    QSlider* attackSlider = nullptr;
    QSlider* releaseSlider = nullptr;
    QSlider* kneeSlider = nullptr;
    QSlider* makeupSlider = nullptr;
    QSlider* mixSlider = nullptr;
};
