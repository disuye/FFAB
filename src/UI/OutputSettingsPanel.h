#ifndef OUTPUTSETTINGSPANEL_H
#define OUTPUTSETTINGSPANEL_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>

class OutputSettingsPanel : public QWidget {
    Q_OBJECT
    
public:
    explicit OutputSettingsPanel(QWidget* parent = nullptr);
    
    QString getOutputFormat() const;
    QString getOutputFolder() const;
    int getSampleRate() const;
    int getBitDepth() const;
    int getBitrate() const;
    
    // Setters for preset loading
    void setOutputFormat(const QString& format);
    void setSampleRate(int rate);
    void setBitDepth(int depth);
    void setBitrate(int rate);
    void setOutputFolder(const QString& folder);
    
signals:
    void settingsChanged();
    void outputFolderChanged(const QString& path);
    
private slots:    
    void onBrowseOutputFolder();

private:
    QComboBox* formatCombo;
    QComboBox* sampleRateComboBox;
    QComboBox* bitDepthComboBox;
    QComboBox* bitrateComboBox;
    QLineEdit* outputFolderEdit;

};

#endif // OUTPUTSETTINGSPANEL_H
