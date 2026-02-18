#include "CommandViewWindow.h"
#include "Core/Preferences.h"
#include "Core/FFmpegSyntax.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QFont>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QLabel>

CommandViewWindow::CommandViewWindow(QWidget* parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("View Command");
    setMinimumSize(500, 120);
    resize(700, 180);
    
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    
    commandText = new QTextEdit();
    commandText->setReadOnly(true);
    commandText->setPlaceholderText("Click [Generate Preview] or [View Command] to see the FFmpeg command");
    
    QFont monoFont("Fira Code");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(12);
    commandText->setFont(monoFont);
    
    commandText->setStyleSheet(R"(
        QTextEdit {
            background-color: #2A2A2A;
            color: #FFFFFF;
            border: 1px solid #2A2A2A;
            border-radius: 4px;
            padding: 12px;
        }
    )");
    
    commandText->setLineWrapMode(QTextEdit::WidgetWidth);
    commandText->installEventFilter(this);
    
    // Create syntax highlighter (attaches to document)
    highlighter = new FFmpegHighlighter(commandText->document());
    
    auto infoLabel = new QLabel("Press [\\] to toggle formatting");
    infoLabel->setStyleSheet("padding: 0; margin: 0; font-size: 9px; color: #808080");
    layout->addWidget(infoLabel);
 
    layout->addWidget(commandText);
   
    // Restore geometry from preferences
    QByteArray geometry = Preferences::instance().commandWindowGeometry();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

bool CommandViewWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == commandText && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Backslash) {
            toggleFormatting();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void CommandViewWindow::setCommand(const QString& command) {
    rawCommand = command;
    updateDisplay();
}

QString CommandViewWindow::getCommand() const {
    return rawCommand;
}

void CommandViewWindow::toggleFormatting() {
    formattingEnabled = !formattingEnabled;
    updateDisplay();
}

void CommandViewWindow::updateDisplay() {
    if (rawCommand.isEmpty()) {
        commandText->clear();
        return;
    }
    
    if (formattingEnabled) {
        // Enable syntax highlighting
        highlighter->setDocument(commandText->document());
        // Apply line break formatting
        QString formatted = FFmpegFormatter::format(rawCommand);
        commandText->setPlainText(formatted);
    } else {
        // Disable syntax highlighting - plain white text
        highlighter->setDocument(nullptr);
        // Raw single-line command
        commandText->setPlainText(rawCommand);
    }
}

void CommandViewWindow::closeEvent(QCloseEvent* event) {
    Preferences::instance().setCommandWindowGeometry(saveGeometry());
    Preferences::instance().setCommandWindowVisible(false);
    QWidget::closeEvent(event);
}