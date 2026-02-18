#include "AsplitRowWidget.h"
#include "RotatedLabel.h"
#include "FilterChainWidget.h"
#include "Filters/ff-asplit.h"
#include "Filters/MultiOutputFilter.h"
#include "Core/FilterChain.h"
#include "Core/AppConfig.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QApplication>
#include <QDebug>
#include <QColor>

AsplitRowWidget::AsplitRowWidget(AsplitFilter* filter, FilterChain* chain, int position,
                                 FilterChainWidget* widget, QWidget* parent)
    : QWidget(parent), asplitFilter(filter), filterChain(chain), 
      chainWidget(widget), filterPosition(position) {
    setupUI();
    updateButtonStates();
    updateControlButtonStates();  // Set initial M/S/O button colors
    setStyleSheet("border: none;"); // CRITICAL: prevents Qt/macOS implicit borders causing layout glitches
    
    // Listen for parameter changes from the filter
    connect(asplitFilter, &AsplitFilter::numSplitsChanged, 
            this, &AsplitRowWidget::updateButtonStates);
}

void AsplitRowWidget::setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 1, 4, 1);
    layout->setSpacing(2);
    
    // Get filter ID for styling
    int filterId = asplitFilter->getFilterId();
    QString hexId = FilterColors::hexId(filterId);
    
    // Generate colors using centralized algorithm
    QColor rowBgColor = FilterColors::rowBackground(filterId);
    QColor labelBgColor = FilterColors::labelBackground(filterId);
    QColor textColor = FilterColors::textColor(filterId);
    
    // Hex ID badge using RotatedLabel (same as standard row)
    auto* idLabel = new RotatedLabel(hexId);
    idLabel->setBackgroundColor(labelBgColor);
    idLabel->setTextColor(textColor);
    idLabel->setStyleSheet("font-size: 10px;");
    idLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(idLabel, 0);
    
    // Button styles
    QString buttonBaseStyle = "QPushButton { border: none; border-radius: 2px; background-color: rgba(128, 128, 128, 0.17); }";
    QString buttonRowBgStyle = QString("QPushButton { border: none; border-radius: 2px; background: %1; }").arg(rowBgColor.name(QColor::HexArgb));
    
    // Create standard control buttons (EXACT same as delegate)
    auto createCompactButton = [](const QString& text) -> QPushButton* {
        auto* btn = new QPushButton(text);
        btn->setFixedSize(20, 20);
        QSizePolicy btnSp(QSizePolicy::Fixed, QSizePolicy::Fixed);
        btnSp.setRetainSizeWhenHidden(false);
        btn->setSizePolicy(btnSp);
        return btn;
    };
    
    upButton = createCompactButton("↑");
    upButton->setStyleSheet(buttonRowBgStyle);
    upButton->setToolTip("Move filter up");
    connect(upButton, &QPushButton::clicked, this, &AsplitRowWidget::moveUpRequested);
    layout->addWidget(upButton);
    
    downButton = createCompactButton("↓");
    downButton->setStyleSheet(buttonRowBgStyle);
    downButton->setToolTip("Move filter down");
    connect(downButton, &QPushButton::clicked, this, &AsplitRowWidget::moveDownRequested);
    layout->addWidget(downButton);
    
    deleteButton = createCompactButton("✕");
    deleteButton->setStyleSheet(buttonBaseStyle);
    deleteButton->setToolTip("Delete filter (ALT+click for immediate delete)");
    connect(deleteButton, &QPushButton::clicked, this, &AsplitRowWidget::deleteRequested);
    layout->addWidget(deleteButton);
    
    muteButton = createCompactButton("M");
    muteButton->setStyleSheet(buttonBaseStyle);
    muteButton->setToolTip("Mute filter (ALT+click for mute all)");
    connect(muteButton, &QPushButton::clicked, [this]() {
        if (QApplication::keyboardModifiers() & Qt::AltModifier) {
            if (chainWidget) chainWidget->handleMuteAllToggle();
        } else {
            emit muteToggled();
        }
    });
    layout->addWidget(muteButton);
    
    soloButton = createCompactButton("S");
    soloButton->setStyleSheet(buttonBaseStyle);
    soloButton->setToolTip("Solo filter (ALT+click to clear all solos)");
    connect(soloButton, &QPushButton::clicked, [this]() {
        Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();
        bool altPressed = (mods & Qt::AltModifier);
        qDebug() << "Asplit solo button clicked - ALT pressed:" << altPressed << "modifiers:" << mods;
        qDebug() << "chainWidget is" << (chainWidget ? "valid" : "NULL");
        
        if (altPressed) {
            qDebug() << "Calling handleSoloAllClear()";
            if (chainWidget) {
                chainWidget->handleSoloAllClear();
            } else {
                qDebug() << "ERROR: chainWidget is NULL!";
            }
        } else {
            qDebug() << "Emitting soloToggled()";
            emit soloToggled();
        }
    });
    layout->addWidget(soloButton);
    
    customOutputButton = createCompactButton("○");
    customOutputButton->setStyleSheet(buttonBaseStyle);
    customOutputButton->setToolTip("Custom output stream");
    connect(customOutputButton, &QPushButton::clicked, this, &AsplitRowWidget::customOutputToggled);
    layout->addWidget(customOutputButton);
    
    // Filter name (same as standard row)
    auto* nameLabel = new QLabel("asplit");
    nameLabel->setStyleSheet("background: transparent; border: none; padding-left: 6px;");
    nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(nameLabel, 1);  // stretch = 1, takes available space
    
    // Stream selector buttons [1]-[7] AFTER the name
    int numSplits = asplitFilter->numSplits();
    for (int i = 1; i <= 7; i++) {
        auto* btn = createCompactButton(QString::number(i));
        btn->setProperty("streamIndex", i);
        btn->setToolTip(QString("Edit stream %1\nALT+click to mute/unmute").arg(i));
        
        // Show/hide based on split count
        if (i > numSplits) {
            btn->setVisible(false);
        }
        
        // Click to open filter tab, ALT+click to mute/unmute
        connect(btn, &QPushButton::clicked, [this, i]() {
            if (QApplication::keyboardModifiers() & Qt::AltModifier) {
                // ALT+click: toggle mute for this stream (only if it has filters)
                if (asplitFilter->streamHasFilters(i)) {
                    asplitFilter->toggleStreamMute(i);
                    updateButtonStates();
                }
            } else {
                // Normal click: open stream view
                emit streamButtonClicked(i);
            }
        });
        
        streamButtons.append(btn);
        layout->addWidget(btn);
    }
    
    // Connect to mute change signal to update button states
    connect(asplitFilter, &AsplitFilter::streamMuteChanged,
            this, &AsplitRowWidget::updateButtonStates);
    
    // Match standard row height
    setFixedHeight(40);
    
    // Set size policy to ensure proper sizing
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    // Force proper size calculation
    updateGeometry();
    layout->update();
}

void AsplitRowWidget::updateButtonStates() {
    // Query asplit filter directly: which streams have filters?
    int numSplits = asplitFilter->numSplits();
    
    // Get active streams directly from the asplit filter (it owns its sub-chains)
    QSet<int> activeStreams = asplitFilter->getActiveStreams();
    QSet<int> mutedStreams = asplitFilter->getMutedStreams();
    
    for (int i = 0; i < streamButtons.size(); i++) {
        int streamIndex = i + 1;
        QPushButton* btn = streamButtons[i];
        
        // Only show/enable buttons up to numSplits
        bool shouldBeVisible = (streamIndex <= numSplits);
        
        // Update visibility if it changed
        if (btn->isVisible() != shouldBeVisible) {
            btn->setVisible(shouldBeVisible);
            // Force layout update when visibility changes
            updateGeometry();
        }
        
        btn->setEnabled(shouldBeVisible);
        
        if (!shouldBeVisible) continue;
        
        // Check if this stream has any filters
        bool hasFilters = activeStreams.contains(streamIndex);
        bool isMuted = mutedStreams.contains(streamIndex);
        
        if (hasFilters && isMuted) {
            // Muted stream with filters: blue color (matches mute button color)
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: rgba(63, 125, 156, 0.8);
                    color: #FFFFFF;
                    border: none;
                    border-radius: 3px;
                }
            )");
            btn->setToolTip(QString("Stream %1 (MUTED)\nClick to edit, ALT+click to unmute").arg(streamIndex));
            btn->setText(QString("M"));
        } else if (hasFilters) {
            // Active: has filters, not muted
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: rgba(156, 63, 156, 0.8);
                    color: #FFFFFF;
                    border: none;
                    border-radius: 3px;
                }
            )");
            btn->setToolTip(QString("Stream %1 (active)\nClick to edit, ALT+click to mute").arg(streamIndex));
            btn->setText(QString("%1").arg(streamIndex));
        } else {
            // Inactive: no filters yet
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: rgba(128, 128, 128, 0.2);
                    border: none;
                    border-radius: 3px;
                }
            )");
            btn->setToolTip(QString("Stream %1 (empty)\nClick to edit").arg(streamIndex));
            btn->setText(QString("%1").arg(streamIndex));
        }
    }
}

void AsplitRowWidget::updateControlButtonStates() {
    if (!chainWidget) return;
    
    // Query current mute/solo/custom output states from FilterChainWidget
    bool isExplicitMute = chainWidget->isFilterExplicitMute(filterPosition);
    bool isImpliedMute = chainWidget->isFilterImpliedMute(filterPosition);
    bool isSoloed = chainWidget->isFilterSoloed(filterPosition);
    bool hasCustomOutput = asplitFilter->usesCustomOutputStream();
    bool anySoloActive = chainWidget->isAnySoloActive();
    
    QString buttonBaseStyle = "QPushButton { border: none; border-radius: 2px; background-color: rgba(128, 128, 128, 0.17); }";
    QString buttonActiveStyle = "QPushButton { border: none; border-radius: 2px; background-color: rgba(128, 128, 128, 0.17); %1 }";
    
    // MUTE BUTTON
    if (isExplicitMute) {
        muteButton->setStyleSheet(buttonActiveStyle.arg("color: #FFFFFF; background-color: rgba(63, 125, 156, 0.8);"));
        muteButton->setToolTip("MUTED (click to unmute)");
    } else if (isImpliedMute) {
        muteButton->setStyleSheet(buttonActiveStyle.arg("color: rgba(63, 125, 156, 0.9); background-color: rgba(63, 125, 156, 0.25); border: 1px solid rgba(63, 125, 156, 0.6);"));
        muteButton->setToolTip("IMPLIED MUTE (another filter is soloed)\nClick to join solo group");
    } else {
        muteButton->setStyleSheet(buttonBaseStyle);
        muteButton->setToolTip("Click to mute");
    }
    
    // SOLO BUTTON
    if (isSoloed) {
        soloButton->setStyleSheet(buttonActiveStyle.arg("color: #FFFFFF; background-color: rgba(194, 80, 24, 0.8);"));
        soloButton->setToolTip("SOLOED (click to unsolo)");
    } else if (anySoloActive) {
        soloButton->setStyleSheet(buttonBaseStyle + " color: rgba(194, 80, 24, 0.6);");
        soloButton->setToolTip("Click to add to solo group");
    } else {
        soloButton->setStyleSheet(buttonBaseStyle);
        soloButton->setToolTip("Click to solo");
    }
    
    // CUSTOM OUTPUT BUTTON
    int filterId = asplitFilter->getFilterId();
    if (hasCustomOutput) {
        customOutputButton->setStyleSheet(buttonActiveStyle.arg("color: #FFFFFF; background-color: rgba(67, 139, 33, 0.5);"));
        customOutputButton->setToolTip("Custom Output: ON\nThis filter branches to [" + 
                              QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper() + 
                              "]\nClick to disable");
    } else {
        customOutputButton->setStyleSheet(buttonBaseStyle);
        customOutputButton->setToolTip("Custom Output: OFF\nClick to branch signal to [" + 
                              QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper() + "]");
    }
}
