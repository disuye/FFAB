#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>
#include <QRegularExpression>
#include <QSlider>

class QDoubleSpinBox;
class QComboBox;
class QLabel;
class QCheckBox;

/**
 * FFAfir - Wraps FFmpeg's 'afir' audio filter
 * Apply Finite Impulse Response filter with supplied coefficients
 * 
 * This is a MULTI-INPUT filter that requires:
 * - Input [0:a]: Audio to be processed
 * - Input [1:a]: Impulse response (IR) file
 * 
 * Use cases:
 * - Convolution reverb
 * - Cabinet simulation (guitar/bass amps)
 * - Speaker modeling
 * - Room acoustics simulation
 * 
 * FFmpeg filter: [0:a][1:a]afir=dry=<dry>:wet=<wet>:length=<length>
 */
class FFAfir : public BaseFilter {
    Q_OBJECT
public:
    FFAfir();
    ~FFAfir() override = default;

    QString displayName() const override { 
        return QString("Convolution IR (afir)");
    }
    QString filterType() const override { return "ff-afir"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;
    
    // Track which sidechain input this filter uses
    int getSidechainInputIndex() const { return sidechainInputIndex; }
    void setSidechainInputIndex(int index) { sidechainInputIndex = index; }
    
    // AFIR handles its own input routing when using apad or amix
    // (splits main input internally, only needs sidechain for IR)
    bool handlesOwnInputRouting() const override { return useAmix || useApad; }

private:
    void updateFFmpegFlags();

    // Parameters
    double dryLevel = 7.0;      // Dry signal level (0.0 to 10.0)
    double wetLevel = 7.0;      // Wet (processed) signal level (0.0 to 10.0)
    double length = 1.0;        // IR length multiplier (0.0 to 1.0, 1.0 = full length)
    bool useApad = true;        // Add apad for reverb tail (default ON)
    double apadDuration = 4.0;  // Apad duration in seconds (0.0 to 60.0)
    int sidechainInputIndex = 1; // Which [X:a] input this filter uses (default [1:a])
    bool useAmix = true;
    int mixBalance = 50;   // 0 = full wet, 100 = full dry
    int mixCurve = 0;      // 0 = equal power, 1 = linear

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QDoubleSpinBox* drySpinBox = nullptr;
    QDoubleSpinBox* wetSpinBox = nullptr;
    QDoubleSpinBox* lengthSpinBox = nullptr;
    QCheckBox* apadCheckBox = nullptr;
    QDoubleSpinBox* apadDurationSpinBox = nullptr;
    QLabel* infoLabel = nullptr;
    QCheckBox* amixCheckBox = nullptr;
    QSlider* mixSlider = nullptr;
    QLabel* mixValueLabel = nullptr;
    QComboBox* mixCurveComboBox = nullptr;
};