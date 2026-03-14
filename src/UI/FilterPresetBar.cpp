#include "UnicodeSymbols.h"
#include "FilterPresetBar.h"

#include <QAction>
#include <QFont>

FilterPresetBar::FilterPresetBar(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    clearPreset();
}

void FilterPresetBar::setupUI() {
    setContentsMargins(0, 0, 0, 0);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(
        "FilterPresetBar { background: qlineargradient(y1:0, y2:1, stop:0 rgba(128,128,128,50), stop:0.5 rgba(128,128,128,25)); border-radius: 4px; padding: 4px; }"
    );
    
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(4, 0, 4, 0);
    m_layout->setSpacing(2);
    
    // Load
    m_loadPresetButton = new QPushButton(this);
    m_loadPresetButton->setText("Load");
    m_loadPresetButton->setToolTip("Load filter preset");
    connect(m_loadPresetButton, &QPushButton::clicked, this, &FilterPresetBar::loadPresetRequested);

    // Save
    m_savePresetButton = new QPushButton(this);
    m_savePresetButton->setText("Save As...");
    m_savePresetButton->setToolTip("Save filter preset");
    connect(m_savePresetButton, &QPushButton::clicked, this, &FilterPresetBar::savePresetAsRequested);

    // Preset name label (centered, stretches)
    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->setStyleSheet("QLabel { font-size: 11px; }");
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // ◀ Previous
    m_prevButton = new QPushButton(this);
    m_prevButton->setIcon(Sym::triangleLeftIcon());
    m_prevButton->setIconSize(QSize(10, 10));
    m_prevButton->setMaximumWidth(24);
    m_prevButton->setToolTip("Previous preset");
    connect(m_prevButton, &QPushButton::clicked, this, &FilterPresetBar::previousRequested);

    // ▶ Next
    m_nextButton = new QPushButton(this);
    m_nextButton->setIcon(Sym::triangleRightIcon());
    m_nextButton->setIconSize(QSize(10, 10));
    m_nextButton->setMaximumWidth(24);
    m_nextButton->setToolTip("Next preset");
    connect(m_nextButton, &QPushButton::clicked, this, &FilterPresetBar::nextRequested);

    m_layout->setAlignment(Qt::AlignVCenter);
    m_layout->addWidget(m_loadPresetButton);
    m_layout->addWidget(m_savePresetButton);
    m_layout->addWidget(m_nameLabel, 4);
    m_layout->addWidget(m_prevButton);
    m_layout->addWidget(m_nextButton);

    // Context menu (right-click anywhere on the bar)
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        showContextMenu(mapToGlobal(pos));
    });
}

void FilterPresetBar::setPresetName(const QString& name) {
    m_presetName = name;
    if (name.isEmpty()) {
        m_nameLabel->setText("No Preset");
        m_nameLabel->setStyleSheet(
            "QLabel { font-size: 11px; font-style: italic; background: transparent; }"
        );
    } else {
        m_nameLabel->setText(name);
        m_nameLabel->setStyleSheet(
            "QLabel { font-size: 11px; background: transparent; }"
        );
    }
}

QString FilterPresetBar::presetName() const {
    return m_presetName;
}

void FilterPresetBar::setFilterType(const QString& type) {
    m_filterType = type;
}

QString FilterPresetBar::filterType() const {
    return m_filterType;
}

void FilterPresetBar::clearPreset() {
    m_presetName.clear();
    m_filterType.clear();
    m_presetList.clear();
    setPresetName(QString());  // triggers "No Preset" display

    // Disable nav when no preset loaded
    m_prevButton->setEnabled(false);
    m_nextButton->setEnabled(false);
}

void FilterPresetBar::setPresetList(const QStringList& presetNames) {
    m_presetList = presetNames;

    // Enable nav buttons if there's something to navigate
    bool hasMultiple = m_presetList.size() > 1;
    m_prevButton->setEnabled(hasMultiple);
    m_nextButton->setEnabled(hasMultiple);
}

void FilterPresetBar::showContextMenu(const QPoint& globalPos) {
    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);

        m_contextMenu->addAction("Load Preset...", this, [this]() {
            emit loadPresetRequested();
        });

        m_contextMenu->addSeparator();

        m_contextMenu->addAction("Save Preset", this, [this]() {
            emit savePresetRequested();
        });

        m_contextMenu->addAction("Save Preset As...", this, [this]() {
            emit savePresetAsRequested();
        });

        m_contextMenu->addSeparator();

        // Preset list submenu
        m_presetSubmenu = m_contextMenu->addMenu("Presets");

        m_contextMenu->addSeparator();

        // Toggle FilterPresetBar UI
        m_contextMenu->addAction("Filter Preset UI...", this, [this]() {
            emit toggleBarRequested();
        });

    }

    rebuildPresetSubmenu();
    m_contextMenu->popup(globalPos);
}

void FilterPresetBar::rebuildPresetSubmenu() {
    if (!m_presetSubmenu) return;

    m_presetSubmenu->clear();

    if (m_presetList.isEmpty()) {
        auto* emptyAction = m_presetSubmenu->addAction("(No presets saved)");
        emptyAction->setEnabled(false);
        return;
    }

    for (const QString& name : m_presetList) {
        auto* action = m_presetSubmenu->addAction(name);

        // Checkmark the currently loaded preset
        if (name == m_presetName) {
            action->setCheckable(true);
            action->setChecked(true);
        }

        connect(action, &QAction::triggered, this, [this, name]() {
            emit presetSelected(name);
        });
    }
}
