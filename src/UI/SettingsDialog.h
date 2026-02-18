#pragma once

#include <QDialog>

class QSpinBox;
class QLineEdit;
class QCheckBox;
class QButtonGroup;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // Processing settings
    int maxConcurrent() const;
    QString ffmpegPath() const;

private:
    void loadSettings();
    void saveSettings();

    // Processing tab widgets
    QSpinBox* m_concurrentSpin = nullptr;
    QLineEdit* m_ffmpegPathEdit = nullptr;

    // Log Level tab widgets
    QCheckBox* m_showBannerCheck = nullptr;
    QButtonGroup* m_logLevelGroup = nullptr;
    QCheckBox* m_showStatsCheck = nullptr;

    // Log File tab widgets
    QCheckBox* m_saveLogCheck = nullptr;
    QCheckBox* m_logPreviewCheck = nullptr;
    QCheckBox* m_logViewWindowCheck = nullptr;

    // Helpers for reading radio button group selections
    QString selectedLogLevel() const;
    void setLogLevel(const QString& level);
};
