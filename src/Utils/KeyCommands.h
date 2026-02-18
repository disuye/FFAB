#pragma once

#include <QKeySequence>
#include <QShortcut>
#include <QWidget>
#include <functional>

/**
 * FFAB Keyboard Shortcuts
 * 
 * Centralized registry for all key commands.
 * Add new shortcuts here, wire them up in MainWindow::setupKeyCommands()
 * 
 * NOTE: Use functions (not global variables) because QKeySequence::StandardKey
 * requires Qt to be initialized first.
 */

namespace KeyCommands {

// ============================================================================
// SHORTCUT DEFINITIONS
// ============================================================================

// Filter Chain
inline QKeySequence AddFilter()          { return QKeySequence(Qt::Key_QuoteLeft); }  // `/~ key
inline QKeySequence AddFilterAlt()       { return QKeySequence(Qt::Key_A); }
// inline QKeySequence DeleteFilter()       { return QKeySequence(Qt::ALT | Qt::Key_X); }
// inline QKeySequence MoveFilterUp()       { return QKeySequence(Qt::ALT | Qt::Key_Up); }
// inline QKeySequence MoveFilterDown()     { return QKeySequence(Qt::ALT | Qt::Key_Down); }

// Preview / Playback
inline QKeySequence GeneratePreview()    { return QKeySequence(Qt::Key_Left); }
inline QKeySequence PlayPreviewAlt()     { return QKeySequence(Qt::Key_Right); } // Up/down scrolling in File List, L to generate, R to play
inline QKeySequence PlayPreview()        { return QKeySequence(Qt::Key_Space); }
inline QKeySequence StopPreview()        { return QKeySequence(Qt::Key_Escape); }

// File Operations
inline QKeySequence NewDocument()        { return QKeySequence::New; }           // Cmd+N
inline QKeySequence OpenPreset()         { return QKeySequence::Open; }          // Cmd+O
inline QKeySequence SavePreset()         { return QKeySequence::Save; }          // Cmd+S
inline QKeySequence SavePresetAs()       { return QKeySequence::SaveAs; }        // Cmd+Shift+S

// Mute/Solo (handled in FilterChainWidget via button clicks + modifiers)
// Alt+M = Mute All Toggle
// Alt+S = Clear All Solos

// View / Windows  
// inline QKeySequence ShowCommandViewer()  { return QKeySequence(Qt::CTRL | Qt::Key_K); }
// inline QKeySequence ToggleInputPanel()   { return QKeySequence(Qt::Key_1); }
// inline QKeySequence ToggleFilterPanel()  { return QKeySequence(Qt::Key_2); }
// inline QKeySequence ToggleOutputPanel()  { return QKeySequence(Qt::Key_3); }

// Command View Window
inline QKeySequence CmdViewToggleHighlight()  { return QKeySequence(Qt::Key_Backslash); }

// ============================================================================
// HELPER: Register a shortcut with a widget
// ============================================================================

inline QShortcut* bind(QWidget* parent, const QKeySequence& key, 
                       std::function<void()> callback,
                       Qt::ShortcutContext context = Qt::ApplicationShortcut) {
    auto* shortcut = new QShortcut(key, parent);
    shortcut->setContext(context);
    QObject::connect(shortcut, &QShortcut::activated, parent, callback);
    return shortcut;
}

// Bind multiple keys to same action
inline void bindMultiple(QWidget* parent, 
                         std::initializer_list<QKeySequence> keys,
                         std::function<void()> callback,
                         Qt::ShortcutContext context = Qt::ApplicationShortcut) {
    for (const auto& key : keys) {
        bind(parent, key, callback, context);
    }
}

} // namespace KeyCommands
