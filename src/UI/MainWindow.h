#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <memory>
#include "Core/AppConfig.h"
#include "Core/FFmpegRunner.h"
#include "RotatedLabel.h"
#include "Utils/KeyCommands.h"
#include "Core/JobListBuilder.h"
#include "FFmpegSetupDialog.h"


// Forward declarations
class FilterChainWidget;
class FilterParamsPanel;
class InputPanel;
class OutputSettingsPanel;
class WaveformPreviewWidget;
class FilterChain;
class BatchProcessor;
class CommandViewWindow;
class PreviewGenerator;
class PresetManager;
class RotatedLabel;
class QHBoxLayout;
class RotatedLabel;
class BatchSettingsWindow;
class RegionPreviewWindow;
class LogViewWindow;
class UpdateChecker;
class QToolButton;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
protected:
    void changeEvent(QEvent* event) override;  // Handle dark mode changes
    void closeEvent(QCloseEvent* event) override;  // Save preferences on close
    
private slots:
    // Filter chain signals
    void onFilterSelected(int position);
    void onSubChainFilterSelected(int filterId);
    void onChainModified();
    
    // Input & Batch signals
    void onAddFolder();
    void onAddFiles();
    void onClearFiles();
    
    // Output signals
    void onOutputFolderChanged(const QString& path);
    
    // File list signals
    void onFileSelectionChanged();
    void onRescanMainFileList();
    void onRescanAudioInputFileList(class AudioInputFilter* audioInput, class FileListWidget* fileListWidget);
    
    // Preview generation signals
    void onGeneratePreview();
    void onViewCommand();
    void onPreviewStarted();
    void onPreviewProgress(int percent, const QString& stage);
    void onPreviewFinished(const QString& audioFile, const QString& waveformFile);
    void onPreviewError(const QString& message);
    
    // Process button
    void onProcessFiles();
    
    // Batch processor signals
    void onBatchStarted(int totalFiles);
    void onFileStarted(const QString& fileName, int fileNumber, int totalFiles);
    void onFileProgress(const FFmpegRunner::ProgressInfo& info);
    void onFileFinished(const QString& fileName, bool success);
    void onBatchFinished(int completed, int failed);
    // Complex batch job list
    void onShowBatchSettings();
    void onBatchProcessRequested(JobListBuilder::Algorithm algorithm,
                                 JobListBuilder::ZipMismatch zipMismatch);    
    
    // Menu actions
    void showAboutDialog();
    void onSave();
    void onSaveAs();
    void onOpen();
    void onNew();
    void onShowSettings();
    
private:
    void setupUI();
    void createMenuBar();
    void setupKeyCommands();
    void connectSignals();
    void connectAudioInputButtons(class AudioInputFilter* audioInput);
    QWidget* showLicenseWindow(const QString& title, const QString& resourcePath);
    QFutureWatcher<QList<FileListWidget::AudioFileInfo>>* m_scanWatcher = nullptr;

    // Core
    std::shared_ptr<FilterChain> filterChain;
    BatchProcessor* batchProcessor;
    class PreviewGenerator* previewGenerator;
    PresetManager* presetManager;
    
    // Preset state
    QString currentPresetPath;
    
    // UI Components
    FilterChainWidget* filterChainWidget;
    RotatedLabel* filterIdBadge = nullptr;
    QHBoxLayout* titleLayout = nullptr;
    
    // Right panel stacked widget
    QLabel* titleLabel;  // Dynamic title ("Input/Filter/Output Parameters")
    QLabel* docLinks;  // Links to ffmpeg / ayosec docs
    QStackedWidget* stackedWidget;  // Switches between 3 panels
    InputPanel* inputPanel;  // Index 0
    FilterParamsPanel* filterParamsPanel;  // Index 1
    OutputSettingsPanel* outputSettingsPanel;  // Index 2
    
    WaveformPreviewWidget* waveformPreview;
    
    // Process button (in filter chain)
    QPushButton* processButton;
    QProgressBar* progressBar;
    QLabel* statusLabel;
    QProgressBar* scanProgressBar;  // Subtle 6px scan progress
    
    // Paths
    QString currentOutputFolder;
    QString ffmpegPath;
    QString ffprobePath;
    
    // FFmpeg status LED
    QLabel* ffmpegStatusLabel;
    
    // Command viewer window
    CommandViewWindow* commandViewWindow = nullptr;

    // View Waveform Window
    RegionPreviewWindow* regionPreviewWindow = nullptr;

    // Batch processor window
    BatchSettingsWindow* batchSettingsWindow = nullptr;

    // Log view window
    LogViewWindow* logViewWindow = nullptr;

    // Update checker
    UpdateChecker* m_updateChecker = nullptr;
    QLabel*        m_updateLabel = nullptr;

    // Helper
    void checkFFmpegAvailability();
    bool regionWindowIsActive() const;
    QString buildPreviewCommand();  // Build FFmpeg command for preview

    // License file text windows
    QWidget* lgplWindow = nullptr;
    QWidget* gplWindow = nullptr;
    QWidget* oflWindow = nullptr;
    QWidget* flWindow = nullptr;
};

#endif // MAINWINDOW_H
