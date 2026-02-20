#pragma once

#include <QDialog>

class QSpinBox;
class QLineEdit;
class QCheckBox;
class QButtonGroup;
class QTabWidget;
class QLabel;
class QPushButton;
class QProgressBar;
class UpdateChecker;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr,
                            UpdateChecker* checker = nullptr);

    // Processing settings
    int maxConcurrent() const;
    QString ffmpegPath() const;

    void setCurrentTab(int index);

private:
    void loadSettings();
    void saveSettings();

    QTabWidget* m_tabs = nullptr;
    UpdateChecker* m_updateChecker = nullptr;

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

    // Updates tab widgets
    QCheckBox* m_checkWeeklyCheck = nullptr;
    QLabel*    m_lastCheckedLabel = nullptr;
    QLabel*    m_updateStatusLabel = nullptr;
    QPushButton* m_checkNowBtn = nullptr;
    QPushButton* m_downloadNowBtn = nullptr;
    QProgressBar* m_downloadProgress = nullptr;

    // Helpers for reading radio button group selections
    QString selectedLogLevel() const;
    void setLogLevel(const QString& level);
};
