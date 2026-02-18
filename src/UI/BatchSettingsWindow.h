#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QStackedWidget>
#include <QElapsedTimer>
#include <QTimer>
#include <QToolButton>
#include <memory>
#include "Core/JobListBuilder.h"
#include "Core/FFmpegRunner.h"

class FilterChain;
class BatchProcessor;
class InputPanel;
class FilterChainWidget;

/**
 * BatchSettingsWindow - Modeless window for batch configuration and progress
 * 
 * Two views in a QStackedWidget:
 * 
 * 1. Settings View:
 *    - DX7-style algorithm selector (numbered buttons)
 *    - Algorithm diagram with node shapes (■ ● ▲)
 *    - Live output count + size estimate
 *    - Zip mismatch mode combo
 *    - Iterate settings (repeat count, gain reduction)
 *    - [Process Batch] button
 * 
 * 2. Progress View:
 *    - Algorithm summary
 *    - Overall progress bar (capped at ~200 visual updates)
 *    - Elapsed / remaining time
 *    - Speed (x realtime)
 *    - Succeeded / failed counts
 *    - [Pause] [Resume] [Cancel Batch] buttons
 * 
 * Behaviors:
 *    - Modeless (show() not exec()) — main UI stays interactive
 *    - Closing hides the window — batch continues in background
 *    - Re-openable from View > Batch Progress or status bar click
 *    - Transitions from Settings → Progress on batch start
 *    - Returns to Settings view when batch finishes
 */
class BatchSettingsWindow : public QDialog {
    Q_OBJECT
    
public:
    explicit BatchSettingsWindow(QWidget* parent = nullptr);
    ~BatchSettingsWindow() override;
    
    // Set references to the app's core objects (called once after construction)
    void setContext(std::shared_ptr<FilterChain> filterChain,
                    BatchProcessor* batchProcessor,
                    InputPanel* inputPanel,
                    FilterChainWidget* filterChainWidget,
                    const QString& ffmpegPath);
    
    // Update the live output count and size estimate
    // Called when file lists or algorithm change
    void refreshOutputEstimate();
    
    // Switch to progress view (called when batch starts)
    void showProgressView();
    
    // Switch to settings view (called when batch finishes)
    void showSettingsView();
    
    // Is currently showing the progress view?
    bool isShowingProgress() const;
    
    // Get the selected algorithm
    JobListBuilder::Algorithm selectedAlgorithm() const;
    
    // Get the selected zip mismatch mode
    JobListBuilder::ZipMismatch selectedZipMismatch() const;
    
    // Get iterate settings
    int iterateRepeatCount() const;
    double iterateGainDb() const;
    
signals:
    // Emitted when user clicks [Process Batch] — MainWindow handles job building + confirmation
    void processRequested(JobListBuilder::Algorithm algorithm,
                          JobListBuilder::ZipMismatch zipMismatch);
    
public slots:
    // Batch processor signal handlers
    void onBatchStarted(int totalFiles);
    void onFileStarted(const QString& fileName, int fileNumber, int totalFiles);
    void onFileProgress(const FFmpegRunner::ProgressInfo& info);
    void onFileFinished(const QString& fileName, bool success);
    void onBatchFinished(int completed, int failed);
    
protected:
    void closeEvent(QCloseEvent* event) override;
    
private slots:
    void onAlgorithmSelected(int algoIndex);
    void onProcessBatchClicked();
    void onPauseClicked();
    void onResumeClicked();
    void onCancelClicked();
    void updateETA();
    
private:
    void setupSettingsView();
    void setupProgressView();
    void updateAlgorithmDiagram();
    void updateZipMismatchVisibility();
    void updateIterateSettingsVisibility();
    
    // Get file counts from the live app state
    int getMainFileCount() const;
    int getAux1FileCount() const;
    int getAux2FileCount() const;
    
    // Context references (not owned)
    std::shared_ptr<FilterChain> filterChain;
    BatchProcessor* batchProcessor = nullptr;
    InputPanel* inputPanel = nullptr;
    FilterChainWidget* filterChainWidget = nullptr;
    QString ffmpegPath;
    
    // Top-level stack: settings vs progress
    QStackedWidget* mainStack = nullptr;
    
    // ===== Settings View Widgets =====
    QWidget* settingsPage = nullptr;
//    QPushButton* algoButtons[7] = {};   // DX7-style numbered buttons
    QToolButton* algoButtons[7] = {};
    QLabel* algoNameLabel = nullptr;
    QLabel* algoDiagramLabel = nullptr;
    QLabel* outputCountLabel = nullptr;
    QLabel* sizeEstimateLabel = nullptr;
    QComboBox* zipMismatchCombo = nullptr;
    QLabel* zipMismatchLabel = nullptr;
    QPushButton* processBatchButton = nullptr;
    
    // Iterate-specific widgets
    QLabel* iterateRepeatLabel = nullptr;
    QSpinBox* iterateRepeatSpin = nullptr;
    QLabel* iterateGainLabel = nullptr;
    QSlider* iterateGainSlider = nullptr;  // Range: -120 to 0 (tenths of dB, so -12.0 to 0.0)
    QLabel* iterateGainValueLabel = nullptr;
    QWidget* iterateSettingsWidget = nullptr;  // Container for show/hide
    
    int currentAlgoIndex = 0;  // 0-based (maps to Algorithm 1-7)
    
    // ===== Progress View Widgets =====
    QWidget* progressPage = nullptr;
    QLabel* progressAlgoLabel = nullptr;
    QLabel* progressCountLabel = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* progressDetailLabel = nullptr;
    QLabel* elapsedLabel = nullptr;
    QLabel* remainingLabel = nullptr;
    QLabel* speedLabel = nullptr;
    QLabel* succeededLabel = nullptr;
    QLabel* failedLabel = nullptr;
    QPushButton* pauseButton = nullptr;
    QPushButton* resumeButton = nullptr;
    QPushButton* cancelButton = nullptr;
    
    // Progress state
    int totalFiles = 0;
    int completedFiles = 0;
    int failedFiles = 0;
    QElapsedTimer elapsedTimer;
    QTimer* etaTimer = nullptr;
    int updateInterval = 1;  // Only repaint every N files (capped at ~200 updates)
};
