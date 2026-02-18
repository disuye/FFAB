#ifndef INPUTPANEL_H
#define INPUTPANEL_H

#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

class FileListWidget;

class InputPanel : public QWidget {
    Q_OBJECT
    
public:
    explicit InputPanel(QWidget* parent = nullptr);
    
    bool shouldScanMetadata() const;
    QString getOutputFolder() const;
    FileListWidget* getFileListWidget() const;
    
    // Setters for preset loading
    void setScanMetadata(bool enabled);
    void setOutputFolder(const QString& folder);
    
signals:
    void addFolderRequested();
    void addFilesRequested();
    void clearRequested();
    void outputFolderChanged(const QString& path);
    
private slots:
    void onBrowseOutputFolder();
    
private:
    QCheckBox* scanMetadataCheckbox;
    QLineEdit* outputFolderEdit;
    FileListWidget* fileListWidget;
};

#endif // INPUTPANEL_H
