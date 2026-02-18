#pragma once

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>

class FilterChain;
class BaseFilter;
class MultiOutputFilter;

/**
 * FilterChainWidget - SMPLR-style implementation
 * Uses QListView with custom delegate for native buttons
 * 
 * View Modes:
 * - MainChainView: Shows the main filter chain (INPUT → filters → OUTPUT)
 * - StreamView: Shows a specific stream's sub-chain from a MultiOutputFilter
 * 
 * With the new architecture, MultiOutputFilters (asplit, etc.) own their sub-chains
 * internally. This widget just asks them for their chain data.
 */
class FilterChainWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit FilterChainWidget(FilterChain* chain, QWidget* parent = nullptr);
    ~FilterChainWidget() override;
    
    void refreshChain();
    
    // Public methods for delegate to call
    void moveFilterUp(int position);
    void moveFilterDown(int position);
    void deleteFilter(int position);
    void confirmDeleteFilter(int position);
    
    // Drag-and-drop support
    void handleDragDrop(int sourceModelRow, int targetModelRow);
    FilterChain* getFilterChain() const { return filterChain; }
    void handleMuteToggle(int position);
    void handleSoloToggle(int position);
    void handleMuteAllToggle();
    void handleSoloAllClear();
    void handleCustomOutputToggle(int position);
    
    // Filter ID-based handlers (for sub-chain filters)
    void handleMuteToggleById(int filterId);
    void handleSoloToggleById(int filterId);
    void handleCustomOutputToggleById(int filterId);
    
    // Sub-chain signal flow label
    QString getSubChainSignalFlowLabel(int multiOutputPos, int streamIndex, int indexInSubChain) const;
    
    // State queries for UI and FFmpeg command generation
    bool isFilterMuted(int position) const;
    bool isFilterSoloed(int position) const;
    bool isFilterImpliedMute(int position) const;
    bool isFilterExplicitMute(int position) const;
    bool isAnySoloActive() const;
    bool isAnySoloActiveInSubChain() const;  // Check only current sub-chain context
    
    // Mute/solo by filter ID (for sub-chain filters)
    bool isFilterMutedById(int filterId) const;
    bool isFilterSoloedById(int filterId) const;
    bool isFilterImpliedMuteById(int filterId) const;
    bool isFilterExplicitMuteById(int filterId) const;
    
    QList<int> getMutedFilterPositions() const;
    QString getFilterInputStreamLabel(int position) const;
    
    // Get/Set mute/solo states for preset save/load
    QList<int> getMutedFilterIds() const;
    QList<int> getSoloFilterIds() const;
    void setMutedFilterIds(const QList<int>& filterIds);
    void setSoloFilterIds(const QList<int>& filterIds);
    
    void clearMuteStates() { explicitMuteStates.clear(); impliedMuteSet.clear(); }
    void clearSoloStates() { soloStates.clear(); impliedMuteSet.clear(); }
    void clearAllStates() { explicitMuteStates.clear(); soloStates.clear(); impliedMuteSet.clear(); }
    
    // KeyCommands API
    void showAddFilterMenu() { onAddFilterClicked(); }
    int getCurrentSelection() const { return currentSelection; }
    
    // Stream view switching
    void showStreamView(int multiOutputPos, int streamIndex);
    void showMainChain();
    bool isInStreamView() const { return currentViewMode == StreamView; }
    int getCurrentStreamIndex() const { return currentStreamIndex; }
    int getCurrentMultiOutputPosition() const { return currentMultiOutputPos; }
    
signals:
    void filterSelected(int position);
    void subChainFilterSelected(int filterId);  // For selecting filters in sub-chains
    void chainModified();
    
private slots:
    void onAddFilterClicked();
    void onListItemClicked(const QModelIndex& index);
    
private:
    void setupUI();
    void addFilterRow(int position);
    void addSubChainFilterRow(std::shared_ptr<BaseFilter> filter, int indexInSubChain);
    void addArrowSeparator();
    void addSubChainHeader();   // INPUT-style row for sub-chain
    void addSubChainFooter();   // OUTPUT-style row for sub-chain
    
    void recomputeImpliedMutes();
    void recomputeSubChainImpliedMutes();  // For sub-chain context
    QSet<int> computeSoloProtectedFilters() const;
    QSet<int> getSubChainFilterIds() const;  // Get filter IDs in current sub-chain
    void updateAsplitButtonStates();
    
    // View modes
    enum ViewMode { MainChainView, StreamView };
    ViewMode currentViewMode = MainChainView;
    int currentStreamIndex = 0;
    int currentMultiOutputPos = -1;
    
    FilterChain* filterChain;
    QListView* listView;
    QStandardItemModel* model;
    QPushButton* addFilterButton;
    
    // State tracking (keyed by filter ID)
    QMap<int, bool> explicitMuteStates;
    QMap<int, bool> soloStates;
    QSet<int> impliedMuteSet;
    
    int currentSelection = -1;
    bool addFilterMenuOpen = false;
};
