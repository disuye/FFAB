#include "SettingsDialog.h"
#include "Core/UpdateChecker.h"
#include "Core/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QSettings>
#include <QThread>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QProgressBar>

SettingsDialog::SettingsDialog(QWidget* parent, UpdateChecker* checker)
    : QDialog(parent)
    , m_updateChecker(checker)
{
    setWindowTitle("FFAB Settings");
    setMinimumSize(560, 520);
    setMaximumSize(560, 520);

    auto* mainLayout = new QVBoxLayout(this);

    m_tabs = new QTabWidget();
    auto* tabs = m_tabs;

    // ===== PROCESSING TAB =====
    auto* processingPage = new QWidget();
    auto* procLayout = new QVBoxLayout(processingPage);

    // --- Concurrent instances ---
    auto* concurrentGroup = new QGroupBox("Batch Processing");
    auto* concurrentForm = new QFormLayout(concurrentGroup);

    int cpuCores = QThread::idealThreadCount();

    m_concurrentSpin = new QSpinBox();
    m_concurrentSpin->setMinimum(1);
    m_concurrentSpin->setMinimumHeight(24);
    m_concurrentSpin->setMaximum(cpuCores * 2);
    m_concurrentSpin->setValue(qMax(1, cpuCores / 2));
    m_concurrentSpin->setToolTip(QString("Your system has %1 logical cores").arg(cpuCores));
    concurrentForm->addRow("Concurrent FFmpeg instances:", m_concurrentSpin);

    auto* coreInfo = new QLabel(
        QString("<small>Detected %1 logical cores. "
                "Recommended: %2–%3 concurrent instances.<br>"
                "FFmpeg uses multiple threads internally, so more instances ≠ always faster.</small>")
        .arg(cpuCores)
        .arg(qMax(1, cpuCores / 4))
        .arg(qMax(1, cpuCores / 2)));
    coreInfo->setStyleSheet("color: #808080;");
    coreInfo->setWordWrap(true);
    concurrentForm->addRow(coreInfo);

    procLayout->addWidget(concurrentGroup);

    // --- FFmpeg path ---
    auto* pathGroup = new QGroupBox("FFmpeg");
    auto* pathForm = new QFormLayout(pathGroup);

    auto* pathRow = new QHBoxLayout();
    m_ffmpegPathEdit = new QLineEdit();
    m_ffmpegPathEdit->setPlaceholderText("/usr/local/bin/ffmpeg");
    m_ffmpegPathEdit->setMinimumWidth(240);
    m_ffmpegPathEdit->setMinimumHeight(24);
    pathRow->addWidget(m_ffmpegPathEdit);

    auto* browseBtn = new QPushButton("Browse...");
    connect(browseBtn, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Select FFmpeg Binary");
        if (!path.isEmpty()) {
            m_ffmpegPathEdit->setText(path);
        }
    });
    pathRow->addWidget(browseBtn);
    pathForm->addRow("FFmpeg path:", pathRow);

    auto* pathInfo = new QLabel(
        "<small>Leave blank to use the default path. "
        "FFAB will auto-detect FFmpeg on launch if available.</small>");
    pathInfo->setStyleSheet("color: #808080;");
    pathInfo->setWordWrap(true);
    pathForm->addRow(pathInfo);

    procLayout->addWidget(pathGroup);
    procLayout->addStretch();

    tabs->addTab(processingPage, "Processing");

    // ===== LOG LEVEL TAB =====
    auto* logPage = new QWidget();
    auto* logLayout = new QVBoxLayout(logPage);

    // // Separator
    // auto* sep1 = new QFrame();
    // sep1->setFrameShape(QFrame::HLine);
    // sep1->setFrameShadow(QFrame::Sunken);
    // logLayout->addWidget(sep1);

    // --- FFmpeg Log Level ---
    auto* logLevelGroup = new QGroupBox("FFmpeg Log Level");
    auto* logLevelLayout = new QVBoxLayout(logLevelGroup);

    m_logLevelGroup = new QButtonGroup(this);

    struct LogLevelEntry { const char* label; const char* value; };
    LogLevelEntry logLevels[] = {
        {"Quiet",               "quiet"},
        {"Panic",               "panic"},
        {"Fatal",               "fatal"},
        {"Error ← FFAB Default",     "error"},
        {"Warning",             "warning"},
        {"Info ← required for Analysis, astats etc.",     "info"},
        {"Verbose",             "verbose"},
        {"Debug",               "debug"},
        {"Trace",               "trace"},
    };

    int logLevelId = 0;
    for (const auto& entry : logLevels) {
        auto* radio = new QRadioButton(entry.label);
        radio->setProperty("logValue", entry.value);
        radio->setMinimumHeight(24);
        m_logLevelGroup->addButton(radio, logLevelId++);
        logLevelLayout->addWidget(radio);
    }

    auto* infoLabel = new QLabel(
    "<small>"
    "Log levels are hierarchical, each level includes previous levels. Save log to disk via Settings -> Log File."
    "</small>");
    infoLabel->setStyleSheet("color: #808080;");
    infoLabel->setWordWrap(true);
    logLevelLayout->addWidget(infoLabel);

    logLayout->addWidget(logLevelGroup);

    // --- Show Banner Checkbox ---
    m_showBannerCheck = new QCheckBox("Show Banner");
    logLayout->addWidget(m_showBannerCheck);

    auto* bannerInfo = new QLabel(
        "<small>Shows FFmpeg version and build info at startup. "
        "Default: OFF (= -hide_banner)</small>");
    bannerInfo->setStyleSheet("color: #808080;");
    bannerInfo->setWordWrap(true);
    logLayout->addWidget(bannerInfo);

    // --- Show Progress Stats Checkbox ---
    m_showStatsCheck = new QCheckBox("Show Progress Stats");
    logLayout->addWidget(m_showStatsCheck);

    auto* statsInfo = new QLabel(
        "<small>Shows size/time/speed progress line during encoding. "
        "Default: ON (= -stats)</small>");
    statsInfo->setStyleSheet("color: #808080;");
    statsInfo->setWordWrap(true);
    logLayout->addWidget(statsInfo);

    // --- Info labels ---

    logLayout->addStretch();

    tabs->addTab(logPage, "Log Level");

    // ===== LOG FILE TAB =====
    auto* logFilePage = new QWidget();
    auto* logFileLayout = new QVBoxLayout(logFilePage);

    // --- Save log to file ---
    m_saveLogCheck = new QCheckBox("Save log to file");
    logFileLayout->addWidget(m_saveLogCheck);

    auto* saveLogInfo = new QLabel(
        "<small>Captures all FFmpeg output to a TXT file in the "
        "OUTPUT folder. One file per batch run.</small>");
    saveLogInfo->setStyleSheet("color: #808080;");
    saveLogInfo->setWordWrap(true);
    logFileLayout->addWidget(saveLogInfo);

    // Separator
    auto* logSep1 = new QFrame();
    logSep1->setFrameShape(QFrame::HLine);
    logSep1->setFrameShadow(QFrame::Sunken);
    logFileLayout->addWidget(logSep1);

    // --- Also save log during Generate Preview ---
    m_logPreviewCheck = new QCheckBox("Also generate log during [Generate Preview]");
    m_logPreviewCheck->setContentsMargins(20, 0, 0, 0);
    logFileLayout->addWidget(m_logPreviewCheck);

    auto* previewLogInfo = new QLabel(
        "<small>For dialing in analysis or detection settings "
        "before committing to a full batch.</small>");
    previewLogInfo->setStyleSheet("color: #808080; margin-left: 20px;");
    previewLogInfo->setWordWrap(true);
    logFileLayout->addWidget(previewLogInfo);

    // --- Open View Log window ---
    m_logViewWindowCheck = new QCheckBox("Auto open [View Log] window");
    m_logViewWindowCheck->setContentsMargins(20, 0, 0, 0);
    logFileLayout->addWidget(m_logViewWindowCheck);

    auto* viewLogInfo = new QLabel(
        "<small>Automatically open the [View Log] window any time [Process] or [Generate Preview] is run. Refreshes when new log content is available.</small>");
    viewLogInfo->setStyleSheet("color: #808080; margin-left: 20px;");
    viewLogInfo->setWordWrap(true);
    logFileLayout->addWidget(viewLogInfo);

    // Separator
    auto* logSep2 = new QFrame();
    logSep2->setFrameShape(QFrame::HLine);
    logSep2->setFrameShadow(QFrame::Sunken);
    logFileLayout->addWidget(logSep2);

    // Info labels
    auto* logFileInfo1 = new QLabel(
        "<small>Log files are saved as tab-separated TXT in the "
        "OUTPUT folder. One file per run, prefixed with "
        "_FFAB_ for easy sorting.</small>");
    logFileInfo1->setStyleSheet("color: #808080;");
    logFileInfo1->setWordWrap(true);
    logFileLayout->addWidget(logFileInfo1);

    auto* logFileInfo2 = new QLabel(
        "<small>For analysis filters (astats, loudnorm, etc.) to "
        "produce output, set Log Level → Info on the "
        "Log Level tab.</small>");
    logFileInfo2->setStyleSheet("color: #808080;");
    logFileInfo2->setWordWrap(true);
    logFileLayout->addWidget(logFileInfo2);

    logFileLayout->addStretch();

    // Parent/child enable logic
    connect(m_saveLogCheck, &QCheckBox::toggled, [this](bool checked) {
        m_logPreviewCheck->setEnabled(checked);
        m_logViewWindowCheck->setEnabled(checked);
    });

    tabs->addTab(logFilePage, "Log File");

    // ===== UPDATES TAB =====
    auto* updatesPage = new QWidget();
    auto* updatesLayout = new QVBoxLayout(updatesPage);

    // --- Check for updates ---
    m_checkWeeklyCheck = new QCheckBox("Check once a week for updates");
    updatesLayout->addWidget(m_checkWeeklyCheck);

    // Separator
    auto* updatesSep1 = new QFrame();
    updatesSep1->setFrameShape(QFrame::HLine);
    updatesSep1->setFrameShadow(QFrame::Sunken);
    updatesLayout->addWidget(updatesSep1);

    // --- Manual link ---
    auto* manualLink = new QLabel(
        QString("<small>Or manually click this link to visit the Github page</small><br><a style='color: %1' href='https://github.com/disuye/FFAB/releases/latest'>https://github.com/disuye/FFAB/releases/latest</a>").arg(LINK_COLOR));
    manualLink->setOpenExternalLinks(true);
    manualLink->setStyleSheet("color: #808080;");
    manualLink->setWordWrap(true);
    updatesLayout->addWidget(manualLink);

    // Separator
    auto* updatesSep2 = new QFrame();
    updatesSep2->setFrameShape(QFrame::HLine);
    updatesSep2->setFrameShadow(QFrame::Sunken);
    updatesLayout->addWidget(updatesSep2);

    // --- Check Now / Download Now buttons ---
    auto* checkRow = new QHBoxLayout();
    m_checkNowBtn = new QPushButton("Check Now");
    m_checkNowBtn->setFixedWidth(100);
    checkRow->addWidget(m_checkNowBtn);

    m_downloadNowBtn = new QPushButton("Download Now");
    m_downloadNowBtn->setFixedWidth(120);
    m_downloadNowBtn->setEnabled(false);
    checkRow->addWidget(m_downloadNowBtn);

    checkRow->addStretch();
    updatesLayout->addLayout(checkRow);

    m_updateStatusLabel = new QLabel("");
    m_updateStatusLabel->setWordWrap(true);
    m_updateStatusLabel->setStyleSheet("color: #808080;");
    updatesLayout->addWidget(m_updateStatusLabel);

    // Download progress bar (hidden by default)
    m_downloadProgress = new QProgressBar();
    m_downloadProgress->setFixedHeight(4);
    m_downloadProgress->setTextVisible(false);
    m_downloadProgress->setStyleSheet(
        "QProgressBar {"
        "    border: none;"
        "    background-color: #404040;"
        "    border-radius: 2px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: rgba(255, 85, 0, 0.80);"
        "    border-radius: 2px;"
        "}"
    );
    m_downloadProgress->setVisible(false);
    updatesLayout->addWidget(m_downloadProgress);

    updatesLayout->addStretch();

    // --- Last checked label ---
    m_lastCheckedLabel = new QLabel("");
    m_lastCheckedLabel->setStyleSheet("color: #808080; font-size: 10px;");
    updatesLayout->addWidget(m_lastCheckedLabel);

    // Signal connections for Updates tab
    if (m_updateChecker) {
        connect(m_checkNowBtn, &QPushButton::clicked, [this]() {
            m_checkNowBtn->setEnabled(false);
            m_updateStatusLabel->setText("Checking...");
            m_updateChecker->checkForUpdate(true);
        });

        connect(m_updateChecker, &UpdateChecker::checkFinished, this, [this](bool available) {
            m_checkNowBtn->setEnabled(true);
            m_downloadNowBtn->setEnabled(available && !m_updateChecker->downloadUrl().isEmpty());
            if (available) {
                m_updateStatusLabel->setText(
                    QString("Update Available...\nLatest version = %1\nThis version = v%2")
                        .arg(m_updateChecker->latestVersion(), VERSION_STR));
            } else {
                m_updateStatusLabel->setText("You are running the latest version.");
            }
            QDateTime lastCheck = m_updateChecker->lastCheckTime();
            if (lastCheck.isValid()) {
                m_lastCheckedLabel->setText(
                    "Last checked: " + lastCheck.toString("MMM d, yyyy"));
            }
        });

        connect(m_updateChecker, &UpdateChecker::checkFailed, this, [this](const QString& err) {
            m_checkNowBtn->setEnabled(true);
            m_updateStatusLabel->setText("Check failed: " + err);
        });

        connect(m_downloadNowBtn, &QPushButton::clicked, [this]() {
            m_downloadNowBtn->setEnabled(false);
            m_updateStatusLabel->setText("Downloading...");
            m_updateChecker->downloadUpdate();
        });

        connect(m_updateChecker, &UpdateChecker::downloadProgress, this,
                [this](qint64 recv, qint64 total) {
            m_downloadProgress->setVisible(true);
            if (total > 0) {
                m_downloadProgress->setMaximum(static_cast<int>(total / 1024));
                m_downloadProgress->setValue(static_cast<int>(recv / 1024));
            }
        });

        connect(m_updateChecker, &UpdateChecker::downloadFinished, this,
                [this](const QString& path) {
            m_downloadProgress->setVisible(false);
            m_updateStatusLabel->setText("Downloaded to: " + path);
        });

        connect(m_updateChecker, &UpdateChecker::downloadFailed, this,
                [this](const QString& err) {
            m_downloadProgress->setVisible(false);
            m_updateStatusLabel->setText("Download failed: " + err);
        });
    } else {
        m_checkNowBtn->setEnabled(false);
    }

    tabs->addTab(updatesPage, "Updates");

    mainLayout->addWidget(tabs);

    // Dialog buttons
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, [this]() {
        saveSettings();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttons);

    loadSettings();
}

int SettingsDialog::maxConcurrent() const {
    return m_concurrentSpin->value();
}

void SettingsDialog::setCurrentTab(int index) {
    m_tabs->setCurrentIndex(index);
}

QString SettingsDialog::ffmpegPath() const {
    QString path = m_ffmpegPathEdit->text().trimmed();
    return path.isEmpty() ? "/usr/local/bin/ffmpeg" : path;
}

QString SettingsDialog::selectedLogLevel() const {
    auto* checked = m_logLevelGroup->checkedButton();
    return checked ? checked->property("logValue").toString() : "error";
}

void SettingsDialog::setLogLevel(const QString& level) {
    for (auto* btn : m_logLevelGroup->buttons()) {
        if (btn->property("logValue").toString() == level) {
            btn->setChecked(true);
            return;
        }
    }
    // Fallback to "error" if level not found
    setLogLevel("error");
}

void SettingsDialog::loadSettings() {
    QSettings settings;
    int cpuCores = QThread::idealThreadCount();

    // Processing tab
    m_concurrentSpin->setValue(
        settings.value("processing/maxConcurrent", qMax(1, cpuCores / 2)).toInt());
    m_ffmpegPathEdit->setText(
        settings.value("processing/ffmpegPath", "/usr/local/bin/ffmpeg").toString());

    // Log Level tab
    m_showBannerCheck->setChecked(settings.value("log/showBanner", false).toBool());
    setLogLevel(settings.value("log/logLevel", "error").toString());
    m_showStatsCheck->setChecked(settings.value("log/showStats", true).toBool());

    // Log File tab
    m_saveLogCheck->setChecked(settings.value("log/saveToFile", false).toBool());
    m_logPreviewCheck->setChecked(settings.value("log/logPreview", false).toBool());
    m_logViewWindowCheck->setChecked(settings.value("log/openViewLog", true).toBool());

    // Apply enabled state
    m_logPreviewCheck->setEnabled(m_saveLogCheck->isChecked());
    m_logViewWindowCheck->setEnabled(m_saveLogCheck->isChecked());

    // Updates tab
    if (m_updateChecker) {
        m_checkWeeklyCheck->setChecked(m_updateChecker->weeklyCheckEnabled());
        m_downloadNowBtn->setEnabled(
            m_updateChecker->updateAvailable() && !m_updateChecker->downloadUrl().isEmpty());

        QDateTime lastCheck = m_updateChecker->lastCheckTime();
        if (lastCheck.isValid()) {
            m_lastCheckedLabel->setText(
                "Last checked: " + lastCheck.toString("MMM d, yyyy"));
        } else {
            m_lastCheckedLabel->setText("Last checked: Never");
        }
    }
}

void SettingsDialog::saveSettings() {
    QSettings settings;

    // Processing tab
    settings.setValue("processing/maxConcurrent", m_concurrentSpin->value());
    settings.setValue("processing/ffmpegPath", m_ffmpegPathEdit->text().trimmed());

    // Log Level tab
    settings.setValue("log/showBanner", m_showBannerCheck->isChecked());
    settings.setValue("log/logLevel", selectedLogLevel());
    settings.setValue("log/showStats", m_showStatsCheck->isChecked());

    // Log File tab
    settings.setValue("log/saveToFile", m_saveLogCheck->isChecked());
    settings.setValue("log/logPreview", m_logPreviewCheck->isChecked());
    settings.setValue("log/openViewLog", m_logViewWindowCheck->isChecked());

    // Updates tab
    if (m_updateChecker) {
        m_updateChecker->setWeeklyCheckEnabled(m_checkWeeklyCheck->isChecked());
    }

    settings.sync();
}
