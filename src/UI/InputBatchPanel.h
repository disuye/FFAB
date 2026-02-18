#pragma once

#include <QWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>

/**
 * InputBatchPanel - File selection and batch controls
 * 
 * Features:
 * - Scan metadata checkbox (optional FFprobe scanning)
 * - Add folder button (folder picker)
 * - Add files button (file picker)
 * - Clear file list button
 */
class InputBatchPanel : public QWidget {
    Q_OBJECT
    
public:
    explicit InputBatchPanel(QWidget* parent = nullptr);
    ~InputBatchPanel() override;
    
    // Get scan metadata preference
    bool shouldScanMetadata() const;
    
signals:
    void addFolderRequested();  // User clicked "Add Folder" button
    void addFilesRequested();   // User clicked "Add Files" button
    void clearRequested();      // User clicked "Clear File List" button
    
private slots:
    void onAddFolderClicked();
    void onAddFilesClicked();
    void onClearClicked();
    
private:
    void setupUI();
    
    QPushButton* addFolderButton;
    QPushButton* addFilesButton;
    QPushButton* clearButton;
    QCheckBox* scanMetadataCheckbox;
};
