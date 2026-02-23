#include "FileListWidget.h"
#include <QHeaderView>
#include <QCheckBox>
#include <QApplication>
#include <QPalette>
#include <QKeyEvent>
#include <algorithm>

FileListWidget::FileListWidget(QWidget* parent) : QWidget(parent) {
    setupUI();
    updateBackground();
}

FileListWidget::~FileListWidget() = default;

void FileListWidget::setupUI() {
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Create table widget
    tableWidget = new QTableWidget();
    tableWidget->setObjectName("fileListTable");  // For stylesheet targeting
    
    // Set up columns
    QStringList headers = {"✓", "Filename", "*", "T", "SR", "BD", "CH", "BR"};
    tableWidget->setColumnCount(headers.size());
    tableWidget->setHorizontalHeaderLabels(headers);
    
    // Table appearance
    tableWidget->setAlternatingRowColors(true);  // Enable native alternating colors
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  // Multi-select with Cmd/Shift
    tableWidget->setShowGrid(false);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setWordWrap(false);
    tableWidget->setSortingEnabled(true);  // Enable column sorting!
    
    // Column sizing
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);  // Checkbox
    tableWidget->setColumnWidth(0, 30);
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  // Filename stretches
    tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    
    // Row height - match SMPLR (20px)
    tableWidget->verticalHeader()->setDefaultSectionSize(20);
    
    // Install event filter to catch Delete key
    tableWidget->installEventFilter(this);

    // Double-click on any non-checkbox column → generate preview + auto-play
    connect(tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int /*row*/, int col) {
        if (col == 0) return;  // Ignore checkbox column
        emit previewRequested();
    });
    
    layout->addWidget(tableWidget);
}

void FileListWidget::updateBackground() {
    // Detect dark mode
    bool isDark = qApp->palette().color(QPalette::Window).lightness() < 128;
    
    QString brightRow = isDark ? "#2C2C2C" : "#FFFFFF";
    QString shadedRow = isDark ? "#212121" : "#F4F6F5";
    
    // Use Qt's native alternating row colors
    QString tableStyle = QString(
        "QTableWidget#fileListTable {"
        "   background-color: %1;"              // Bright row
        "   alternate-background-color: %2;"    // Shaded row
        "   font-size: 11px;"
        "   font-weight: light;"
        "}"
        "QTableWidget#fileListTable::item {"
        "   padding: 2px 0x;"
        "}"
    ).arg(brightRow, shadedRow);
    
    tableWidget->setStyleSheet(tableStyle);
}

void FileListWidget::addFile(const AudioFileInfo& fileInfo) {
    files.append(fileInfo);
    
    // Add single row instead of rebuilding entire table (O(1) vs O(n))
    tableWidget->setRowCount(files.size());
    populateSingleRow(files.size() - 1);
}

void FileListWidget::addFiles(const QList<AudioFileInfo>& newFiles) {
    if (newFiles.isEmpty()) return;
    
    // Disable sorting and updates during batch add
    tableWidget->setSortingEnabled(false);  // ADD THIS
    tableWidget->setUpdatesEnabled(false);
    
    int startIndex = files.size();
    files.append(newFiles);
    
    // Pre-allocate rows
    tableWidget->setRowCount(files.size());
    
    // Add only the new rows
    for (int i = startIndex; i < files.size(); ++i) {
        populateSingleRow(i);
    }
    
    tableWidget->setUpdatesEnabled(true);
    tableWidget->setSortingEnabled(true);  // ADD THIS - re-enable after population
}

void FileListWidget::clearFiles() {
    files.clear();
    tableWidget->setRowCount(0);
}

void FileListWidget::deleteSelectedFiles() {
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        return;  // Nothing selected
    }
    
    // Collect original indices to delete
    QList<int> indicesToDelete;
    for (const QModelIndex& index : selectedRows) {
        int visualRow = index.row();
        QTableWidgetItem* filenameItem = tableWidget->item(visualRow, 1);
        if (filenameItem) {
            int originalIndex = filenameItem->data(Qt::UserRole).toInt();
            if (originalIndex >= 0 && originalIndex < files.size()) {
                indicesToDelete.append(originalIndex);
            }
        }
    }
    
    // Sort in descending order to delete from end to start (avoids index shifting issues)
    std::sort(indicesToDelete.begin(), indicesToDelete.end(), std::greater<int>());
    
    // Remove files from the list
    for (int index : indicesToDelete) {
        files.removeAt(index);
    }
    
    // Repopulate table to reflect changes (full rebuild needed after delete)
    populateTable();
    
    // Emit signal that file selection has changed
    emit fileSelectionChanged();
}

bool FileListWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == tableWidget && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        
        // Delete or Backspace key
        if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
            deleteSelectedFiles();
            return true;  // Event handled
        }
    }
    
    // Pass event to base class
    return QWidget::eventFilter(obj, event);
}

void FileListWidget::rescanMetadata() {
    // Emit signal so MainWindow can trigger AudioFileScanner
    emit rescanRequested();
}

void FileListWidget::populateSingleRow(int i) {
    const AudioFileInfo& file = files[i];
    
    // Checkbox for enabling/disabling file
    auto checkbox = new QCheckBox();
    checkbox->setChecked(file.enabled);
    checkbox->setProperty("originalIndex", i);  // Store original file index
    connect(checkbox, &QCheckBox::clicked, [this, checkbox]() {
        // Get the original file index from the checkbox property
        int originalIndex = checkbox->property("originalIndex").toInt();
        
        // Get the state of the CLICKED checkbox
        bool newState = checkbox->isChecked();
        
        // Find the visual row of this checkbox
        int visualRow = -1;
        for (int r = 0; r < tableWidget->rowCount(); r++) {
            QWidget* widget = tableWidget->cellWidget(r, 0);
            if (widget && widget->findChild<QCheckBox*>() == checkbox) {
                visualRow = r;
                break;
            }
        }
        
        // Get all selected visual rows
        QSet<int> selectedVisualRows;
        auto selectedItems = tableWidget->selectedItems();
        for (auto* item : selectedItems) {
            selectedVisualRows.insert(item->row());
        }
        
        // If current visual row is in selection, apply to all selected rows
        if (!selectedVisualRows.isEmpty() && selectedVisualRows.contains(visualRow)) {
            for (int visRow : selectedVisualRows) {
                // Get original index from filename column's UserRole data
                QTableWidgetItem* filenameItem = tableWidget->item(visRow, 1);
                if (filenameItem) {
                    int origIdx = filenameItem->data(Qt::UserRole).toInt();
                    if (origIdx >= 0 && origIdx < files.size()) {
                        files[origIdx].enabled = newState;

                        // Update sort item
                        QTableWidgetItem* sortItem = tableWidget->item(visRow, 0);
                        if (sortItem) sortItem->setText(newState ? "1" : "0");

                        // Update checkbox visually
                        QWidget* widget = tableWidget->cellWidget(visRow, 0);
                        if (widget) {
                            QCheckBox* otherCheckbox = widget->findChild<QCheckBox*>();
                            if (otherCheckbox) {
                                otherCheckbox->blockSignals(true);
                                otherCheckbox->setChecked(newState);
                                otherCheckbox->blockSignals(false);
                            }
                        }
                    }
                }
            }
        } else {
            // Not in selection, just toggle this one
            if (originalIndex < files.size()) {
                files[originalIndex].enabled = newState;

                // Update sort item
                if (visualRow >= 0) {
                    QTableWidgetItem* sortItem = tableWidget->item(visualRow, 0);
                    if (sortItem) sortItem->setText(newState ? "1" : "0");
                }
            }
        }
        
        emit fileSelectionChanged();
    });
    
    QWidget* checkboxWidget = new QWidget();
    auto checkboxLayout = new QHBoxLayout(checkboxWidget);
    checkboxLayout->addWidget(checkbox);
    checkboxLayout->setAlignment(Qt::AlignCenter);
    checkboxLayout->setContentsMargins(0, 0, 0, 0);
    tableWidget->setCellWidget(i, 0, checkboxWidget);

    // Hidden item drives sort for column 0; text transparent so it doesn't show behind the widget
    auto checkItem = new QTableWidgetItem(file.enabled ? "1" : "0");
    checkItem->setFlags(checkItem->flags() & ~Qt::ItemIsEditable);
    checkItem->setForeground(QBrush(Qt::transparent));
    tableWidget->setItem(i, 0, checkItem);

    // Filename
    auto filenameItem = new QTableWidgetItem(file.fileName);
    filenameItem->setData(Qt::UserRole, i);  // Store original index for sorting
    filenameItem->setFlags(filenameItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 1, filenameItem);
    
    // Format
    auto formatItem = new QTableWidgetItem(file.format);
    formatItem->setFlags(formatItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 2, formatItem);
    
    // Duration
    auto durationItem = new QTableWidgetItem(file.duration);
    durationItem->setFlags(durationItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 3, durationItem);
    
    // Sample Rate (store integer for proper sorting)
    auto sampleRateItem = new QTableWidgetItem(QString("%1 Hz").arg(file.sampleRate));
    sampleRateItem->setData(Qt::UserRole, file.sampleRate);  // Store actual number
    sampleRateItem->setFlags(sampleRateItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 4, sampleRateItem);

    // Bits Per Sample (store integer for proper sorting)
    auto bitsPerSampleItem = new QTableWidgetItem(file.bitsPerSample > 0 ? QString("%1 bit").arg(file.bitsPerSample) : "Lossy");
    bitsPerSampleItem->setData(Qt::UserRole, file.bitsPerSample);  // Store actual number
    bitsPerSampleItem->setFlags(bitsPerSampleItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 5, bitsPerSampleItem);
    
    // Channels (store integer for proper sorting)
    QString channelStr = file.channels == 1 ? "Mono" : file.channels == 2 ? "Stereo" : file.channels >= 3 ? "Multi" : QString::number(file.channels);
    auto channelsItem = new QTableWidgetItem(channelStr);
    channelsItem->setData(Qt::UserRole, file.channels);  // Store actual number
    channelsItem->setFlags(channelsItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 6, channelsItem);
    
    // Bitrate (store integer for proper sorting)
    auto bitrateItem = new QTableWidgetItem(file.bitrate > 0 ? QString("%1 kbps").arg(file.bitrate) : "N/A");
    bitrateItem->setData(Qt::UserRole, file.bitrate);  // Store actual number
    bitrateItem->setFlags(bitrateItem->flags() & ~Qt::ItemIsEditable);
    tableWidget->setItem(i, 7, bitrateItem);
}

void FileListWidget::populateTable() {
    tableWidget->setSortingEnabled(false);  // ADD THIS
    tableWidget->setUpdatesEnabled(false);
    tableWidget->setRowCount(files.size());
    
    for (int i = 0; i < files.size(); ++i) {
        populateSingleRow(i);
    }
    
    tableWidget->setUpdatesEnabled(true);
    tableWidget->setSortingEnabled(true);  // ADD THIS
}

QList<FileListWidget::AudioFileInfo> FileListWidget::getEnabledFiles() const {
    QList<AudioFileInfo> enabledFiles;
    for (const AudioFileInfo& file : files) {
        if (file.enabled) {
            enabledFiles.append(file);
        }
    }
    return enabledFiles;
}

QList<FileListWidget::AudioFileInfo> FileListWidget::getSelectedFiles() const {
    QList<AudioFileInfo> selectedFiles;
    QModelIndexList selectedRows = tableWidget->selectionModel()->selectedRows();
    
    for (const QModelIndex& index : selectedRows) {
        int visualRow = index.row();
        
        // Get the original file index from the filename column (column 1)
        QTableWidgetItem* filenameItem = tableWidget->item(visualRow, 1);
        if (filenameItem) {
            int originalIndex = filenameItem->data(Qt::UserRole).toInt();

            qDebug() << "Visual row:" << visualRow 
                     << "-> Original index:" << originalIndex
                     << "-> File:" << (originalIndex < files.size() ? files[originalIndex].fileName : "OUT OF BOUNDS");

            if (originalIndex >= 0 && originalIndex < files.size()) {
                selectedFiles.append(files[originalIndex]);
            }
        }
    }
    
    return selectedFiles;
}

QList<FileListWidget::AudioFileInfo> FileListWidget::getAllFiles() const {
    return files;
}