#pragma once

#include <QWidget>
#include <QString>

class QTextEdit;
class QCloseEvent;
class FFmpegHighlighter;

/**
 * CommandViewWindow - Displays the current FFmpeg command
 * 
 * Features:
 * - Syntax highlighting for FFmpeg commands
 * - Toggle between raw/formatted view with \ key
 * - Intelligent line breaking for readability
 * 
 * Color scheme:
 *   - Blue (#3F7D9C):   Input stream labels [0:a], [XXXX]
 *   - Orange (#C25118): Output stream labels (before semicolons)
 *   - Purple (#C586C0): FFmpeg flags (-i, -map, etc.)
 *   - Yellow (#DCDCAA): Filter names
 *   - Tan (#CE9178):    Quoted strings
 */
class CommandViewWindow : public QWidget {
    Q_OBJECT
public:
    explicit CommandViewWindow(QWidget* parent = nullptr);
    ~CommandViewWindow() override = default;
    
    void setCommand(const QString& command);
    QString getCommand() const;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void toggleFormatting();
    void updateDisplay();
    
    QTextEdit* commandText = nullptr;
    FFmpegHighlighter* highlighter = nullptr;
    QString rawCommand;
    bool formattingEnabled = true;  // Start with formatting ON
};
