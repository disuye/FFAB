#include "BatchSettingsWindow.h"
#include "Core/FilterChain.h"
#include "Core/BatchProcessor.h"
#include "Filters/OutputFilter.h"
#include "Filters/AudioInputFilter.h"
#include "InputPanel.h"
#include "FileListWidget.h"
#include "FilterChainWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QCloseEvent>
#include <QLocale>
#include <QDebug>
#include <QSettings>
#include <QToolButton>

BatchSettingsWindow::BatchSettingsWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Batch Settings");
    setMinimumSize(500, 560);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    
    // Modeless — no exec(), just show()
    setModal(false);
    
    auto* topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(12, 12, 12, 12);
    topLayout->setSpacing(0);
    
    mainStack = new QStackedWidget();
    topLayout->addWidget(mainStack);
    
    setupSettingsView();
    setupProgressView();
    
    mainStack->setCurrentWidget(settingsPage);
    
    // ETA update timer (fires every second during processing)
    etaTimer = new QTimer(this);
    connect(etaTimer, &QTimer::timeout, this, &BatchSettingsWindow::updateETA);

    // Restore saved geometry
    restoreGeometry(QSettings().value("batchWindow/geometry").toByteArray());
}

BatchSettingsWindow::~BatchSettingsWindow() = default;

// ========== CONTEXT ==========

void BatchSettingsWindow::setContext(std::shared_ptr<FilterChain> chain,
                                    BatchProcessor* processor,
                                    InputPanel* input,
                                    FilterChainWidget* chainWidget,
                                    const QString& ffmpeg) {
    filterChain = chain;
    batchProcessor = processor;
    inputPanel = input;
    filterChainWidget = chainWidget;
    ffmpegPath = ffmpeg;
    
    refreshOutputEstimate();
}

// ========== SETTINGS VIEW ==========

void BatchSettingsWindow::setupSettingsView() {
    settingsPage = new QWidget();
    auto* layout = new QVBoxLayout(settingsPage);
    layout->setSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Algorithm selector: numbered buttons in a row (DX7 style)
    auto* algoGroup = new QGroupBox("Batch Algorithm");
    auto* algoLayout = new QVBoxLayout(algoGroup);
    algoLayout->setSpacing(8);
    
    auto* buttonsRow = new QHBoxLayout();
    buttonsRow->setSpacing(4);
    
    auto algos = JobListBuilder::getAllAlgorithms();
    for (int i = 0; i < 7; ++i) {
        algoButtons[i] = new QToolButton();
        algoButtons[i]->setText(QString::number(i + 1));
        // algoButtons[i]->setFixedSize(36, 28);
        algoButtons[i]->setCheckable(true);
        algoButtons[i]->setToolTip(algos[i].name);
        
        // Disable unavailable algorithms
        if (!algos[i].available) {
            algoButtons[i]->setEnabled(false);
            algoButtons[i]->setToolTip(algos[i].name + " (coming soon)");
        }
        
        int idx = i;
        connect(algoButtons[i], &QPushButton::clicked, [this, idx]() {
            onAlgorithmSelected(idx);
        });
        
        buttonsRow->addWidget(algoButtons[i]);
    }
    buttonsRow->addStretch();
    algoLayout->addLayout(buttonsRow);
    
    // Algorithm name
    algoNameLabel = new QLabel();
    QFont nameFont = algoNameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(nameFont.pointSize() + 1);
    algoNameLabel->setFont(nameFont);
    algoLayout->addWidget(algoNameLabel);
    
    // Algorithm diagram (ASCII art in monospace)
    algoDiagramLabel = new QLabel();
    QFont monoFont("Courier");
    monoFont.setPointSize(11);
    algoDiagramLabel->setFont(monoFont);
    algoDiagramLabel->setStyleSheet("QLabel { background-color: rgba(128,128,128,0.06); "
                                    "padding: 8px; border-radius: 4px; }");
    algoDiagramLabel->setMinimumHeight(80);
    algoLayout->addWidget(algoDiagramLabel);
    
    layout->addWidget(algoGroup);
    
    // Zip mismatch combo (only visible for Zip algorithm)
    auto* zipRow = new QHBoxLayout();
    zipMismatchLabel = new QLabel("List length mismatch:");
    zipMismatchCombo = new QComboBox();
    zipMismatchCombo->addItems({"Truncate to shortest", "Cycle shorter list", "Random from shorter"});
    connect(zipMismatchCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int) { refreshOutputEstimate(); });
    zipRow->addWidget(zipMismatchLabel);
    zipRow->addWidget(zipMismatchCombo);
    zipRow->addStretch();
    layout->addLayout(zipRow);
    
    // Iterate settings (only visible for Iterate algorithm)
    iterateSettingsWidget = new QWidget();
    auto* iterLayout = new QVBoxLayout(iterateSettingsWidget);
    iterLayout->setContentsMargins(0, 0, 0, 0);
    iterLayout->setSpacing(6);
    
    // Repeat count row
    auto* repeatRow = new QHBoxLayout();
    iterateRepeatLabel = new QLabel("Repeat count:");
    iterateRepeatSpin = new QSpinBox();
    iterateRepeatSpin->setRange(JobListBuilder::ITERATE_MIN_REPEATS,
                                 JobListBuilder::ITERATE_MAX_REPEATS);
    iterateRepeatSpin->setValue(JobListBuilder::ITERATE_DEFAULT_REPEATS);
    iterateRepeatSpin->setToolTip("Number of times to re-process each file (2–50)");
    connect(iterateRepeatSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int) { refreshOutputEstimate(); });
    repeatRow->addWidget(iterateRepeatLabel);
    repeatRow->addWidget(iterateRepeatSpin);
    repeatRow->addStretch();
    iterLayout->addLayout(repeatRow);
    
    // Gain reduction row
    auto* gainRow = new QHBoxLayout();
    iterateGainLabel = new QLabel("Gain per pass:");
    iterateGainSlider = new QSlider(Qt::Horizontal);
    iterateGainSlider->setRange(-120, 0);  // Tenths of dB: -12.0 to 0.0
    iterateGainSlider->setValue(static_cast<int>(JobListBuilder::ITERATE_DEFAULT_GAIN_DB * 10));
    iterateGainSlider->setSingleStep(5);   // 0.5 dB steps
    iterateGainSlider->setPageStep(10);    // 1.0 dB page steps
    iterateGainSlider->setToolTip("Per-iteration volume reduction to prevent clipping.\n"
                                   "0 dB = no reduction (chaos mode)\n"
                                   "-3 dB = recommended default\n"
                                   "-12 dB = aggressive reduction");
    
    iterateGainValueLabel = new QLabel();
    iterateGainValueLabel->setMinimumWidth(55);
    
    auto updateGainLabel = [this]() {
        double db = iterateGainSlider->value() / 10.0;
        if (qFuzzyCompare(db, 0.0)) {
            iterateGainValueLabel->setText("0 dB ☠");  // Skull for chaos mode
        } else {
            iterateGainValueLabel->setText(QString("%1 dB").arg(db, 0, 'f', 1));
        }
    };
    connect(iterateGainSlider, &QSlider::valueChanged, this, updateGainLabel);
    updateGainLabel();  // Set initial text
    
    gainRow->addWidget(iterateGainLabel);
    gainRow->addWidget(iterateGainSlider, 1);
    gainRow->addWidget(iterateGainValueLabel);
    iterLayout->addLayout(gainRow);
    
    layout->addWidget(iterateSettingsWidget);
    
    // Output estimate
    auto* estimateGroup = new QGroupBox("Output Estimate");
    auto* estLayout = new QVBoxLayout(estimateGroup);
    
    outputCountLabel = new QLabel("Output: —");
    QFont countFont = outputCountLabel->font();
    countFont.setBold(true);
    outputCountLabel->setFont(countFont);
    estLayout->addWidget(outputCountLabel);
    
    sizeEstimateLabel = new QLabel("Est. size: —");
    estLayout->addWidget(sizeEstimateLabel);
    
    layout->addWidget(estimateGroup);
    
    layout->addStretch();
    
    // Process button
    processBatchButton = new QPushButton("Process Batch");
    // processBatchButton->setMinimumHeight(36);
    QFont btnFont = processBatchButton->font();
    // btnFont.setBold(true);
    // processBatchButton->setFont(btnFont);
    connect(processBatchButton, &QPushButton::clicked, this, &BatchSettingsWindow::onProcessBatchClicked);
    layout->addWidget(processBatchButton);
    
    mainStack->addWidget(settingsPage);
    
    // Select algorithm 1 by default
    onAlgorithmSelected(0);
}

// ========== PROGRESS VIEW ==========

void BatchSettingsWindow::setupProgressView() {
    progressPage = new QWidget();
    auto* layout = new QVBoxLayout(progressPage);
    layout->setSpacing(8);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // Algorithm summary
    progressAlgoLabel = new QLabel();
    QFont algoFont = progressAlgoLabel->font();
    algoFont.setBold(true);
    progressAlgoLabel->setFont(algoFont);
    layout->addWidget(progressAlgoLabel);
    
    layout->addSpacing(4);
    
    // Progress counter: "42 / 120" during processing, summary at end
    progressDetailLabel = new QLabel();
    QFont detailFont = progressDetailLabel->font();
    detailFont.setPointSize(detailFont.pointSize() + 4);
    progressDetailLabel->setFont(detailFont);
    layout->addWidget(progressDetailLabel);
    
    // Slim progress bar — matches MainWindow scan bar
    progressBar = new QProgressBar();
    progressBar->setFixedHeight(4);
    progressBar->setTextVisible(false);
    progressBar->setStyleSheet(
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
    layout->addWidget(progressBar);
    
    layout->addSpacing(8);
    
    // Time + speed: compact horizontal row
    auto* timeGrid = new QGridLayout();
    timeGrid->setSpacing(4);
    
    timeGrid->addWidget(new QLabel("Elapsed:"), 0, 0);
    elapsedLabel = new QLabel("0:00");
    elapsedLabel->setStyleSheet("font-weight: bold;");
    timeGrid->addWidget(elapsedLabel, 0, 1);
    
    timeGrid->addWidget(new QLabel("Remaining:"), 0, 2);
    remainingLabel = new QLabel("—");
    remainingLabel->setStyleSheet("font-weight: bold;");
    timeGrid->addWidget(remainingLabel, 0, 3);
    
    // timeGrid->addWidget(new QLabel("Speed:"), 1, 0);
    // speedLabel = new QLabel("—");
    // speedLabel->setStyleSheet("font-weight: bold;");
    // timeGrid->addWidget(speedLabel, 1, 1);
    
    timeGrid->setColumnStretch(1, 1);
    timeGrid->setColumnStretch(3, 1);
    layout->addLayout(timeGrid);
    
    layout->addSpacing(4);

    // Per-worker instance section (hidden when N=1, populated at batch start)
    m_instanceHeaderLabel = new QLabel("FFmpeg Instances:");
    // m_instanceHeaderLabel->setStyleSheet("font-weight: bold;");
    m_instanceHeaderLabel->setVisible(false);
    layout->addWidget(m_instanceHeaderLabel);

    m_instanceSection = new QWidget();
    m_instanceLayout  = new QVBoxLayout(m_instanceSection);
    m_instanceLayout->setContentsMargins(0, 2, 0, 2);
    m_instanceLayout->setSpacing(3);
    m_instanceSection->setVisible(false);
    layout->addWidget(m_instanceSection);

    m_instanceFooterLabel = new QLabel(
        "<small><font color='#808080'>Adjust the number of FFmpeg Instances in Settings \u2192 Processing</font></small>");
    m_instanceFooterLabel->setVisible(false);
    layout->addWidget(m_instanceFooterLabel);

    layout->addSpacing(4);

    // Results — hidden during processing, shown at completion (only if failures)
    auto* resultsRow = new QHBoxLayout();
    succeededLabel = new QLabel();
    succeededLabel->setStyleSheet("color: #4CAF50;");
    succeededLabel->setVisible(false);
    resultsRow->addWidget(succeededLabel);
    
    failedLabel = new QLabel();
    failedLabel->setStyleSheet("color: #F44336;");
    failedLabel->setVisible(false);
    resultsRow->addWidget(failedLabel);
    resultsRow->addStretch();
    layout->addLayout(resultsRow);
    
    // progressCountLabel not used — kept as hidden dummy for signal compatibility
    progressCountLabel = new QLabel();
    progressCountLabel->setVisible(false);
    
    layout->addStretch();
    
    // Control buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    pauseButton = new QPushButton("Pause");
    pauseButton->setMinimumWidth(80);
    connect(pauseButton, &QPushButton::clicked, this, &BatchSettingsWindow::onPauseClicked);
    btnLayout->addWidget(pauseButton);
    
    resumeButton = new QPushButton("Resume");
    resumeButton->setMinimumWidth(80);
    resumeButton->setVisible(false);
    connect(resumeButton, &QPushButton::clicked, this, &BatchSettingsWindow::onResumeClicked);
    btnLayout->addWidget(resumeButton);
    
    cancelButton = new QPushButton("Cancel Batch");
    cancelButton->setMinimumWidth(100);
    connect(cancelButton, &QPushButton::clicked, this, &BatchSettingsWindow::onCancelClicked);
    btnLayout->addWidget(cancelButton);
    
    layout->addLayout(btnLayout);
    
    mainStack->addWidget(progressPage);
}

// ========== ALGORITHM SELECTION ==========

void BatchSettingsWindow::onAlgorithmSelected(int algoIndex) {
    currentAlgoIndex = algoIndex;
    
    // Update button checked states (exclusive selection)
    for (int i = 0; i < 7; ++i) {
        algoButtons[i]->setChecked(i == algoIndex);
    }
    
    auto info = JobListBuilder::getAlgorithmInfo(selectedAlgorithm());
    algoNameLabel->setText(QString("Algorithm %1: %2").arg(algoIndex + 1).arg(info.name));
    
    updateAlgorithmDiagram();
    updateZipMismatchVisibility();
    updateIterateSettingsVisibility();
    refreshOutputEstimate();
}

void BatchSettingsWindow::updateAlgorithmDiagram() {
    int M = getMainFileCount();
    int A1 = getAux1FileCount();
    int A2 = getAux2FileCount();
    
    QLocale loc;
    auto algo = selectedAlgorithm();
    int outCount = JobListBuilder::calculateOutputCount(algo, M, A1, A2, selectedZipMismatch(),
                                                        iterateRepeatCount());
    
    QString diagram;
    
    switch (algo) {
        case JobListBuilder::Algorithm::Sequential:
            diagram = QString(
                "  ┌───┐\n"
                "  │ ■ │  Main Input: %1 file(s)\n"
                "  └─┬─┘\n"
                "  ┌─┴─┐\n"
                "  │OUT│  → %2 files\n"
                "  └───┘\n\n"
                "■ = all from list, sequential\n"
            ).arg(loc.toString(M)).arg(loc.toString(outCount));
            break;
            
        case JobListBuilder::Algorithm::Iterate: {
            int R = iterateRepeatSpin ? iterateRepeatSpin->value() : JobListBuilder::ITERATE_DEFAULT_REPEATS;
            double gain = iterateGainSlider ? iterateGainSlider->value() / 10.0 : JobListBuilder::ITERATE_DEFAULT_GAIN_DB;
            QString gainStr = qFuzzyCompare(gain, 0.0) ? "0 dB" : QString("%1 dB").arg(gain, 0, 'f', 1);
            diagram = QString(
                "  ┌───┐\n"
                "  │ ■ │  Main Input: %1 file(s)\n"
                "  └─┬─┘\n"
                "  ┌─┴─┐  Gain: %4/pass\n"
                "  │OUT│─┐\n"
                "  └───┘ │\n"
                "  ┌─────┘\n"
                "  │ ×%3 passes\n"
                "  └───┘  → %2 files\n\n"
                "■ = all from list, sequential\n"
                "Each output feeds back as input.\n"
                "All intermediate files kept.\n"
            ).arg(loc.toString(M)).arg(loc.toString(outCount)).arg(R).arg(gainStr);
            break;
        }
            
        case JobListBuilder::Algorithm::Zip:
            diagram = QString(
                "  ┌───┐ ┌───┐\n"
                "  │ ■ │ │ ■ │  Main Input: %1 file(s)\n"
                "  │ M │ │ I │  + Audio Input: %2 file(s)\n"              
                "  └─┬─┘ └─┬─┘ \n"
                "    │     │\n"
                "  ┌─┴─────┴─┐\n"
                "  │   OUT   │  → %3 files\n"
                "  └─────────┘\n\n"
                "■ = all from list, sequential\n"
            ).arg(M).arg(A1).arg(loc.toString(outCount));
            break;
            
        case JobListBuilder::Algorithm::BroadcastFixed:
            diagram = QString(
                "  ┌───┐ ┌───┐\n"
                "  │ ■ │ │ ● │  Main Input: %1 file(s)\n"
                "  │ M │ │ I │  × Audio Input: choose 1\n"
                "  └─┬─┘ └─┬─┘\n"
                "    │     │\n"
                "  ┌─┴─────┴─┐\n"
                "  │   OUT   │  → %2 files\n"
                "  └─────────┘\n\n"
                "■ = all from list, sequential\n"
                "● = one from list, selected\n"
            ).arg(M).arg(loc.toString(outCount));
            break;
            
        case JobListBuilder::Algorithm::BroadcastRandom:
            diagram = QString(
                "  ┌───┐ ┌───┐\n"
                "  │ ■ │ │ ▲ │  Main Input: %1 file(s)\n"
                "  │ M │ │ I │  × Audio Input: random pairing\n"
                "  └─┬─┘ └─┬─┘\n"
                "    │     │\n"
                "  ┌─┴─────┴─┐\n"
                "  │   OUT   │  → %2 files\n"
                "  └─────────┘\n\n"
                "■ = all from list, sequential\n"
                "▲ = random every time\n"    
            ).arg(M).arg(loc.toString(outCount));
            break;
            
        case JobListBuilder::Algorithm::Cartesian:
            diagram = QString(
                "  ┌───┐ ┌───┐\n"
                "  │ ■ │ │ ■ │  Main Input: %1 file(s)\n"
                "  │ M │ │ I │  × Audio Input: %2 file(s)\n"
                "  └─┬─┘ └─┬─┘ \n"
                "    │  ×  │\n"
                "  ┌─┴─────┴─┐\n"
                "  │   OUT   │  → %3 files\n"
                "  └─────────┘\n\n"
                "■ = all from list, multiply\n"
            ).arg(M).arg(A1).arg(loc.toString(outCount));
            break;
            
        case JobListBuilder::Algorithm::CartesianTriple:
            diagram = QString(
                "  ┌───┐ ┌───┐ ┌───┐\n"
                "  │ ■ │ │ ■ │ │ ■ │  Main Input: %1 file(s)\n"
                "  │ M │ │ A │ │ B │  × Audio Input A: %2 file(s)\n"
                "  └─┬─┘ └─┬─┘ └─┬─┘  × Audio Input B: %3 file(s)\n"
                "    │  ×  │  ×  │\n"
                "  ┌─┴─────┴─────┴─┐\n"
                "  │      OUT      │  → %4 files\n"
                "  └───────────────┘\n\n"
                "■ = all from list, multiply\n"
            ).arg(M).arg(A1).arg(A2).arg(loc.toString(outCount));
            break;
    }
    
    algoDiagramLabel->setText(diagram);
}

void BatchSettingsWindow::updateZipMismatchVisibility() {
    bool show = (selectedAlgorithm() == JobListBuilder::Algorithm::Zip);
    zipMismatchLabel->setVisible(show);
    zipMismatchCombo->setVisible(show);
}

void BatchSettingsWindow::updateIterateSettingsVisibility() {
    bool show = (selectedAlgorithm() == JobListBuilder::Algorithm::Iterate);
    iterateSettingsWidget->setVisible(show);
}

// ========== LIVE ESTIMATE ==========

void BatchSettingsWindow::refreshOutputEstimate() {
    auto algo = selectedAlgorithm();
    int M = getMainFileCount();
    int A1 = getAux1FileCount();
    int A2 = getAux2FileCount();
    
    int outCount = JobListBuilder::calculateOutputCount(algo, M, A1, A2, selectedZipMismatch(),
                                                        iterateRepeatCount());
    
    QLocale loc;
    outputCountLabel->setText(QString("Output: %1 files").arg(loc.toString(outCount)));
    
    // Estimate size using output filter settings
    if (filterChain && outCount > 0) {
        auto lastFilter = filterChain->getFilter(filterChain->filterCount() - 1);
        auto* output = dynamic_cast<OutputFilter*>(lastFilter.get());
        
        QString format = output ? output->getFileExtension() : "wav";
        
        // Estimate average duration from main file list
        double avgDuration = 60.0;  // Default guess: 1 minute
        if (inputPanel) {
            auto files = inputPanel->getFileListWidget()->getEnabledFiles();
            if (!files.isEmpty()) {
                // Parse duration strings and average them
                double totalDuration = 0.0;
                int validCount = 0;
                for (const auto& f : files) {
                    // Duration format: "00:03:45" or "00:00:12.34"
                    QStringList parts = f.duration.split(':');
                    if (parts.size() >= 3) {
                        double secs = parts[0].toDouble() * 3600 +
                                      parts[1].toDouble() * 60 +
                                      parts[2].toDouble();
                        if (secs > 0) {
                            totalDuration += secs;
                            validCount++;
                        }
                    }
                }
                if (validCount > 0) {
                    avgDuration = totalDuration / validCount;
                }
            }
        }
        
        auto est = JobListBuilder::estimateSize(outCount, avgDuration, format);
        sizeEstimateLabel->setText(QString("Est. size: %1").arg(est.formattedSize));
    } else {
        sizeEstimateLabel->setText("Est. size: —");
    }
    
    // Update diagram too (counts might have changed)
    updateAlgorithmDiagram();
    
    // Enable/disable process button
    bool canProcess = (outCount > 0);
    auto algoInfo = JobListBuilder::getAlgorithmInfo(algo);
    if (!algoInfo.available) canProcess = false;
    processBatchButton->setEnabled(canProcess);
}

int BatchSettingsWindow::getMainFileCount() const {
    if (!inputPanel) return 0;
    return inputPanel->getFileListWidget()->getEnabledFiles().size();
}

int BatchSettingsWindow::getAux1FileCount() const {
    if (!filterChain) return 0;
    auto audioInputs = filterChain->getAllAudioInputFilters();
    if (audioInputs.empty()) return 0;
    return audioInputs[0]->getEnabledFilePaths().size();
}

int BatchSettingsWindow::getAux2FileCount() const {
    if (!filterChain) return 0;
    auto audioInputs = filterChain->getAllAudioInputFilters();
    if (audioInputs.size() < 2) return 0;
    return audioInputs[1]->getEnabledFilePaths().size();
}

// ========== PROCESS ==========

void BatchSettingsWindow::onProcessBatchClicked() {
    emit processRequested(selectedAlgorithm(), selectedZipMismatch());
}

// ========== VIEW SWITCHING ==========

void BatchSettingsWindow::showProgressView() {
    // Reset progress state
    completedFiles = 0;
    failedFiles = 0;
    
    succeededLabel->setVisible(false);
    failedLabel->setVisible(false);
    progressBar->setValue(0);
    progressDetailLabel->setText("0 / —");
    elapsedLabel->setText("0:00");
    remainingLabel->setText("—");
    // speedLabel->setText("—");
    
    pauseButton->setVisible(true);
    resumeButton->setVisible(false);
    cancelButton->setText("Cancel Batch");
    
    // Show algo info in progress view
    auto info = JobListBuilder::getAlgorithmInfo(selectedAlgorithm());
    progressAlgoLabel->setText(QString("Algorithm %1: %2")
        .arg(static_cast<int>(selectedAlgorithm()))
        .arg(info.name));
    
    mainStack->setCurrentWidget(progressPage);
    setWindowTitle("Batch Processing");
}

void BatchSettingsWindow::showSettingsView() {
    etaTimer->stop();
    mainStack->setCurrentWidget(settingsPage);
    setWindowTitle("Batch Settings");
    refreshOutputEstimate();
}

bool BatchSettingsWindow::isShowingProgress() const {
    return mainStack->currentWidget() == progressPage;
}

JobListBuilder::Algorithm BatchSettingsWindow::selectedAlgorithm() const {
    return static_cast<JobListBuilder::Algorithm>(currentAlgoIndex + 1);
}

JobListBuilder::ZipMismatch BatchSettingsWindow::selectedZipMismatch() const {
    return static_cast<JobListBuilder::ZipMismatch>(zipMismatchCombo->currentIndex());
}

int BatchSettingsWindow::iterateRepeatCount() const {
    return iterateRepeatSpin ? iterateRepeatSpin->value() : JobListBuilder::ITERATE_DEFAULT_REPEATS;
}

double BatchSettingsWindow::iterateGainDb() const {
    return iterateGainSlider ? iterateGainSlider->value() / 10.0 : JobListBuilder::ITERATE_DEFAULT_GAIN_DB;
}

// ========== BATCH PROGRESS SLOTS ==========

void BatchSettingsWindow::onBatchStarted(int total) {
    totalFiles = total;
    completedFiles = 0;
    failedFiles = 0;

    // Cap visual updates at ~200
    updateInterval = qMax(1, total / 200);

    QLocale loc;
    progressDetailLabel->setText(QString("0 / %1").arg(loc.toString(total)));
    progressBar->setRange(0, total);
    progressBar->setValue(0);

    // Hide results until completion
    succeededLabel->setVisible(false);
    failedLabel->setVisible(false);

    // Build per-worker rows
    // Clear old rows from layout
    while (QLayoutItem* item = m_instanceLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    m_workerRows.clear();

    m_batchWorkerCount = QSettings().value("processing/maxConcurrent", 1).toInt();
    m_batchWorkerCount = qMax(1, m_batchWorkerCount);

    static const QString kBarStyle =
        "QProgressBar {"
        "    border: none;"
        "    background-color: #404040;"
        "    border-radius: 2px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: rgba(255, 85, 0, 0.80);"
        "    border-radius: 2px;"
        "}";

    for (int i = 0; i < m_batchWorkerCount; ++i) {
        auto* row    = new QWidget();
        auto* rowLay = new QHBoxLayout(row);
        rowLay->setContentsMargins(0, 0, 0, 0);
        rowLay->setSpacing(6);

        auto* numLabel = new QLabel(QString("%1").arg(i + 1, 2, 10, QChar('0')));
        numLabel->setFixedWidth(20);
        numLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QFont mono = numLabel->font();
        mono.setFamily("Courier");
        numLabel->setFont(mono);
        rowLay->addWidget(numLabel);

        auto* bar = new QProgressBar();
        bar->setFixedHeight(4);
        bar->setTextVisible(false);
        bar->setRange(0, 10000);
        bar->setValue(0);
        bar->setStyleSheet(kBarStyle);
        rowLay->addWidget(bar, 1);

        m_instanceLayout->addWidget(row);
        m_workerRows.append({row, bar});
    }

    bool showInstances = (m_batchWorkerCount > 1);
    m_instanceHeaderLabel->setVisible(showInstances);
    m_instanceSection->setVisible(showInstances);
    m_instanceFooterLabel->setVisible(showInstances);

    elapsedTimer.start();
    etaTimer->start(1000);  // Update elapsed/ETA every second

    showProgressView();
}

void BatchSettingsWindow::onFileStarted(const QString& fileName, int fileNumber, int total,
                                        int workerIndex) {
    Q_UNUSED(fileName);

    // Reset this worker's bar to indeterminate (pulsing) until a real percentage arrives
    if (workerIndex >= 0 && workerIndex < m_workerRows.size()) {
        auto* bar = m_workerRows[workerIndex].bar;
        bar->setRange(0, 0);  // Indeterminate / busy indicator
    }

    Q_UNUSED(fileNumber);
    Q_UNUSED(total);
}

void BatchSettingsWindow::onFileProgress(const FFmpegRunner::ProgressInfo& info, int workerIndex) {
    // Update per-worker bar
    if (workerIndex >= 0 && workerIndex < m_workerRows.size()) {
        auto* bar = m_workerRows[workerIndex].bar;
        if (info.totalTime > 0.0 && info.currentTime > 0.0) {
            if (bar->maximum() == 0) bar->setRange(0, 10000);  // Switch from indeterminate
            bar->setValue(qMax(1, static_cast<int>((info.currentTime / info.totalTime) * 10000)));
        }
        // If no valid timing data yet, the bar stays in indeterminate (pulsing) mode
    }

    // speedLabel hidden — per-worker speed isn't meaningful as an aggregate
}

void BatchSettingsWindow::onFileFinished(const QString& fileName, bool success, int workerIndex) {
    Q_UNUSED(fileName);

    // Clear this worker's bar (reset from indeterminate if needed)
    if (workerIndex >= 0 && workerIndex < m_workerRows.size()) {
        auto* bar = m_workerRows[workerIndex].bar;
        bar->setRange(0, 10000);
        bar->setValue(0);
    }

    if (success) {
        completedFiles++;
    } else {
        failedFiles++;
    }

    int done = completedFiles + failedFiles;

    // Visual update throttling
    if (done % updateInterval == 0 || done == totalFiles) {
        QLocale loc;
        progressDetailLabel->setText(
            QString("%1 / %2").arg(loc.toString(done)).arg(loc.toString(totalFiles)));
        progressBar->setValue(done);
    }
}

void BatchSettingsWindow::onBatchFinished(int completed, int failed) {
    etaTimer->stop();

    // Clear all worker bars (reset from indeterminate if needed)
    for (auto& row : m_workerRows) {
        row.bar->setRange(0, 10000);
        row.bar->setValue(0);
    }
    
    completedFiles = completed;
    failedFiles = failed;
    
    // Final bar state
    progressBar->setValue(totalFiles);
    
    // Format elapsed time
    qint64 elapsedMs = elapsedTimer.elapsed();
    int totalSec = static_cast<int>(elapsedMs / 1000);
    int m = totalSec / 60;
    int s = totalSec % 60;
    elapsedLabel->setText(QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0')));
    remainingLabel->setText("Done");
    
    // Summary in the main counter area
    QLocale loc;
    if (failed == 0) {
        progressDetailLabel->setText(
            QString("%1 / %1 ✓").arg(loc.toString(completed)));
    } else {
        progressDetailLabel->setText(
            QString("%1 / %2").arg(loc.toString(completed + failed)).arg(loc.toString(totalFiles)));
        
        // Show failure detail only when there are failures
        succeededLabel->setText(QString("✓ %1 succeeded").arg(loc.toString(completed)));
        succeededLabel->setVisible(true);
        failedLabel->setText(QString("✗ %1 failed").arg(loc.toString(failed)));
        failedLabel->setVisible(true);
    }
    
    pauseButton->setVisible(false);
    resumeButton->setVisible(false);
    cancelButton->setText("Close");
}

void BatchSettingsWindow::updateETA() {
    qint64 elapsedMs = elapsedTimer.elapsed();
    int done = completedFiles + failedFiles;
    
    // Elapsed
    int totalSec = static_cast<int>(elapsedMs / 1000);
    int m = totalSec / 60;
    int s = totalSec % 60;
    elapsedLabel->setText(QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0')));
    
    // Remaining estimate
    if (done > 0 && done < totalFiles) {
        double msPerFile = static_cast<double>(elapsedMs) / done;
        int remaining = totalFiles - done;
        int remainingSec = static_cast<int>((msPerFile * remaining) / 1000);
        int rm = remainingSec / 60;
        int rs = remainingSec % 60;
        
        if (rm >= 60) {
            int rh = rm / 60;
            rm = rm % 60;
            remainingLabel->setText(QString("%1h %2m").arg(rh).arg(rm));
        } else {
            remainingLabel->setText(QString("%1:%2").arg(rm).arg(rs, 2, 10, QChar('0')));
        }
    }
}

// ========== CONTROLS ==========

void BatchSettingsWindow::onPauseClicked() {
    if (batchProcessor) {
        batchProcessor->pause();
    }
    pauseButton->setVisible(false);
    resumeButton->setVisible(true);
    etaTimer->stop();
}

void BatchSettingsWindow::onResumeClicked() {
    if (batchProcessor) {
        batchProcessor->resume();
    }
    resumeButton->setVisible(false);
    pauseButton->setVisible(true);
    etaTimer->start(1000);
}

void BatchSettingsWindow::onCancelClicked() {
    if (batchProcessor &&
        (batchProcessor->getState() == BatchProcessor::State::Processing ||
         batchProcessor->getState() == BatchProcessor::State::Paused)) {
        batchProcessor->cancel();
    }
    
    // If batch is already finished, "Close" returns to settings
    showSettingsView();
}

// ========== CLOSE EVENT ==========

void BatchSettingsWindow::closeEvent(QCloseEvent* event) {
    QSettings().setValue("batchWindow/geometry", saveGeometry());

    // Hide instead of close — batch continues in background
    if (isShowingProgress() && batchProcessor &&
        batchProcessor->getState() == BatchProcessor::State::Processing) {
        event->ignore();
        hide();
    } else {
        event->accept();
    }
}
