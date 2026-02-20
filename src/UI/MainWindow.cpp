#include "MainWindow.h"
#include "FilterChainWidget.h"
#include "FilterParamsPanel.h"
#include "InputPanel.h"
#include "OutputSettingsPanel.h"
#include "WaveformPreviewWidget.h"
#include "FileListWidget.h"
#include "RotatedLabel.h"
#include "CommandViewWindow.h"
#include "RegionPreviewWindow.h"
#include "PresetManager.h"
#include "Utils/KeyCommands.h"
#include "Core/FilterChain.h"
#include "Core/AppConfig.h"
#include "Core/AudioFileScanner.h"
#include "Core/BatchProcessor.h"
#include "Core/PreviewGenerator.h"
#include "Core/FFmpegRunner.h"
#include "Core/FFmpegDetector.h"
#include "Core/Preferences.h"
#include "Filters/BaseFilter.h"
#include "Filters/AudioInputFilter.h"
#include "Filters/OutputFilter.h"
#include "Filters/ChannelEqFilter.h"
#include "ChannelEqWidget.h"
#include "SettingsDialog.h"
#include "BatchSettingsWindow.h"
#include "BatchConfirmDialog.h"
#include "LogViewWindow.h"
#include "Core/JobListBuilder.h"
#include "Core/UpdateChecker.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QHBoxLayout>
#include <QSplitter>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QBuffer>
#include <QMessageBox>
#include <QEvent>
#include <QCloseEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QApplication>
#include <QFileInfo>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QUrl>
#include <QPlainTextEdit>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("FFAB");
    resize(900, 700);
    setMinimumWidth(900);
    
    filterChain = std::make_shared<FilterChain>();
    batchProcessor = new BatchProcessor(this);
    batchSettingsWindow = new BatchSettingsWindow(this);
    commandViewWindow = new CommandViewWindow(this);
    previewGenerator = new PreviewGenerator(this);
    presetManager = new PresetManager(this);
    regionPreviewWindow = new RegionPreviewWindow(this);
    logViewWindow = new LogViewWindow(this);
    m_updateChecker = new UpdateChecker(this);

    setupUI();
    createMenuBar();
    setupKeyCommands();
    connectSignals();
    checkFFmpegAvailability();

    // Update checker signals
    connect(m_updateChecker, &UpdateChecker::checkFinished, this, [this](bool available) {
        m_updateIcon->setVisible(available);
        m_updateLabel->setVisible(available);
        if (available) {
            m_updateLabel->setText(
                QString("<a style='color: #FF5500; text-decoration: none;' href='#'>"
                        "Version %1 available now | Current %2</a>")
                    .arg(m_updateChecker->latestVersion(), VERSION_STR));
        }
    });

    auto openUpdatesTab = [this]() {
        SettingsDialog dialog(this, m_updateChecker);
        dialog.setCurrentTab(3);
        if (dialog.exec() == QDialog::Accepted) {
            checkFFmpegAvailability();
        }
    };

    connect(m_updateIcon, &QToolButton::clicked, this, openUpdatesTab);
    connect(m_updateLabel, &QLabel::linkActivated, this, openUpdatesTab);

    // Check for updates on startup (respects 7-day cache)
    if (m_updateChecker->weeklyCheckEnabled())
        m_updateChecker->checkForUpdate(false);

    // Restore window geometry from preferences
    QByteArray geometry = Preferences::instance().mainWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    QByteArray state = Preferences::instance().mainWindowState();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    connect(qApp, &QApplication::aboutToQuit, [this]() {
        qDebug() << "Application quitting - cleaning up processes...";
        if (previewGenerator) {
            previewGenerator->cancel();
        }
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent* event) {
    // Save window geometry to preferences
    Preferences::instance().setMainWindowGeometry(saveGeometry());
    Preferences::instance().setMainWindowState(saveState());
    
    // Save command window state if it exists
    if (commandViewWindow) {
        Preferences::instance().setCommandWindowGeometry(commandViewWindow->saveGeometry());
        Preferences::instance().setCommandWindowVisible(commandViewWindow->isVisible());
    }

    // Kill Preview Generation on exit, if necessary
    if (previewGenerator) {
        previewGenerator->cancel();
    }
    
    Preferences::instance().sync();
    QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::PaletteChange) {
        // System theme changed (dark <-> light mode)
        inputPanel->getFileListWidget()->updateBackground();
        
        // Update all AudioInput filter file list backgrounds (main chain + sub-chains)
        for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
            if (auto* fileListWidget = audioInput->getFileListWidget()) {
                fileListWidget->updateBackground();
            }
        }
        // Force comboboxes to refresh with system colors
        // Clear any inline stylesheets and let system theme apply
        QList<QComboBox*> combos = findChildren<QComboBox*>();
        for (auto* combo : combos) {
            combo->setStyleSheet("");  // Clear custom styles
            combo->style()->unpolish(combo);
            combo->style()->polish(combo);
            combo->update();
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::setupUI() {
    QWidget* centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // ========== LEFT PANEL: Filter Chain (fixed 330px internal width) ==========
    
    auto leftPanel = new QWidget();
    leftPanel->setMinimumWidth(380); // FILTER-CHAIN-WIDTH --> contains FilterChainWidget.cpp + 4px + 3px + 1px border
    leftPanel->setMaximumWidth(380); // FILTER-CHAIN-WIDTH --> contains FilterChainWidget.cpp + 4px + 3px + 1px border
    auto leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(4, 4, 3, 4); // L-T-R-B
    
    // Dynamic left panel title label (static, for now)
    auto leftTitleLabel = new QLabel("Filter Chain");
    leftTitleLabel->setStyleSheet("QLabel { color: #808080; font-size: 14px; margin-left: 3px;}");
    leftLayout->addWidget(leftTitleLabel, 0);

    //leftLayout->addWidget(new QLabel("Filter Chain"), 0);
    
    filterChainWidget = new FilterChainWidget(filterChain.get());
    leftLayout->addWidget(filterChainWidget, 1);
    
    auto* processRow = new QHBoxLayout();
    processRow->setSpacing(4);

    processButton = new QPushButton("Process Files");
    processButton->setEnabled(false);
    processButton->setToolTip("Add file(s) to the INPUT panel\nor choose an OUTPUT folder\nbefore clicking Process Files");
    processRow->addWidget(processButton);

    auto batchSettingsBtn = new QPushButton("Batch Settings");
    connect(batchSettingsBtn, &QPushButton::clicked, this, &MainWindow::onShowBatchSettings);
    processRow->addWidget(batchSettingsBtn);

    leftLayout->addLayout(processRow, 0);
    
    // ========== RIGHT PANEL: Stacked Contexts (dynamic width) ==========
    
    auto rightPanel = new QWidget();
    rightPanel->setMinimumWidth(400);
    auto rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(4, 4, 4, 2);
    
    // Dynamic title label with ID badge container
    auto titleContainer = new QWidget();
    titleLayout = new QHBoxLayout(titleContainer);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(8);

    titleLabel = new QLabel("Filter Parameters");
    titleLabel->setStyleSheet("QLabel { color: #808080; font-size: 14px; }");
    titleLayout->addWidget(titleLabel);
    
    titleLayout->addStretch(1);

    docLinks = new QLabel("");
    docLinks->setOpenExternalLinks(true);
    titleLayout->addWidget(docLinks);

    filterIdBadge = new RotatedLabel("");
    filterIdBadge->setRotation(0);  // Horizontal
    filterIdBadge->setVisible(false);  // Hidden until filter selected
    rightLayout->addWidget(filterIdBadge);

    rightLayout->addWidget(titleContainer, 0);
    
    // Stacked widget with 3 panels
    stackedWidget = new QStackedWidget();
    
    inputPanel = new InputPanel();
    filterParamsPanel = new FilterParamsPanel();
    outputSettingsPanel = new OutputSettingsPanel();
    
    stackedWidget->addWidget(inputPanel);          // Index 0 - INPUT
    stackedWidget->addWidget(filterParamsPanel);   // Index 1 - FILTER
    stackedWidget->addWidget(outputSettingsPanel); // Index 2 - OUTPUT
    
    stackedWidget->setCurrentIndex(0);  // Start with INPUT panel
    titleLabel->setText("Input Parameters");
    
    rightLayout->addWidget(stackedWidget, 1);  // Stretches
    
    // Waveform preview (always visible at bottom)
    waveformPreview = new WaveformPreviewWidget();
    rightLayout->addWidget(waveformPreview, 0);
    
    // Progress Bar
    // progressBar = new QProgressBar();
    // progressBar->setVisible(false);
    // progressBar->setRange(0, 100);
    // rightLayout->addWidget(progressBar, 0);
    
    // Status Label
    statusLabel = new QLabel("");
    statusLabel->setStyleSheet("QLabel { font-size: 10px; color: #808080; }");
    statusLabel->setWordWrap(true);
    rightLayout->addWidget(statusLabel, 0);
    
    // Bottom status row (scan progress + FFmpeg LED)
    auto bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(8);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    
    // Subtle 6px scan progress bar
    scanProgressBar = new QProgressBar();
    scanProgressBar->setFixedHeight(4);
    scanProgressBar->setTextVisible(false);
    scanProgressBar->setStyleSheet(
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
    scanProgressBar->setVisible(false);
    bottomLayout->addWidget(scanProgressBar, 1);
    
    // Update available icon (hidden by default)
    m_updateIcon = new QToolButton();
    m_updateIcon->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
    m_updateIcon->setIconSize(QSize(14, 14));
    m_updateIcon->setFixedSize(18, 18);
    m_updateIcon->setStyleSheet(
        "QToolButton { border: none; background: transparent; }"
        "QToolButton:hover { background: rgba(128,128,128,0.2); border-radius: 2px; }");
    m_updateIcon->setCursor(Qt::PointingHandCursor);
    m_updateIcon->setVisible(false);
    bottomLayout->addWidget(m_updateIcon, 0);

    m_updateLabel = new QLabel("");
    m_updateLabel->setStyleSheet("QLabel { font-size: 10px; color: #808080; }");
    m_updateLabel->setCursor(Qt::PointingHandCursor);
    m_updateLabel->setVisible(false);
    bottomLayout->addWidget(m_updateLabel, 0);

    // FFmpeg Status LED (bottom right corner)
    ffmpegStatusLabel = new QLabel("");
    ffmpegStatusLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    ffmpegStatusLabel->setStyleSheet("QLabel { font-size: 10px; color: #808080; }");
    bottomLayout->addWidget(ffmpegStatusLabel, 0);

    rightLayout->addLayout(bottomLayout);
    
    // ========== MAIN SPLITTER ==========
    
    auto mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 0);  // Left panel fixed
    mainSplitter->setStretchFactor(1, 1);  // Right panel stretches
    
    mainLayout->addWidget(mainSplitter);
}

void MainWindow::createMenuBar() {
    using namespace KeyCommands;
    
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QMenu* viewMenu = menuBar()->addMenu("&View");
    QMenu* helpMenu = menuBar()->addMenu("&Help");
    

    viewMenu->addAction("View Command", [this]() {
        if (commandViewWindow) {
            commandViewWindow->show();
            commandViewWindow->raise();
            commandViewWindow->activateWindow();
        }
    });
    
    viewMenu->addAction("View Log", [this]() {
        if (logViewWindow) {
            logViewWindow->show();
            logViewWindow->raise();
            logViewWindow->activateWindow();
        }
    });
    
    viewMenu->addAction("Audio Preview", [this]() {
            if (regionPreviewWindow) {
                waveformPreview->stop();    
                regionPreviewWindow->show();
                regionPreviewWindow->raise();
                regionPreviewWindow->activateWindow();
                waveformPreview->setCanvasVisible(false);
                onGeneratePreview();
            }
        });
        
    viewMenu->addAction("Batch/Progress", [this]() {
        if (batchSettingsWindow) {
            batchSettingsWindow->show();
            batchSettingsWindow->raise();
            batchSettingsWindow->activateWindow();
        }
    });

    helpMenu->addAction("Qt Framework", qApp, &QApplication::aboutQt);

    helpMenu->addAction("Install FFmpeg...", this, [this]() {
        FFmpegInstallDialog installDlg(this);
        if (installDlg.exec() == QDialog::Accepted && !installDlg.installedPath().isEmpty()) {
            // Re-run detection with the newly installed binary
            checkFFmpegAvailability();
        }
    });
    

    // Preset actions
    QAction* newAction = fileMenu->addAction("&New", this, &MainWindow::onNew);
    newAction->setShortcut(NewDocument());
    
    fileMenu->addSeparator();

    QAction* loadAction = fileMenu->addAction("&Open...", this, &MainWindow::onOpen);
    loadAction->setShortcut(OpenPreset());
    
    fileMenu->addSeparator();
    
    QAction* saveAction = fileMenu->addAction("&Save", this, &MainWindow::onSave);
    saveAction->setShortcut(SavePreset());
    
    QAction* saveAsAction = fileMenu->addAction("Save &As...", this, &MainWindow::onSaveAs);
    saveAsAction->setShortcut(SavePresetAs());
    
    fileMenu->addSeparator();
    
    QAction* settingsAction = fileMenu->addAction("Settings...", this, &MainWindow::onShowSettings);
    settingsAction->setMenuRole(QAction::NoRole);
    
    //    QAction* settingsAction = fileMenu->addAction("Settings...", this, &MainWindow::onShowSettings);
    //    settingsAction->setMenuRole(QAction::PreferencesRole);
    
    fileMenu->addSeparator();
    
    fileMenu->addAction("&About...", this, &MainWindow::showAboutDialog);
    fileMenu->addAction("&Exit", this, &QWidget::close);
}

void MainWindow::setupKeyCommands() {
    using namespace KeyCommands;
    
    // ~/` or A key to open [+ Add Filter] menu
    bindMultiple(this, {AddFilter(), AddFilterAlt()}, [this]() {
        filterChainWidget->showAddFilterMenu();
    });

    // Right click in Filter Chain pane to open [+ Add Filter] menu
    filterChainWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filterChainWidget, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        filterChainWidget->showAddFilterMenu();
    });
   
    // Generate Previews (use with up / down in File List)
    bind(this, GeneratePreview(), [this]() {
        onGeneratePreview();
    });

    bindMultiple(this, {PlayPreview(), PlayPreviewAlt()}, [this]() {
        if (regionWindowIsActive()) {
            regionPreviewWindow->playFromRegionOrStart();
        } else if (waveformPreview) {
            waveformPreview->play();
        }
    });

    bind(this, StopPreview(), [this]() {
        if (regionWindowIsActive()) {
            regionPreviewWindow->stopPlayback();
        } else if (waveformPreview) {
            waveformPreview->stop();
        }
    });
     
    // bind(this, DeleteFilter(), [this]() {
    //     // Delete currently selected filter
    //     int pos = filterChainWidget->getCurrentSelection();
    //     if (pos >= 0) {
    //         filterChainWidget->confirmDeleteFilter(pos);
    //     }
    // });
    
    // bind(this, MoveFilterUp(), [this]() {
    //     int pos = filterChainWidget->getCurrentSelection();
    //     if (pos >= 0) {
    //         filterChainWidget->moveFilterUp(pos);
    //     }
    // });
    
    // bind(this, MoveFilterDown(), [this]() {
    //     int pos = filterChainWidget->getCurrentSelection();
    //     if (pos >= 0) {
    //         filterChainWidget->moveFilterDown(pos);
    //     }
    // });    
    
    // // --- View ---
    // bind(this, ShowCommandViewer(), [this]() {
    //     onShowCommandViewer();
    // });
}

void MainWindow::connectSignals() {
    // Filter chain signals
    connect(filterChainWidget, &FilterChainWidget::filterSelected,
            this, &MainWindow::onFilterSelected);
    
    connect(filterChainWidget, &FilterChainWidget::subChainFilterSelected,
            this, &MainWindow::onSubChainFilterSelected);
    
    connect(filterChainWidget, &FilterChainWidget::chainModified,
            this, &MainWindow::onChainModified);
    
    // Input Panel signals
    connect(inputPanel, &InputPanel::addFolderRequested,
            this, &MainWindow::onAddFolder);
    
    connect(inputPanel, &InputPanel::addFilesRequested,
            this, &MainWindow::onAddFiles);
    
    connect(inputPanel, &InputPanel::clearRequested,
            this, &MainWindow::onClearFiles);
    
    // connect(inputPanel, &InputPanel::outputFolderChanged, // <- OUTPUT PANEL on Input Page
    connect(outputSettingsPanel, &OutputSettingsPanel::outputFolderChanged,    
            this, &MainWindow::onOutputFolderChanged);
    
    // File list signals (access through inputPanel)
    connect(inputPanel->getFileListWidget(), &FileListWidget::fileSelectionChanged,
            this, &MainWindow::onFileSelectionChanged);
    
    connect(inputPanel->getFileListWidget(), &FileListWidget::rescanRequested,
            this, &MainWindow::onRescanMainFileList);
    
    // Preview generation signals
    connect(waveformPreview, &WaveformPreviewWidget::generatePreviewRequested,
            this, &MainWindow::onGeneratePreview);

    connect(regionPreviewWindow, &RegionPreviewWindow::generatePreviewRequested, 
            this, &MainWindow::onGeneratePreview);        
    
    connect(waveformPreview, &WaveformPreviewWidget::viewCommandRequested,
            this, &MainWindow::onViewCommand);

    connect(waveformPreview, &WaveformPreviewWidget::regionPreviewRequested, this, [this]() {
        waveformPreview->stop();    
        regionPreviewWindow->show();
        regionPreviewWindow->raise();
        regionPreviewWindow->activateWindow();
        waveformPreview->setCanvasVisible(false);
        onGeneratePreview();
    });        
    
    // Restore small waveform when region window closes
    connect(regionPreviewWindow, &RegionPreviewWindow::windowClosed, this, [this]() {
        waveformPreview->stop();
        waveformPreview->setCanvasVisible(true);
    });
    
    // Cross-stop: when either player starts, stop the other
    connect(regionPreviewWindow, &RegionPreviewWindow::playbackStarted,
            waveformPreview, &WaveformPreviewWidget::stop);
    connect(waveformPreview, &WaveformPreviewWidget::playbackStarted,
            regionPreviewWindow, &RegionPreviewWindow::stopPlayback);
    
    connect(previewGenerator, &PreviewGenerator::started,
            this, &MainWindow::onPreviewStarted);
    
    connect(previewGenerator, &PreviewGenerator::progress,
            this, &MainWindow::onPreviewProgress);
    
    connect(previewGenerator, &PreviewGenerator::finished,
            this, &MainWindow::onPreviewFinished);
    
    connect(previewGenerator, &PreviewGenerator::error,
            this, &MainWindow::onPreviewError);
    
    // Process button
    connect(processButton, &QPushButton::clicked,
            this, &MainWindow::onProcessFiles);

    // Batch Settings Window
    connect(batchSettingsWindow, &BatchSettingsWindow::processRequested,
            this, &MainWindow::onBatchProcessRequested);
    
    // Connect batch processor signals to BOTH MainWindow AND BatchSettingsWindow
    connect(batchProcessor, &BatchProcessor::started,
            batchSettingsWindow, &BatchSettingsWindow::onBatchStarted);

    connect(batchProcessor, &BatchProcessor::fileStarted,
            batchSettingsWindow, &BatchSettingsWindow::onFileStarted);

    connect(batchProcessor, &BatchProcessor::fileProgress,
            batchSettingsWindow, &BatchSettingsWindow::onFileProgress);

    connect(batchProcessor, &BatchProcessor::fileFinished,
            batchSettingsWindow, &BatchSettingsWindow::onFileFinished);

    connect(batchProcessor, &BatchProcessor::allFinished,
            batchSettingsWindow, &BatchSettingsWindow::onBatchFinished);            
    
    // Batch signals
    connect(batchProcessor, &BatchProcessor::started,
            this, &MainWindow::onBatchStarted);
    
    connect(batchProcessor, &BatchProcessor::fileStarted,
            this, &MainWindow::onFileStarted);
    
    connect(batchProcessor, &BatchProcessor::fileProgress,
            this, &MainWindow::onFileProgress);
    
    connect(batchProcessor, &BatchProcessor::fileFinished,
            this, &MainWindow::onFileFinished);
    
    connect(batchProcessor, &BatchProcessor::allFinished,
            this, &MainWindow::onBatchFinished);

    // Log view window connections
    connect(batchProcessor, &BatchProcessor::logFileCreated, this, [this](const QString& path) {
        QSettings settings;
        if (settings.value("log/openViewLog", true).toBool()) {
            logViewWindow->setLogFile(path);
            logViewWindow->show();
            logViewWindow->raise();
        }
    });

    connect(previewGenerator, &PreviewGenerator::logFileCreated, this, [this](const QString& path) {
        QSettings settings;
        if (settings.value("log/openViewLog", true).toBool()) {
            logViewWindow->setLogFile(path);
            logViewWindow->show();
            logViewWindow->raise();
        }
    });

    // Live log updates during batch and preview
    connect(batchProcessor, &BatchProcessor::logContentWritten, this, [this]() {
        if (logViewWindow && logViewWindow->isVisible()) {
            logViewWindow->reload();
        }
    });

    connect(previewGenerator, &PreviewGenerator::logContentWritten, this, [this]() {
        if (logViewWindow && logViewWindow->isVisible()) {
            logViewWindow->reload();
        }
    });

    // Final reload when preview finishes (catches any data written after last live update)
    connect(previewGenerator, &PreviewGenerator::finished, this, [this]() {
        if (logViewWindow && logViewWindow->isVisible()) {
            logViewWindow->reload();
        }
    });
}

void MainWindow::connectAudioInputButtons(AudioInputFilter* audioInput) {
    if (!audioInput) return;
    
    auto fileListWidget = audioInput->getFileListWidget();
    if (!fileListWidget) return;
    
    // Disconnect ALL existing connections from buttons to avoid duplicates
    if (audioInput->addFolderBtn) {
        audioInput->addFolderBtn->disconnect();
    }
    if (audioInput->addFilesBtn) {
        audioInput->addFilesBtn->disconnect();
    }
    if (audioInput->clearBtn) {
        audioInput->clearBtn->disconnect();
    }
    
    // Disconnect and reconnect rescan signal
    disconnect(fileListWidget, &FileListWidget::rescanRequested, nullptr, nullptr);
    connect(fileListWidget, &FileListWidget::rescanRequested, [this, audioInput, fileListWidget]() {
        onRescanAudioInputFileList(audioInput, fileListWidget);
    });
    
    // Add Folder button
    connect(audioInput->addFolderBtn, &QPushButton::clicked, [this, audioInput, fileListWidget]() {
        QString defaultPath = Preferences::instance().lastAudioInputDirectory();
        QString inputFolder = QFileDialog::getExistingDirectory(
            this,
            "Select AudioInput Folder",
            defaultPath,
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        
        if (inputFolder.isEmpty()) return;
        
        // Save the directory for next time
        Preferences::instance().setLastAudioInputDirectory(inputFolder);
        
        bool shouldScan = audioInput->shouldScanMetadata();
        statusLabel->setText(shouldScan ? "Scanning AudioInput folder..." : "Adding AudioInput files...");
        scanProgressBar->setVisible(true);
        scanProgressBar->setRange(0, 0);
        
        QList<FileListWidget::AudioFileInfo> files = AudioFileScanner::scanFolder(
            inputFolder,
            shouldScan ? ffprobePath : "",
            [this](int current, int total) {
                if (scanProgressBar->maximum() != total) {
                    scanProgressBar->setRange(0, total);
                }
                scanProgressBar->setValue(current);
                statusLabel->setText(QString("Scanning... %1/%2").arg(current).arg(total));
                QCoreApplication::processEvents();
            }
        );
        
        scanProgressBar->setVisible(false);
        fileListWidget->addFiles(files);
        statusLabel->setText(QString("Added %1 AudioInput files").arg(files.size()));
        onChainModified();  // Update command preview
    });
    
    // Add Files button
    connect(audioInput->addFilesBtn, &QPushButton::clicked, [this, audioInput, fileListWidget]() {
        QString defaultPath = Preferences::instance().lastAudioInputDirectory();
        QStringList filePaths = QFileDialog::getOpenFileNames(
            this,
            "Add AudioInput Files",
            defaultPath,
            "Audio Files (*.wav *.mp3 *.flac *.aiff *.aif *.m4a *.ogg *.opus *.wma *.aac);;All Files (*)"
        );
        
        if (filePaths.isEmpty()) return;
        
        // Save the directory for next time
        Preferences::instance().setLastAudioInputDirectory(QFileInfo(filePaths.first()).absolutePath());
        
        bool shouldScan = audioInput->shouldScanMetadata();
        statusLabel->setText(shouldScan ?
            QString("Scanning %1 AudioInput files...").arg(filePaths.size()) :
            QString("Adding %1 AudioInput files...").arg(filePaths.size()));
        scanProgressBar->setVisible(true);
        scanProgressBar->setRange(0, filePaths.size());
        scanProgressBar->setValue(0);
        
        QList<FileListWidget::AudioFileInfo> files;
        for (int i = 0; i < filePaths.size(); ++i) {
            FileListWidget::AudioFileInfo info;
            
            if (shouldScan) {
                info = AudioFileScanner::extractMetadata(filePaths[i], ffprobePath);
            } else {
                info.filePath = filePaths[i];
                info.fileName = QFileInfo(filePaths[i]).fileName();
                info.format = QFileInfo(filePaths[i]).suffix().toUpper();
                info.duration = "00:00:00";
                info.sampleRate = 0;
                info.channels = 0;
                info.bitrate = 0;
                info.enabled = true;
            }
            
            files.append(info);
            scanProgressBar->setValue(i + 1);
            QCoreApplication::processEvents();
        }
        
        scanProgressBar->setVisible(false);
        fileListWidget->addFiles(files);
        statusLabel->setText(shouldScan ?
            QString("Added %1 AudioInput files with metadata").arg(files.size()) :
            QString("Added %1 AudioInput files").arg(files.size()));
        onChainModified();  // Update command preview
    });
    
    // Clear button
    connect(audioInput->clearBtn, &QPushButton::clicked, [this, fileListWidget]() {
        fileListWidget->clearFiles();
        statusLabel->setText("AudioInput file list cleared");
        onChainModified();  // Update command preview
    });
}

// ========== FILTER CHAIN SLOTS ==========

void MainWindow::onFilterSelected(int position) {
    auto filter = filterChain->getFilter(position);

    if (!filter) return;
    
    if (filter->filterType() == "input") {
        titleLabel->setText("Input Parameters");
        filterIdBadge->setVisible(false);
        stackedWidget->setCurrentIndex(0);
        docLinks->setText("");
    } else if (filter->filterType() == "output") {
        titleLabel->setText("Output Parameters");
        filterIdBadge->setVisible(false);
        stackedWidget->setCurrentIndex(1);  // Use filterParamsPanel, not outputSettingsPanel
        docLinks->setText("");
        filterParamsPanel->setFilterWidget(filter->getParametersWidget());
        // Connect output folder changed signal
        if (auto* outputFilter = dynamic_cast<OutputFilter*>(filter.get())) {
            connect(outputFilter, &OutputFilter::outputFolderChanged,
                    this, &MainWindow::onOutputFolderChanged, Qt::UniqueConnection);
        }
    } else {
        // Filter # ID (hexadecimal label with unique color)
        int filterId = filter->getFilterId();
        QString idHex = QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper();
        QColor rowBgColor = FilterColors::rowBackground(filterId);
        QColor labelBgColor = FilterColors::labelBackground(filterId);
        QColor textColor = FilterColors::textColor(filterId);
        
        titleLabel->setText(QString("Filter Parameters"));

        // FFmpeg links to documentation
        docLinks->setText(R"(
            <small>
                <a style='color:#808080;text-decoration:none' href="https://ffmpeg.org/ffmpeg-filters.html#Audio-Filters">FFmpeg Docs</a> | 
                <a style='color:#808080;text-decoration:none' href="https://ayosec.github.io/ffmpeg-filters-docs/8.0/Filters/Audio/">AyoseC Docs</a>
            </small>
        )");

        filterIdBadge->setText(QString("%1").arg(idHex));
        filterIdBadge->setBackgroundColor(labelBgColor);
        filterIdBadge->setTextColor(textColor);
        filterIdBadge->setVisible(true);
        filterIdBadge->setStyleSheet("font-size: 16px");

        stackedWidget->setCurrentIndex(1);
        filterParamsPanel->setFilterWidget(filter->getParametersWidget());
        
        if (auto* audioInput = dynamic_cast<AudioInputFilter*>(filter.get())) {
            connectAudioInputButtons(audioInput);
        }
    }
}

void MainWindow::onSubChainFilterSelected(int filterId) {
    // Find filter by ID in the current sub-chain
    int multiOutputPos = filterChainWidget->getCurrentMultiOutputPosition();
    int streamIndex = filterChainWidget->getCurrentStreamIndex();
    
    if (multiOutputPos < 0 || streamIndex < 1) return;
    
    // Search the sub-chain for the filter with this ID
    const auto& subChain = filterChain->getSubChain(multiOutputPos, streamIndex);
    std::shared_ptr<BaseFilter> filter;
    
    for (const auto& f : subChain) {
        if (f && f->getFilterId() == filterId) {
            filter = f;
            break;
        }
    }
    
    if (!filter) return;
    
    // Display filter parameters (same as onFilterSelected for middle filters)
    QString idHex = QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper();
    QColor rowBgColor = FilterColors::rowBackground(filterId);
    QColor labelBgColor = FilterColors::labelBackground(filterId);
    QColor textColor = FilterColors::textColor(filterId);
    
    titleLabel->setText(QString("Filter Parameters"));
    
    filterIdBadge->setText(QString("%1").arg(idHex));
    filterIdBadge->setBackgroundColor(labelBgColor);
    filterIdBadge->setTextColor(textColor);
    filterIdBadge->setVisible(true);
    filterIdBadge->setStyleSheet("font-size: 16px");
    
    stackedWidget->setCurrentIndex(1);
    filterParamsPanel->setFilterWidget(filter->getParametersWidget());
    
    if (auto* audioInput = dynamic_cast<AudioInputFilter*>(filter.get())) {
        connectAudioInputButtons(audioInput);
    }
}

void MainWindow::onChainModified() {
    // Update AudioInput filter indices whenever chain is modified
    if (filterChain) {
        filterChain->updateAudioInputIndices();
        filterChain->updateMultiInputFilterIndices();
        // Refresh the chain widget to update display names
        filterChainWidget->refreshChain();
    }
    
    // Get muted filter positions
    auto mutedPositions = filterChainWidget->getMutedFilterPositions();
    
    // Build example FFmpeg command for debugging
    // Collect sidechain files from all AudioInput filters (main chain + sub-chains)
    // IMPORTANT: Always include an entry for each AudioInput (even if empty) to keep indices stable
    QStringList sidechainFiles;
    for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
        auto files = audioInput->getEnabledFilePaths();
        if (!files.isEmpty()) {
            sidechainFiles.append(files.first());
        } else {
            // No files loaded - add empty placeholder
            // FilterChain will warn about this
            sidechainFiles.append("");
        }
    }
    
    auto logSettings = LogSettings::fromQSettings();
    QString command;
    if (!sidechainFiles.isEmpty()) {
        command = filterChain->buildCompleteCommand(
            "input.wav",
            sidechainFiles,
            "output.wav",
            mutedPositions,
            logSettings
        );
    } else {
        command = filterChain->buildCompleteCommand(
            "input.wav",
            "output.wav",
            mutedPositions,
            logSettings
        );
    }
    
    qDebug() << "Filter chain modified";
    qDebug() << "Muted positions:" << mutedPositions;
    qDebug() << "Sidechain files:" << sidechainFiles;
    qDebug() << "FFmpeg command:" << command;
    
    // Update command viewer if it's open
    if (commandViewWindow && commandViewWindow->isVisible()) {
        commandViewWindow->setCommand(buildPreviewCommand());
    }
}

// ========== INPUT & BATCH SLOTS ==========

void MainWindow::onAddFolder() {
    // Prompt user for folder
    QString defaultPath = Preferences::instance().lastInputDirectory();
    QString inputFolder = QFileDialog::getExistingDirectory(
        this,
        "Select Folder to Add",
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (inputFolder.isEmpty()) {
        return;
    }
    
    // Save the directory for next time
    Preferences::instance().setLastInputDirectory(inputFolder);
    
    bool shouldScan = inputPanel->shouldScanMetadata();
    
    qDebug() << "Adding folder:" << inputFolder << "| Scan metadata:" << shouldScan;
    statusLabel->setText(shouldScan ? "Scanning folder..." : "Adding files...");
    
    // Show scan progress bar
    scanProgressBar->setVisible(true);
    scanProgressBar->setRange(0, 0);  // Indeterminate while counting
    
    if (shouldScan) {
        // Scan folder with FFprobe metadata
        QList<FileListWidget::AudioFileInfo> files = AudioFileScanner::scanFolder(
            inputFolder, 
            ffprobePath,
            [this](int current, int total) {
                // Update progress bar
                if (scanProgressBar->maximum() != total) {
                    scanProgressBar->setRange(0, total);
                }
                scanProgressBar->setValue(current);
                statusLabel->setText(QString("Scanning... %1/%2").arg(current).arg(total));
                QCoreApplication::processEvents();  // Keep UI responsive!
            }
        );
        
        // Hide scan progress bar
        scanProgressBar->setVisible(false);
        
        // Add to file list widget
        inputPanel->getFileListWidget()->addFiles(files);
        
        statusLabel->setText(QString("Found %1 audio files").arg(files.size()));
        qDebug() << "Loaded" << files.size() << "audio files with metadata";
        
    } else {
        // Fast add without metadata - just get file paths
        QList<FileListWidget::AudioFileInfo> files = AudioFileScanner::scanFolder(
            inputFolder, 
            "",  // No ffprobe path = skip metadata
            [this](int current, int total) {
                if (scanProgressBar->maximum() != total) {
                    scanProgressBar->setRange(0, total);
                }
                scanProgressBar->setValue(current);
                statusLabel->setText(QString("Adding... %1/%2").arg(current).arg(total));
                QCoreApplication::processEvents();
            }
        );
        
        // Hide scan progress bar
        scanProgressBar->setVisible(false);
        
        // Add to file list widget
        inputPanel->getFileListWidget()->addFiles(files);
        
        statusLabel->setText(QString("Added %1 audio files (no metadata)").arg(files.size()));
        qDebug() << "Added" << files.size() << "audio files without metadata";
    }
    
    // Enable process button if we have files and output folder
    if (inputPanel->getFileListWidget()->getAllFiles().size() > 0 && !currentOutputFolder.isEmpty()) {
        processButton->setEnabled(true);
        processButton->setText("Process Files");
    }
}

void MainWindow::onAddFiles() {
    QString defaultPath = Preferences::instance().lastInputDirectory();
    
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "Add Audio Files",
        defaultPath,
        "Audio Files (*.wav *.mp3 *.flac *.aiff *.aif *.m4a *.ogg *.opus *.wma *.aac);;All Files (*)"
    );
    
    if (filePaths.isEmpty()) {
        return;
    }
    
    // Save the directory for next time
    Preferences::instance().setLastInputDirectory(QFileInfo(filePaths.first()).absolutePath());
    
    bool shouldScan = inputPanel->shouldScanMetadata();
    
    qDebug() << "Adding" << filePaths.size() << "files | Scan metadata:" << shouldScan;
    statusLabel->setText(shouldScan ? 
        QString("Scanning %1 files...").arg(filePaths.size()) :
        QString("Adding %1 files...").arg(filePaths.size()));
    
    // Show scan progress bar
    scanProgressBar->setVisible(true);
    scanProgressBar->setRange(0, filePaths.size());
    scanProgressBar->setValue(0);
    
    // Extract metadata for each file (or just add without metadata)
    QList<FileListWidget::AudioFileInfo> files;
    for (int i = 0; i < filePaths.size(); ++i) {
        FileListWidget::AudioFileInfo info;
        
        if (shouldScan) {
            // Full metadata extraction with FFprobe
            info = AudioFileScanner::extractMetadata(filePaths[i], ffprobePath);
        } else {
            // Fast add - just filename, no metadata
            info.filePath = filePaths[i];
            info.fileName = QFileInfo(filePaths[i]).fileName();
            info.format = QFileInfo(filePaths[i]).suffix().toUpper();
            info.duration = "00:00:00";
            info.sampleRate = 0;
            info.channels = 0;
            info.bitrate = 0;
            info.enabled = true;
        }
        
        files.append(info);
        scanProgressBar->setValue(i + 1);
        QCoreApplication::processEvents();  // Keep UI responsive!
    }
    
    // Hide scan progress bar
    scanProgressBar->setVisible(false);
    
    // Add to file list widget
    inputPanel->getFileListWidget()->addFiles(files);
    
    // Enable process button if we have output folder
    if (!currentOutputFolder.isEmpty()) {
        processButton->setEnabled(true);
        processButton->setText("Process Files");
    }
    
    statusLabel->setText(shouldScan ?
        QString("Added %1 files with metadata").arg(files.size()) :
        QString("Added %1 files (no metadata)").arg(files.size()));
    qDebug() << "Added" << files.size() << "files" << (shouldScan ? "with metadata" : "without metadata");
}

void MainWindow::onClearFiles() {
    inputPanel->getFileListWidget()->clearFiles();
    processButton->setEnabled(false);
        processButton->setToolTip("Add file(s) to the INPUT panel\nbefore clicking Process Files");
    statusLabel->setText("File list cleared");
    qDebug() << "File list cleared";
}

// ========== SIDECHAIN INPUT SLOTS ==========


// ========== OUTPUT SLOTS ==========

void MainWindow::onOutputFolderChanged(const QString& path) {
    currentOutputFolder = path;
    qDebug() << "Output folder changed to:" << path;
    
    // Enable process button if we have files
    if (inputPanel->getFileListWidget()->getAllFiles().size() > 0) {
        processButton->setEnabled(true);
        processButton->setText("Process Files");  // Update text
    }
}

// ========== FILE LIST SLOTS ==========

void MainWindow::onFileSelectionChanged() {
    auto enabledFiles = inputPanel->getFileListWidget()->getEnabledFiles();
    qDebug() << "File selection changed -" << enabledFiles.size() << "files enabled";
    
    // Update status
    statusLabel->setText(QString("%1 files selected for processing").arg(enabledFiles.size()));
}

// Replace onRescanMainFileList() with:
void MainWindow::onRescanMainFileList() {
    auto fileListWidget = inputPanel->getFileListWidget();
    if (!fileListWidget) return;
    
    auto files = fileListWidget->getAllFiles();
    if (files.isEmpty()) {
        statusLabel->setText("No files to rescan");
        return;
    }
    
    if (!inputPanel->shouldScanMetadata()) {
        statusLabel->setText("Metadata scanning is disabled - enable checkbox first");
        return;
    }
    
    // Cancel any existing scan
    if (m_scanWatcher && m_scanWatcher->isRunning()) {
        m_scanWatcher->cancel();
        m_scanWatcher->waitForFinished();
    }
    
    // Collect file paths
    QStringList filePaths;
    for (const auto& f : files) {
        filePaths.append(f.filePath);
    }
    
    // Setup progress UI
    scanProgressBar->setVisible(true);
    scanProgressBar->setRange(0, filePaths.size());
    scanProgressBar->setValue(0);
    statusLabel->setText("Rescanning metadata...");
    
    // Store ffprobe path for lambda capture
    QString probePath = ffprobePath;
    
    // Create watcher
    if (!m_scanWatcher) {
        m_scanWatcher = new QFutureWatcher<QList<FileListWidget::AudioFileInfo>>(this);
    }
    
    // Connect completion handler
    disconnect(m_scanWatcher, nullptr, nullptr, nullptr);  // Clear old connections
    connect(m_scanWatcher, &QFutureWatcher<QList<FileListWidget::AudioFileInfo>>::finished, 
            this, [this, fileListWidget]() {
        auto results = m_scanWatcher->result();
        
        fileListWidget->setUpdatesEnabled(false);
        fileListWidget->clearFiles();
        fileListWidget->addFiles(results);
        fileListWidget->setUpdatesEnabled(true);
        
        scanProgressBar->setVisible(false);
        statusLabel->setText(QString("Rescanned %1 files").arg(results.size()));
    });
    
    // Progress updates via timer (since QtConcurrent::run doesn't support progress)
    auto* progressTimer = new QTimer(this);
    auto progressCounter = std::make_shared<std::atomic<int>>(0);
    connect(progressTimer, &QTimer::timeout, this, [this, progressCounter, filePaths, progressTimer]() {
        int current = progressCounter->load();
        scanProgressBar->setValue(current);
        statusLabel->setText(QString("Rescanning metadata... %1/%2").arg(current).arg(filePaths.size()));
        
        if (current >= filePaths.size()) {
            progressTimer->stop();
            progressTimer->deleteLater();
        }
    });
    progressTimer->start(100);  // Update UI every 100ms
    
    // Run scanning in background thread
    m_scanWatcher->setFuture(QtConcurrent::run([filePaths, probePath, progressCounter]() {
        QList<FileListWidget::AudioFileInfo> results;
        for (const QString& path : filePaths) {
            results.append(AudioFileScanner::extractMetadata(path, probePath));
            (*progressCounter)++;
        }
        return results;
    }));
}

void MainWindow::onRescanAudioInputFileList(AudioInputFilter* audioInput, FileListWidget* fileListWidget) {
    if (!audioInput || !fileListWidget) return;
    
    auto files = fileListWidget->getAllFiles();
    if (files.isEmpty()) {
        statusLabel->setText("No Audio Input files to rescan");
        return;
    }
    
    if (!audioInput->shouldScanMetadata()) {
        statusLabel->setText("Metadata scanning is disabled - enable checkbox first");
        return;
    }
    
    // Collect file paths
    QStringList filePaths;
    for (const auto& f : files) {
        filePaths.append(f.filePath);
    }
    
    // Setup progress UI
    scanProgressBar->setVisible(true);
    scanProgressBar->setRange(0, filePaths.size());
    scanProgressBar->setValue(0);
    statusLabel->setText("Rescanning AudioInput metadata...");
    
    QString probePath = ffprobePath;
    
    // Create watcher for this scan (use a local one since we might have multiple AudioInputs)
    auto* watcher = new QFutureWatcher<QList<FileListWidget::AudioFileInfo>>(this);
    
    // Progress updates via timer
    auto* progressTimer = new QTimer(this);
    auto progressCounter = std::make_shared<std::atomic<int>>(0);
    connect(progressTimer, &QTimer::timeout, this, [this, progressCounter, filePaths, progressTimer]() {
        int current = progressCounter->load();
        scanProgressBar->setValue(current);
        statusLabel->setText(QString("Rescanning AudioInput metadata... %1/%2").arg(current).arg(filePaths.size()));
        
        if (current >= filePaths.size()) {
            progressTimer->stop();
            progressTimer->deleteLater();
        }
    });
    progressTimer->start(100);
    
    // Connect completion handler
    connect(watcher, &QFutureWatcher<QList<FileListWidget::AudioFileInfo>>::finished,
            this, [this, fileListWidget, watcher, progressTimer]() {
        auto results = watcher->result();
        
        fileListWidget->setUpdatesEnabled(false);
        fileListWidget->clearFiles();
        fileListWidget->addFiles(results);
        fileListWidget->setUpdatesEnabled(true);
        
        scanProgressBar->setVisible(false);
        statusLabel->setText(QString("Rescanned %1 AudioInput files").arg(results.size()));
        
        progressTimer->stop();
        progressTimer->deleteLater();
        watcher->deleteLater();
    });
    
    // Run scanning in background thread
    watcher->setFuture(QtConcurrent::run([filePaths, probePath, progressCounter]() {
        QList<FileListWidget::AudioFileInfo> results;
        for (const QString& path : filePaths) {
            results.append(AudioFileScanner::extractMetadata(path, probePath));
            (*progressCounter)++;
        }
        return results;
    }));
}

// ========== PROCESS BUTTON ==========

void MainWindow::onProcessFiles() {
    // Cancel any existing batch first
    batchProcessor->cancel();

    // Get output folder from OutputFilter in the chain
    QString outputFolder;
    if (filterChain) {
        qDebug() << "FilterChain exists, filter count:" << filterChain->filterCount();
        auto outputFilter = filterChain->getFilter(filterChain->filterCount() - 1);
        if (outputFilter) {
            qDebug() << "Got filter, type:" << outputFilter->filterType();
        }
        if (auto* output = dynamic_cast<OutputFilter*>(outputFilter.get())) {
            outputFolder = output->getOutputFolder();
            qDebug() << "OutputFilter cast succeeded, folder:" << outputFolder;
        } else {
            qDebug() << "OutputFilter cast FAILED";
        }
    } else {
        qDebug() << "FilterChain is NULL";
    }
    
    if (outputFolder.isEmpty()) {
        QMessageBox::warning(this, "No Output Folder", 
            "Please select an output folder first.");
        return;
    }
    
    auto enabledFiles = inputPanel->getFileListWidget()->getEnabledFiles();
    
    if (enabledFiles.isEmpty()) {
        QMessageBox::warning(this, "No Files Selected", 
            "Please enable at least one file to process.");
        return;
    }
    
    // Get muted filter positions
    auto mutedPositions = filterChainWidget->getMutedFilterPositions();
    
    // Update filter indices before processing
    if (filterChain) {
        filterChain->updateAudioInputIndices();
        filterChain->updateMultiInputFilterIndices();
    }
    
    // Collect sidechain files from all AudioInput filters (main chain + sub-chains)
    // IMPORTANT: Always include an entry for each AudioInput (even if empty) to keep indices stable
    QStringList sidechainFiles;
    for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
        auto files = audioInput->getEnabledFilePaths();
        if (!files.isEmpty()) {
            sidechainFiles.append(files.first());
        } else {
            // No files loaded - add empty placeholder
            sidechainFiles.append("");
        }
    }
    
    qDebug() << "Starting batch processing:" << enabledFiles.size() << "files";
    qDebug() << "Sidechain files:" << sidechainFiles;
    
    // Start batch processing with detected ffmpeg path and sidechain files
    batchProcessor->start(enabledFiles, outputFolder, filterChain, mutedPositions, sidechainFiles, ffmpegPath);
}

// ========== BATCH PROCESSOR SLOTS ==========

void MainWindow::onBatchStarted(int totalFiles) {
    processButton->setEnabled(false);
    processButton->setToolTip("Pause or quit the running Batch\nbefore clicking Process Files");
    scanProgressBar->setVisible(true);
    scanProgressBar->setRange(0, 100);
    scanProgressBar->setValue(0);
    statusLabel->setText(QString("Processing %1 files...").arg(totalFiles));
    
    qDebug() << "Batch started:" << totalFiles << "files";
}

void MainWindow::onFileStarted(const QString& fileName, int fileNumber, int totalFiles) {
    statusLabel->setText(QString("Processing %1/%2: %3")
        .arg(fileNumber)
        .arg(totalFiles)
        .arg(fileName));
    
   scanProgressBar->setValue(0);
    
    qDebug() << "Processing file" << fileNumber << "/" << totalFiles << ":" << fileName;
}

void MainWindow::onFileProgress(const FFmpegRunner::ProgressInfo& info) {
   scanProgressBar->setValue(info.progressPercent);
    
    QString speedStr = QString::number(info.speed, 'f', 1) + "x";
    QString totalTimeStr = info.totalTime > 0 
        ? QString::number(info.totalTime, 'f', 1) + "s"
        : "unknown";
    
    statusLabel->setText(
        QString("Progress: %1 / %2 (Speed: %3)")
        .arg(info.timeString)
        .arg(totalTimeStr)
        .arg(speedStr)
    );
}

void MainWindow::onFileFinished(const QString& fileName, bool success) {
    if (success) {
        qDebug() << "Successfully processed:" << fileName;
    } else {
        qWarning() << "Failed to process:" << fileName;
    }
}

void MainWindow::onBatchFinished(int completed, int failed) {
    processButton->setEnabled(true);
   scanProgressBar->setVisible(false);
   scanProgressBar->setValue(0);
    
    QString message = QString("Batch processing complete!\n\n"
                              "%1 files succeeded\n"
                              "%2 files failed")
                      .arg(completed)
                      .arg(failed);
    
    statusLabel->setText(QString("%1 succeeded, %2 failed").arg(completed).arg(failed));
    
    qDebug() << "Batch finished:" << completed << "succeeded," << failed << "failed";
    
    QMessageBox::information(this, "Batch Complete", message);
}

// ========== MENU ACTIONS ==========


void MainWindow::onShowSettings() {
    SettingsDialog dialog(this, m_updateChecker);
    if (dialog.exec() == QDialog::Accepted) {
        // Re-detect FFmpeg in case the user changed the path
        checkFFmpegAvailability();
    }
}

void MainWindow::showAboutDialog() {
    QDialog *aboutDialog = new QDialog(this);
    aboutDialog->setWindowTitle("About FFAB");
    aboutDialog->setMinimumSize(320, 480);
    aboutDialog->setMaximumSize(320, 480);
    
    QPixmap iconPixmap(":/AppIcon.png");
    QPixmap iconSmall = iconPixmap.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    if (!iconPixmap.isNull()) {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        iconSmall.save(&buffer, "PNG");
        QByteArray byteArray = buffer.data();
        QString base64Image = QString(byteArray.toBase64());
        
        QString text = R"(
            <center>
            <div><img src='data:image/png;base64,)" + base64Image + R"(' width='64' height='64'></div>
            <div style="font-size:24px; letter-spacing: 8px;">FFAB</div>

            <div>
            <i>FFmpeg Audio Batch</i><br>
            <small>Version )" + QString(VERSION_STR) + R"(<small><br>
            </div>

            <div>
            ©2025 Dan F / <a style='color:)" + QString(LINK_COLOR) + R"(;' href="https://www.disuye.com/">Disuye</a><br>
            Made in Hong Kong<br>
            Free | Donationware | <a style='color:)" + QString(LINK_COLOR) + R"(;' href="https://www.disuye.com/">Buy My Music</a><br>
            </div>

            <small>
            <div>
            Powered by FFmpeg<br>
            <a style='color:)" + QString(LINK_COLOR) + R"(;' href="https://ffmpeg.org/download.html">ffmpeg.org</a><br>
            </div>

            <div>
            Qt 6.x Framework<br>
            <a style='color:)" + QString(LINK_COLOR) + R"(;' href="https://download.qt.io/official_releases/qt/">Source Code &amp; License</a><br>
            GNU LGP License v3<br>
            </div>

            <div>
            Fira Code font ©2014<br>
            <a style='color:)" + QString(LINK_COLOR) + R"(;' href="https://github.com/tonsky/FiraCode">Fira Code Project Authors</a><br>
            SIL OF License v1.1
            </div>
            </small>
            </center>
        )";
        
        QVBoxLayout *layout = new QVBoxLayout(aboutDialog);
        QLabel *label = new QLabel(text);
        label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        label->setOpenExternalLinks(true);
        layout->addWidget(label);
        
        // Add license buttons
        auto* btnLayout = new QHBoxLayout();
        btnLayout->setSpacing(8);
        
        auto* lgplBtn = new QPushButton("LGPLv3");
        auto* gplBtn = new QPushButton("GPLv3");
        auto* oflBtn = new QPushButton("OFLv1.1");
        auto* flBtn = new QPushButton("FFAB");
        
        connect(flBtn, &QPushButton::clicked, [this]() {
            if (flWindow && flWindow->isVisible()) {
                flWindow->raise();
                flWindow->activateWindow();
                return;
            }
            flWindow = showLicenseWindow("FFAB", ":/licenses/FFAB-license.txt");
        });
                connect(lgplBtn, &QPushButton::clicked, [this]() {
            if (lgplWindow && lgplWindow->isVisible()) {
                lgplWindow->raise();
                lgplWindow->activateWindow();
                return;
            }
            lgplWindow = showLicenseWindow("LGPLv3.txt", ":/licenses/LGPLv3.txt");
        });
        connect(gplBtn, &QPushButton::clicked, [this]() {
            if (gplWindow && gplWindow->isVisible()) {
                gplWindow->raise();
                gplWindow->activateWindow();
                return;
            }
            gplWindow = showLicenseWindow("GPLv3.txt", ":/licenses/GPLv3.txt");
        });
        connect(oflBtn, &QPushButton::clicked, [this]() {
            if (oflWindow && oflWindow->isVisible()) {
                oflWindow->raise();
                oflWindow->activateWindow();
                return;
            }
            oflWindow = showLicenseWindow("FiraCode-OFL.txt", ":/licenses/FiraCode-OFL.txt");
        });
        connect(aboutDialog, &QWidget::destroyed, [this]() {
            if (flWindow)  { flWindow->close();  flWindow = nullptr; }
            if (lgplWindow) { lgplWindow->close(); lgplWindow = nullptr; }
            if (gplWindow)  { gplWindow->close();  gplWindow = nullptr; }
            if (oflWindow)  { oflWindow->close();  oflWindow = nullptr; }
        });
        
        btnLayout->addStretch();
                
        flBtn->setAutoDefault(false);
        flBtn->setDefault(false);
        btnLayout->addWidget(flBtn);
        
        lgplBtn->setAutoDefault(false);
        lgplBtn->setDefault(false);
        btnLayout->addWidget(lgplBtn);
        
        gplBtn->setAutoDefault(false);
        gplBtn->setDefault(false);
        btnLayout->addWidget(gplBtn);
        
        oflBtn->setAutoDefault(false);
        oflBtn->setDefault(false);
        btnLayout->addWidget(oflBtn);
        
        btnLayout->addStretch();
        
        layout->addLayout(btnLayout);

        // Open window but avoid modal event loop (allow UI interaction)
        aboutDialog->setLayout(layout);
        aboutDialog->setModal(false);
        aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
        aboutDialog->show();
        aboutDialog->raise();
    } 
}

// ========== PREVIEW GENERATION SLOTS ==========

QString MainWindow::buildPreviewCommand() {
    // Get selected file from file list
    auto fileList = inputPanel->getFileListWidget();
    auto selectedFiles = fileList->getSelectedFiles();
    
    if (selectedFiles.size() > 1) {
        return "# Multiple files selected - please highlight only one in the INPUT File List";
    }

    QString sourceFile;

    if (!selectedFiles.isEmpty()) {
        // Use highlighted/selected file
        sourceFile = selectedFiles.first().filePath;
    } else {
        // Fall back to first enabled file
        auto enabledFiles = fileList->getEnabledFiles();
        if (!enabledFiles.isEmpty()) {
            sourceFile = enabledFiles.first().filePath;
        }
    }

    if (sourceFile.isEmpty()) {
        return "# No file available - please highlight or [✓] enable one from the INPUT File List";
    }
        
    // Get output file extension from OutputFilter
    QString outputExtension = "wav";  // Default
    if (filterChain) {
        auto outputFilter = filterChain->getFilter(filterChain->filterCount() - 1);
        if (auto* output = dynamic_cast<OutputFilter*>(outputFilter.get())) {
            outputExtension = output->getFileExtension();
        }
    }
    
    // Get muted filter positions
    auto mutedPositions = filterChainWidget->getMutedFilterPositions();
    
    // Update filter indices to ensure they're current
    if (filterChain) {
        filterChain->updateAudioInputIndices();
        filterChain->updateMultiInputFilterIndices();
    }
    
    // Collect sidechain files from all AudioInput filters (main chain + sub-chains)
    QStringList sidechainFiles;
    for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
        QString selectedFile = audioInput->getSelectedFilePath();
        sidechainFiles.append(selectedFile);
    }
    
    // Build the command - output to a placeholder
    QString outputFile = "OUTPUT." + outputExtension;
    
    // ========== USE FILTERCHAIN AS SOURCE OF TRUTH ==========
    // Build complete command including all aux outputs
    // View Command respects user's log settings from Settings > Log Level
    auto logSettings = LogSettings::fromQSettings();
    QString command = filterChain->buildCompleteCommand(
        sourceFile,
        sidechainFiles,
        outputFile,
        mutedPositions,
        logSettings
    );

    // Format for display with ffmpeg path and pretty printing
    return FilterChain::formatCommandForDisplay(command, ffmpegPath);
}

void MainWindow::onViewCommand() {
    // Create window if needed
    if (!commandViewWindow) {
        commandViewWindow = new CommandViewWindow(this);
    }
    
    // Build and display the command
    QString command = buildPreviewCommand();
    commandViewWindow->setCommand(command);
    
    // Show the window
    commandViewWindow->show();
    commandViewWindow->raise();
    commandViewWindow->activateWindow();
}

void MainWindow::onGeneratePreview() {
    // Update command viewer if it's open
    if (commandViewWindow && commandViewWindow->isVisible()) {
        commandViewWindow->setCommand(buildPreviewCommand());
    }
    
   // Get selected file from file list
    auto fileList = inputPanel->getFileListWidget();
    auto selectedFiles = fileList->getSelectedFiles();

    qDebug() << "=== PREVIEW DEBUG ===";
    qDebug() << "Selected files count:" << selectedFiles.size();

    QString sourceFile;

    if (selectedFiles.size() > 1) {
        QMessageBox::warning(this, "Multiple Files Selected",
            "Please highlight only one audio file from the Input > File List.");
        return;
    }

    if (!selectedFiles.isEmpty()) {
        // Use highlighted/selected file
        sourceFile = selectedFiles.first().filePath;
    } else {
        // Fall back to first enabled file
        auto enabledFiles = fileList->getEnabledFiles();
        if (!enabledFiles.isEmpty()) {
            sourceFile = enabledFiles.first().filePath;
        }
    }

    if (sourceFile.isEmpty()) {
        QMessageBox::warning(this, "No File Available",
            "Please select or enable a file from the File List to preview.");
        return;
    }
        
//    QString sourceFile = selectedFiles.first().filePath;
    
    // Get output settings from OutputFilter
    QString outputFormat = "wav";
    int sampleRate = 48000;
    int bitDepth = 24;
    
    if (filterChain) {
        auto outputFilter = filterChain->getFilter(filterChain->filterCount() - 1);
        if (auto* output = dynamic_cast<OutputFilter*>(outputFilter.get())) {
            outputFormat = output->getFileExtension();
            // Note: OutputFilter now handles all codec generation internally
            // For preview, we just need the extension for temp file naming
        }
    }
    
    // Get muted filter positions
    auto mutedPositions = filterChainWidget->getMutedFilterPositions();
    
    // Update filter indices to ensure they're current
    if (filterChain) {
        filterChain->updateAudioInputIndices();
        filterChain->updateMultiInputFilterIndices();
    }
    
    // Collect sidechain files from all AudioInput filters (main chain + sub-chains)
    // For PREVIEW: use highlighted/selected file (same behavior as main Input File List)
    // IMPORTANT: Always include an entry for each AudioInput (even if empty) to keep indices stable
    QStringList sidechainFiles;
    for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
        QString selectedFile = audioInput->getSelectedFilePath();
        sidechainFiles.append(selectedFile);  // May be empty if nothing selected/enabled
    }
    
    qDebug() << "Generating preview for:" << sourceFile;
    qDebug() << "With sidechain files:" << sidechainFiles;
    
    // Start preview generation
    // Use higher resolution waveform when Region Preview window is open
    QString waveformSize = regionWindowIsActive() ? "3000x2000" : "2000x160";
    
    previewGenerator->generate(
        sourceFile,
        outputFormat,
        sampleRate,
        bitDepth,
        filterChain,
        mutedPositions,
        sidechainFiles,
        ffmpegPath,
        waveformSize
    );
}

void MainWindow::onPreviewStarted() {
    scanProgressBar->setVisible(true);
    scanProgressBar->setRange(0, 100);
    scanProgressBar->setValue(0);
    statusLabel->setText("Generating preview...");
    
    qDebug() << "Preview generation started";
}

void MainWindow::onPreviewProgress(int percent, const QString& stage) {
    scanProgressBar->setValue(percent);
    statusLabel->setText(stage);
    QCoreApplication::processEvents();  // Keep UI responsive
}

void MainWindow::onPreviewFinished(const QString& audioFile, const QString& waveformFile) {
    scanProgressBar->setVisible(false);
    statusLabel->setText(audioFile);
    statusLabel->setStyleSheet("QLabel { font-size: 9px ;}");
    
    // Load preview into both waveform widgets
    waveformPreview->setPreviewFile(audioFile, waveformFile);
    regionPreviewWindow->setPreviewFile(audioFile, waveformFile);
    
    qDebug() << "Preview finished:" << audioFile << waveformFile;
}

void MainWindow::onPreviewError(const QString& message) {
    scanProgressBar->setVisible(false);
    statusLabel->setText("Preview generation failed");
    
    QMessageBox::critical(this, "Preview Error", 
        "Failed to generate preview:\n\n" + message);
    
    qWarning() << "Preview error:" << message;
}

// ========== FFMPEG DETECTION ==========

bool MainWindow::regionWindowIsActive() const {
    return regionPreviewWindow && regionPreviewWindow->isVisible();
}

void MainWindow::checkFFmpegAvailability() {
    auto paths = FFmpegDetector::detect();
    
    if (paths.isValid()) {
        // FFmpeg found
        ffmpegPath = paths.ffmpeg;
        ffprobePath = paths.ffprobe;
        // ffplayPath = paths.ffplay;  // uncomment if you add this member
        
        ffmpegStatusLabel->setText(QString("%1/ffmpeg <span style='padding-top:2px; color: #4eb74d;'>⬤</span>")
                                   .arg(QFileInfo(ffmpegPath).path()));

        qDebug() << "FFmpeg available:" << ffmpegPath;
        qDebug() << "FFprobe available:" << ffprobePath;
        
    } else {
        // FFmpeg NOT found — show setup dialog
        ffmpegStatusLabel->setText("FFmpeg not found <span style='padding-top:2px; color: #d42424;'>⬤</span>");
        
        FFmpegMissingDialog dialog(this);
        dialog.exec();
        
        switch (dialog.userChoice()) {
            case FFmpegMissingDialog::Installed: {
                // USE THE PATH DIRECTLY — don't rely on QSettings re-detect
                QString installed = dialog.installedPath();
                if (!installed.isEmpty() && QFile::exists(installed)) {
                    ffmpegPath = installed;
                    
                    // Derive ffprobe from same directory
                    QFileInfo fi(installed);
                    QString probeCandidate = fi.absolutePath() + "/ffprobe";
                    if (QFile::exists(probeCandidate))
                        ffprobePath = probeCandidate;
                    
                    ffmpegStatusLabel->setText(
                        QString("%1/ffmpeg <span style='padding-top:2px; color: #4eb74d;'>⬤</span>")
                            .arg(fi.absolutePath()));
                    
                    qDebug() << "FFmpeg installed to:" << ffmpegPath;
                    qDebug() << "FFprobe:" << (ffprobePath.isEmpty() ? "not found" : ffprobePath);
                } else {
                    // Fallback: try full re-detection
                    checkFFmpegAvailability();
                }
                return;
            }
                
            case FFmpegMissingDialog::OpenSettings:
                // User wants to manually enter a path
                onShowSettings();
                // After settings close, re-check
                checkFFmpegAvailability();
                return;
                
            case FFmpegMissingDialog::Cancelled:
            default:
                // User dismissed — disable processing
                processButton->setEnabled(false);
                processButton->setToolTip("FFmpeg is not installed.\nUse Help > Install FFmpeg... to set it up.");
                qWarning() << "FFmpeg not found — processing disabled.";
                break;
        }
    }
}

// ========== OPEN / SAVE MENU ACTIONS ==========

void MainWindow::onSave() {
    // If we have a current preset path, save to it directly
    if (!currentPresetPath.isEmpty()) {
        QString presetName = QFileInfo(currentPresetPath).baseName();
        
        if (presetManager->savePreset(currentPresetPath, presetName, 
                                      filterChain.get(), inputPanel, 
                                      outputSettingsPanel, filterChainWidget, 
                                      true)) {  // Don't include file lists for quick save
            statusLabel->setText("Preset saved: " + QFileInfo(currentPresetPath).fileName());
            qDebug() << "Preset saved to:" << currentPresetPath;
        } else {
            QMessageBox::warning(this, "Save Failed", 
                               "Failed to save preset to:\n" + currentPresetPath);
        }
    } else {
        // No current preset - use Save As
        onSaveAs();
    }
}

void MainWindow::onSaveAs() {
    // Create custom dialog for preset saving
    QDialog dialog(this);
    dialog.setWindowTitle("Save Preset");
    dialog.resize(500, 200);
    
    auto layout = new QVBoxLayout(&dialog);
    
    // Preset name field
    auto nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Preset Name:"));
    auto nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("My Filter Chain");
    nameLayout->addWidget(nameEdit);
    layout->addLayout(nameLayout);
    
    // Include file lists checkbox
    auto includeFilesCheckbox = new QCheckBox("Save preset with File List(s)?");
    includeFilesCheckbox->setChecked(true);  // Default: true / Include
    layout->addWidget(includeFilesCheckbox);
    
    layout->addWidget(new QLabel("<small><i>Note: File List(s) include all paths & selected states<br>"
                                           "for Main INPUT files plus all SideChain / Audio Input(s).<br>"
                                           "Files must exist at the same location when re-loading.</i></small>"));
    
    // Dialog buttons
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString presetName = nameEdit->text().trimmed();
        if (presetName.isEmpty()) {
            presetName = "Untitled";
        }
        
        // Get save location
        QString defaultDir = Preferences::instance().lastPresetDirectory();
        QString defaultPath = defaultDir + "/" + presetName + ".ffabjson";
        
        QString filepath = QFileDialog::getSaveFileName(
            this,
            "Save Preset As",
            defaultPath,
            "FFAB Presets (*.ffabjson);;All Files (*)"
        );
        
        if (!filepath.isEmpty()) {
            // Ensure .ffabjson extension
            if (!filepath.endsWith(".ffabjson", Qt::CaseInsensitive)) {
                filepath += ".ffabjson";
            }
            
            bool includeFiles = includeFilesCheckbox->isChecked();
            
            if (presetManager->savePreset(filepath, presetName, 
                                         filterChain.get(), inputPanel, 
                                         outputSettingsPanel, filterChainWidget, 
                                         includeFiles)) {
                currentPresetPath = filepath;
                Preferences::instance().setLastPresetDirectory(QFileInfo(filepath).absolutePath());
                Preferences::instance().addRecentPreset(filepath);
                statusLabel->setText("Preset saved: " + QFileInfo(filepath).fileName());
                qDebug() << "Preset saved to:" << filepath;
            } else {
                QMessageBox::warning(this, "Save Failed", 
                                   "Failed to save preset to:\n" + filepath);
            }
        }
    }
}

void MainWindow::onOpen() {
    QString defaultDir = Preferences::instance().lastPresetDirectory();
    
    QString filepath = QFileDialog::getOpenFileName(
        this,
        "Load Preset",
        defaultDir,
        "FFAB Presets (*.ffabjson);;All Files (*)"
    );
    
    if (!filepath.isEmpty()) {
        PresetManager::MissingFilesInfo missingInfo;
        
        // Show progress during load
       scanProgressBar->setVisible(true);
       scanProgressBar->setRange(0, 100);
       scanProgressBar->setValue(0);
        statusLabel->setText("Loading preset...");
        
        // Progress callback
        auto progressCallback = [this](int current, int total, const QString& message) {
            if (total > 0) {
                int percent = (current * 100) / total;
               scanProgressBar->setValue(percent);
            }
            statusLabel->setText(message);
        };
        
        if (presetManager->loadPreset(filepath, filterChain.get(), inputPanel, 
                                      outputSettingsPanel, filterChainWidget, 
                                      missingInfo, progressCallback)) {
            currentPresetPath = filepath;
            Preferences::instance().setLastPresetDirectory(QFileInfo(filepath).absolutePath());
            Preferences::instance().addRecentPreset(filepath);
            
            // Refresh UI
            filterChainWidget->refreshChain();
            
            // Update chain indices
            filterChain->updateAudioInputIndices();
            filterChain->updateMultiInputFilterIndices();
            
            // Hide progress bar
           scanProgressBar->setVisible(false);
            statusLabel->setText("Ready");
            
            // Show success message
            QString message = "Preset loaded successfully!";
            
            if (missingInfo.totalFiles > 0) {
                message += QString("\n\nFiles: %1 found, %2 missing")
                          .arg(missingInfo.foundFiles)
                          .arg(missingInfo.missingFiles.size());
            }
            
            statusLabel->setText("Preset loaded: " + QFileInfo(filepath).fileName());
            
            // If there are missing files, write report and show dialog
            if (!missingInfo.missingFiles.isEmpty()) {
                PresetManager::writeMissingFilesReport(missingInfo);
                
                QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
                QString reportPath = docsPath + "/FFAB/FFAB_missing_files.txt";
                
                QMessageBox msgBox(this);
                msgBox.setWindowTitle("Missing Files");
                msgBox.setText(QString("%1 of %2 files could not be located.")
                              .arg(missingInfo.missingFiles.size())
                              .arg(missingInfo.totalFiles));
                msgBox.setInformativeText("A list of missing files has been saved.\n\n"
                                         "Click 'Open Report' to view the missing file list.");
                msgBox.setIcon(QMessageBox::Warning);
                
                QPushButton* openButton = msgBox.addButton("Open Report", QMessageBox::ActionRole);
                msgBox.addButton(QMessageBox::Ok);
                
                msgBox.exec();
                
                if (msgBox.clickedButton() == openButton) {
                    // Open the report file in default text editor
                    QDesktopServices::openUrl(QUrl::fromLocalFile(reportPath));
                }
            } else {
                QMessageBox::information(this, "Preset Loaded", message);
            }
            
            qDebug() << "Preset loaded from:" << filepath;
        } else {
            // Hide progress bar on error
           scanProgressBar->setVisible(false);
            statusLabel->setText("Failed to load preset");
            
            QMessageBox::critical(this, "Load Failed", 
                                "Failed to load preset from:\n" + filepath);
        }
    }
}

void MainWindow::onNew() {
    // Confirm if there's existing work
    if (filterChain->filterCount() > 2 || 
        inputPanel->getFileListWidget()->getAllFiles().size() > 0) {
        
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "New Document",
            "Clear current filter chain and file list?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        
        if (reply != QMessageBox::Yes) return;
    }
    
    // Clear filter chain (keep only INPUT/OUTPUT)
    while (filterChain->filterCount() > 2) {
        filterChain->removeFilter(1);
    }
    
    // Clear M/S states
    filterChainWidget->clearMuteStates();
    filterChainWidget->clearSoloStates();
    
    // Refresh chain widget
    filterChainWidget->refreshChain();
    
    // Clear file lists
    inputPanel->getFileListWidget()->clearFiles();
    
    // Clear any AudioInput file lists too (main chain + sub-chains)
    for (auto* audioInput : filterChain->getAllAudioInputFilters()) {
        if (auto* fileListWidget = audioInput->getFileListWidget()) {
            fileListWidget->clearFiles();
        }
    }
    
    // Reset UI to INPUT panel
    titleLabel->setText("Input Parameters");
    filterIdBadge->setVisible(false);
    stackedWidget->setCurrentIndex(0);
    
    // Reset status
    statusLabel->setText("Ready");
    
    // Disable process button
    processButton->setEnabled(false);
    processButton->setToolTip("Add file(s) to the INPUT panel\nbefore clicking Process Files");
    processButton->setText("Process Files");
}
void MainWindow::onShowBatchSettings() {
    if (!batchSettingsWindow) return;
    
    // Update context every time we show the window
    batchSettingsWindow->setContext(
        filterChain,
        batchProcessor,
        inputPanel,
        filterChainWidget,
        ffmpegPath
    );
    
    batchSettingsWindow->show();
    batchSettingsWindow->raise();
    batchSettingsWindow->activateWindow();
}

// Batch Processing and Job Builder

void MainWindow::onBatchProcessRequested(JobListBuilder::Algorithm algorithm,
                                          JobListBuilder::ZipMismatch zipMismatch) {
    // Validate output folder
    QString outputFolder;
    if (filterChain) {
        auto outputFilter = filterChain->getFilter(filterChain->filterCount() - 1);
        if (auto* output = dynamic_cast<OutputFilter*>(outputFilter.get())) {
            outputFolder = output->getOutputFolder();
        }
    }
    
    if (outputFolder.isEmpty()) {
        QMessageBox::warning(this, "No Output Folder",
            "Please select an output folder in the OUTPUT filter first.");
        return;
    }
    
    // Get enabled files
    auto mainFiles = inputPanel->getFileListWidget()->getEnabledFiles();
    if (mainFiles.isEmpty()) {
        QMessageBox::warning(this, "No Files",
            "Please add files to the main input list first.");
        return;
    }
    
    // Get muted positions and update indices
    auto mutedPositions = filterChainWidget->getMutedFilterPositions();
    filterChain->updateAudioInputIndices();
    filterChain->updateMultiInputFilterIndices();
    
    // Collect AudioInput filters
    auto audioInputs = filterChain->getAllAudioInputFilters();
    
    // Build "other sidechain files" baseline (empty strings for each AudioInput)
    QStringList baseSidechainFiles;
    for (auto* ai : audioInputs) {
        auto files = ai->getEnabledFilePaths();
        baseSidechainFiles.append(files.isEmpty() ? QString() : files.first());
    }
    
    // Get aux file lists for algorithms that need them
    QList<FileListWidget::AudioFileInfo> aux1Files;
    QList<FileListWidget::AudioFileInfo> aux2Files;
    int aux1InputIndex = 1;
    int aux2InputIndex = 2;
    QString aux1SelectedFile;
    
    if (!audioInputs.empty()) {
        aux1InputIndex = audioInputs[0]->getInputIndex();
        if (audioInputs[0]->getFileListWidget()) {
            aux1Files = audioInputs[0]->getFileListWidget()->getEnabledFiles();
        }
        aux1SelectedFile = audioInputs[0]->getSelectedFilePath();
    }
    if (audioInputs.size() >= 2) {
        aux2InputIndex = audioInputs[1]->getInputIndex();
        if (audioInputs[1]->getFileListWidget()) {
            aux2Files = audioInputs[1]->getFileListWidget()->getEnabledFiles();
        }
    }
    
    // Build jobs using JobListBuilder
    QList<BatchProcessor::JobInfo> jobs;
    
    switch (algorithm) {
        case JobListBuilder::Algorithm::Sequential:
            jobs = JobListBuilder::buildSequential(
                mainFiles, outputFolder, filterChain, mutedPositions, baseSidechainFiles);
            break;
            
        case JobListBuilder::Algorithm::Zip:
            jobs = JobListBuilder::buildZip(
                mainFiles, aux1Files, aux1InputIndex, outputFolder,
                filterChain, mutedPositions, baseSidechainFiles, zipMismatch);
            break;
            
        case JobListBuilder::Algorithm::BroadcastFixed:
            jobs = JobListBuilder::buildBroadcastFixed(
                mainFiles, aux1SelectedFile, aux1InputIndex, outputFolder,
                filterChain, mutedPositions, baseSidechainFiles);
            break;
            
        case JobListBuilder::Algorithm::BroadcastRandom:
            jobs = JobListBuilder::buildBroadcastRandom(
                mainFiles, aux1Files, aux1InputIndex, outputFolder,
                filterChain, mutedPositions, baseSidechainFiles);
            break;
            
        case JobListBuilder::Algorithm::Cartesian:
            jobs = JobListBuilder::buildCartesian(
                mainFiles, aux1Files, aux1InputIndex, outputFolder,
                filterChain, mutedPositions, baseSidechainFiles);
            break;
            
        case JobListBuilder::Algorithm::CartesianTriple:
            jobs = JobListBuilder::buildCartesianTriple(
                mainFiles, aux1Files, aux2Files,
                aux1InputIndex, aux2InputIndex, outputFolder,
                filterChain, mutedPositions, baseSidechainFiles);
            break;
            
        case JobListBuilder::Algorithm::Iterate:
            jobs = JobListBuilder::buildIterate(
                mainFiles,
                batchSettingsWindow->iterateRepeatCount(),
                batchSettingsWindow->iterateGainDb(),
                outputFolder, filterChain, mutedPositions, baseSidechainFiles);
            break;
    }
    
    if (jobs.isEmpty()) {
        QMessageBox::warning(this, "No Jobs",
            "No processing jobs could be generated. Check that input files "
            "and sidechain files are loaded for the selected algorithm.");
        return;
    }
    
    // Validate output count
    QString warning = JobListBuilder::validateOutputCount(jobs.size());
    if (warning.contains("exceeds the hard limit")) {
        QMessageBox::critical(this, "Too Many Files", warning);
        return;
    }
    
    // Get output format info for confirmation dialog
    QString formatInfo;
    auto lastFilter = filterChain->getFilter(filterChain->filterCount() - 1);
    if (auto* output = dynamic_cast<OutputFilter*>(lastFilter.get())) {
        formatInfo = output->getFileExtension().toUpper();
    }
    
    // Estimate size
    double avgDuration = 60.0;
    auto allMainFiles = inputPanel->getFileListWidget()->getEnabledFiles();
    if (!allMainFiles.isEmpty()) {
        double totalDur = 0.0;
        int validCount = 0;
        for (const auto& f : allMainFiles) {
            QStringList parts = f.duration.split(':');
            if (parts.size() >= 3) {
                double secs = parts[0].toDouble() * 3600 +
                              parts[1].toDouble() * 60 +
                              parts[2].toDouble();
                if (secs > 0) { totalDur += secs; validCount++; }
            }
        }
        if (validCount > 0) avgDuration = totalDur / validCount;
    }
    
    auto sizeEst = JobListBuilder::estimateSize(jobs.size(), avgDuration,
        formatInfo.isEmpty() ? "wav" : formatInfo.toLower());
    
    // Tier 1 confirmation
    int result = BatchConfirmDialog::confirmBatch(
        this,
        algorithm,
        mainFiles.size(),
        aux1Files.size(),
        aux2Files.size(),
        jobs.size(),
        sizeEst.formattedSize,
        outputFolder,
        formatInfo,
        (algorithm == JobListBuilder::Algorithm::BroadcastFixed && !aux1SelectedFile.isEmpty())
            ? QFileInfo(aux1SelectedFile).fileName() : QString());
    
    if (result != QDialog::Accepted) return;
    
    // Tier 2 confirmation for large batches
    if (jobs.size() > JobListBuilder::TIER2_THRESHOLD) {
        // Rough time estimate: assume 2x realtime average
        double estTimeSec = (avgDuration * jobs.size()) / 2.0;
        int estMinutes = static_cast<int>(estTimeSec / 60);
        QString timeEst;
        if (estMinutes >= 60) {
            timeEst = QString("~%1 hours at 2× realtime").arg(estMinutes / 60);
        } else {
            timeEst = QString("~%1 minutes at 2× realtime").arg(estMinutes);
        }
        
        int result2 = BatchConfirmDialog::confirmLargeBatch(
            this, jobs.size(), sizeEst.formattedSize, timeEst);
        
        if (result2 != QDialog::Accepted) return;
    }
    
    // Cancel any existing batch
    batchProcessor->cancel();
    
    // Start processing!
    qDebug() << "Starting batch:" << jobs.size() << "jobs, algorithm"
             << static_cast<int>(algorithm);
    
    batchProcessor->start(jobs, ffmpegPath);
}


// LICENSE TEXT POP_UP
QWidget* MainWindow::showLicenseWindow(const QString& title, const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not open license file:\n" + resourcePath);
        return nullptr;
    }
    QString text = QString::fromUtf8(file.readAll());
    file.close();

    if (lgplWindow) { lgplWindow->close(); lgplWindow = nullptr; }
    if (gplWindow)  { gplWindow->close();  gplWindow = nullptr; }
    if (oflWindow)  { oflWindow->close();  oflWindow = nullptr; }
    if (flWindow)  { flWindow->close();  flWindow = nullptr; }

    auto* win = new QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint);
    win->setAttribute(Qt::WA_DeleteOnClose);
    win->setWindowTitle(title);
    win->resize(620, 480);
    win->setStyleSheet("background-color: #2a2a2a;");

    // Null out the tracking pointer when closed
    QWidget** tracking = title.contains("LGPL") ? &lgplWindow
                   : title.contains("GPL")  ? &gplWindow
                   : title.contains("OFL") ? &oflWindow
                   : &flWindow;
    connect(win, &QWidget::destroyed, [tracking]() {
        *tracking = nullptr;
    });

    auto* layout = new QVBoxLayout(win);
    layout->setContentsMargins(1, 1, 1, 1);

    auto* titleBar = new QHBoxLayout();
    titleBar->setContentsMargins(10, 6, 10, 4);

    auto* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("color: #808080; font-size: 12px; background: transparent;");
    titleBar->addWidget(titleLabel);
    titleBar->addStretch();

    auto* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(24, 24);
    closeBtn->setStyleSheet(
        "QPushButton { color: #808080; background: transparent; border: none; font-size: 14px; }"
        "QPushButton:hover { color: #ffffff; }"
    );
    connect(closeBtn, &QPushButton::clicked, win, &QWidget::close);
    titleBar->addWidget(closeBtn);

    layout->addLayout(titleBar);

    auto* textEdit = new QPlainTextEdit();
    textEdit->setPlainText(text);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #2a2a2a;"
        "    color: #ffffff;"
        "    border: none;"
        "    font-family: 'Fira Code', monospace;"
        "    font-size: 11px;"
        "    padding: 8px;"
        "}"
        "QScrollBar:vertical {"
        "    background: #2a2a2a; width: 8px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #555555; border-radius: 4px;"
        "}"
    );
    layout->addWidget(textEdit);

    win->show();
    win->raise();
    win->activateWindow();
    return win;
}