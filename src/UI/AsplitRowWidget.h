#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include <QLabel>

class AsplitFilter;
class FilterChain;

/**
 * AsplitRowWidget - Custom UI for asplit filter row
 * 
 * Displays inline stream selector buttons: [o][1][2][3][4][5][6][7]
 * - [o] = Stream 0 (thru/main chain)
 * - [1]-[7] = Split streams 1-7
 * 
 * Button states:
 * - Active (colored): Stream has filters
 * - Inactive (grey): Stream is empty
 * - Clicking button opens stream view for editing
 */
class AsplitRowWidget : public QWidget {
    Q_OBJECT
    
public:
    AsplitRowWidget(AsplitFilter* filter, FilterChain* chain, int position, 
                    class FilterChainWidget* chainWidget, QWidget* parent = nullptr);
    
    void updateButtonStates();  // Refresh which streams are active
    void updateControlButtonStates();  // Refresh M/S/O button colors
    
    QSize sizeHint() const override {
        return QSize(372, 40);  // Match standard filter row size
    }
    
    QSize minimumSizeHint() const override {
        return QSize(372, 40);
    }
    
signals:
    void streamButtonClicked(int streamIndex);  // 0-7
    void moveUpRequested();
    void moveDownRequested();
    void deleteRequested();
    void muteToggled();
    void soloToggled();
    void customOutputToggled();
    
private:
    void setupUI();
    
    AsplitFilter* asplitFilter;
    FilterChain* filterChain;
    class FilterChainWidget* chainWidget;
    int filterPosition;
    
    // Control buttons
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* deleteButton;
    QPushButton* muteButton;
    QPushButton* soloButton;
    QPushButton* customOutputButton;
    
    // Stream selector buttons [1]-[7]
    QVector<QPushButton*> streamButtons;
};
