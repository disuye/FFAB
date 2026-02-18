#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>

/**
 * OutputPanel - Output folder and codec settings
 * 
 * Features:
 * - Output folder browse button
 * - Audio codec selection (future)
 * - File naming patterns (future)
 */
class OutputPanel : public QWidget {
    Q_OBJECT
    
public:
    explicit OutputPanel(QWidget* parent = nullptr);
    ~OutputPanel() override;
    
    // Get output folder
    QString getOutputFolder() const;
    
    // Set output folder programmatically
    void setOutputFolder(const QString& path);
    
signals:
    void outputFolderChanged(const QString& path);
    
private slots:
    void onBrowseOutputClicked();
    
private:
    void setupUI();
    
    QLineEdit* outputFolderEdit;
    QPushButton* browseOutputButton;
    // QComboBox* codecComboBox;  // Future
    // QLineEdit* namingPatternEdit;  // Future
};
