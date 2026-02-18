#pragma once

#include "BaseFilter.h"
#include <QString>
#include <QWidget>
#include <QList>

class QListWidget;
class QDoubleSpinBox;
class QComboBox;
class QPushButton;
class QSlider;

/**
 * FFAnequalizer - Wraps FFmpeg's 'anequalizer' audio filter
 * High-order audio parametric multi-band equalizer
 * FFmpeg filter: anequalizer=params='c0 f=FREQ w=WIDTH g=GAIN t=TYPE|...'
 */
class FFAnequalizer : public BaseFilter {
    Q_OBJECT
public:
    FFAnequalizer();
    ~FFAnequalizer() override = default;

    QString displayName() const override { return "Parametric Multi EQ"; }
    QString filterType() const override { return "ff-anequalizer"; }
    QWidget* getParametersWidget() override;
    QString buildFFmpegFlags() const override;
    
    void toJSON(QJsonObject& json) const override;
    void fromJSON(const QJsonObject& json) override;

private:
    void updateFFmpegFlags();
    void addBand();
    void removeBand();
    void updateBandFromUI();
    void selectBand(int index);
    void refreshBandList();

    // EQ Band structure
    struct EQBand {
        double freq = 1000.0;    // 20 - 20000 Hz
        double width = 100.0;    // bandwidth
        double gain = 0.0;       // -30 to +30 dB
        int type = 0;            // 0=Butterworth, 1=Chebyshev1, 2=Chebyshev2, 3=Elliptic
        int channel = -1;        // -1 = all channels, 0+ = specific channel
    };

    QList<EQBand> m_bands;
    int m_selectedBand = -1;

    QString ffmpegFlags;

    // UI widgets
    QWidget* parametersWidget = nullptr;
    QListWidget* bandList = nullptr;
    QPushButton* addButton = nullptr;
    QPushButton* removeButton = nullptr;
    
    // Band editor widgets
    QDoubleSpinBox* freqSpinBox = nullptr;
    QSlider* freqSlider = nullptr;
    QDoubleSpinBox* widthSpinBox = nullptr;
    QSlider* widthSlider = nullptr;
    QDoubleSpinBox* gainSpinBox = nullptr;
    QSlider* gainSlider = nullptr;
    QComboBox* typeCombo = nullptr;
    QComboBox* channelCombo = nullptr;
    QWidget* bandEditorWidget = nullptr;
};
