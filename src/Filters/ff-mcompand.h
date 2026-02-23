#pragma once

#include "BaseFilter.h"
#include "CompandBandData.h"
#include <QString>
#include <QWidget>
#include <QList>

class QSlider;
class QDoubleSpinBox;
class QTabWidget;
class CompandCurveWidget;

struct McompandBand {
    CompandBandData curveData;      // transfer points + soft-knee
    double attack = 0.005;          // seconds
    double decay = 0.1;             // seconds
    double crossoverFreq = 100.0;   // Hz
    double gain = 0.0;              // dB (optional per-band)
    double volume = 0.0;            // dB (optional per-band)
    double delay = 0.0;             // seconds (optional per-band)
};

/**
 * FFMcompand - Wraps FFmpeg's 'mcompand' audio filter
 * Multiband Compress or expand audio dynamic range
 * FFmpeg filter: mcompand='band1 | band2 | ...'
 * Per-band: attack,decay soft-knee transfer_points crossover_freq [delay [volume [gain]]]
 */
class FFMcompand : public BaseFilter {
    Q_OBJECT
public:
    FFMcompand();
    ~FFMcompand() override = default;

    QString displayName() const override { return "Multiband Compand"; }
    QString filterType() const override { return "ff-mcompand"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;

    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

    static constexpr int MAX_BANDS = 16;

private:
    void updateFFmpegFlags();
    QString bandToString(const McompandBand& band) const;
    void parseLegacyArgs(const QString& args);
    void rebuildUI();
    QWidget* createBandTab(int bandIndex);

    QList<McompandBand> m_bands;

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QTabWidget* tabWidget = nullptr;

    struct BandWidgets {
        CompandCurveWidget* curve = nullptr;
        QSlider* attackSlider = nullptr;
        QDoubleSpinBox* attackSpin = nullptr;
        QSlider* decaySlider = nullptr;
        QDoubleSpinBox* decaySpin = nullptr;
        QSlider* kneeSlider = nullptr;
        QDoubleSpinBox* kneeSpin = nullptr;
        QSlider* crossoverSlider = nullptr;
        QDoubleSpinBox* crossoverSpin = nullptr;
        QSlider* gainSlider = nullptr;
        QDoubleSpinBox* gainSpin = nullptr;
        QSlider* volumeSlider = nullptr;
        QDoubleSpinBox* volumeSpin = nullptr;
        QSlider* delaySlider = nullptr;
        QDoubleSpinBox* delaySpin = nullptr;
    };
    QList<BandWidgets> m_bandWidgets;
};
