#ifndef FILTERPRESETBAR_H
#define FILTERPRESETBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>
#include <QMenu>

// FilterPresetBar — a thin, unobtrusive preset navigation strip
// that sits at the top of the Filter Parameters panel.
//
// UI only — no save/load logic. All actions emit signals
// that MainWindow (or a future PresetManager) can connect to.
//
// Visibility is toggled globally via View > Filter Presets.
// Right-click context menu on the filter params area provides
// the full preset workflow (Load / Save / Save As / Preset List).

class FilterPresetBar : public QWidget {
    Q_OBJECT

public:
    explicit FilterPresetBar(QWidget* parent = nullptr);

    // Update the displayed preset name (or empty/"No Preset" for none)
    void setPresetName(const QString& name);
    QString presetName() const;

    // Update the filter type context (used for folder lookups later)
    void setFilterType(const QString& type);
    QString filterType() const;

    // Clear state (e.g. when switching to a filter with no loaded preset)
    void clearPreset();

    // Build and show the right-click context menu at the given global position.
    // Caller can connect this from a customContextMenuRequested signal.
    void showContextMenu(const QPoint& globalPos);

    // Populate the preset list submenu with dummy entries (for UI testing).
    // Replace with real folder scanning later.
    void setPresetList(const QStringList& presetNames);

signals:
    // Navigation
    void previousRequested();
    void nextRequested();

    // Context menu actions
    void loadPresetRequested();
    void savePresetRequested();
    void savePresetAsRequested();
    void presetSelected(const QString& presetName);

private:
    void setupUI();
    void rebuildPresetSubmenu();

    QHBoxLayout* m_layout = nullptr;
    QPushButton* m_loadPresetButton = nullptr;
    QPushButton* m_savePresetButton = nullptr;
    QLabel*      m_nameLabel = nullptr;
    QPushButton* m_prevButton = nullptr;
    QPushButton* m_nextButton = nullptr;

    QString m_presetName;
    QString m_filterType;
    QStringList m_presetList;

    // Context menu (lazy-created)
    QMenu* m_contextMenu = nullptr;
    QMenu* m_presetSubmenu = nullptr;
};

#endif // FILTERPRESETBAR_H
