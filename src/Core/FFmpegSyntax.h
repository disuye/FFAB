#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QString>
#include <QVector>
#include <QJsonObject>

/**
 * FFmpegSyntax - Syntax highlighting and formatting for FFmpeg commands
 * 
 * Token scopes (TextMate-compatible naming):
 *   - stream.input        : Input stream labels [0:a], [prev]
 *   - stream.output       : Output stream labels (before ; or end)
 *   - keyword.flag        : Flags like -i, -filter_complex, -map
 *   - entity.filter       : Filter names (volume, afir, etc.)
 *   - string.quoted       : Quoted strings "..."
 *   - string.path         : File paths (often at end of -map lines)
 *   - constant.parameter  : Filter parameters (key=value)
 *   - punctuation.semicolon : Semicolons (filter chain separators)
 *   - comment.continuation : Line continuation backslashes
 * 
 * The syntax definition is stored as JSON and could be exported
 * to TextMate .tmLanguage format for VS Code extensions.
 */

// ============================================================================
// SYNTAX DEFINITION (JSON format, embeddable or loadable from file)
// ============================================================================

namespace FFmpegSyntaxDef {

// This JSON can be externalized to a file if needed
// Scopes follow TextMate naming conventions for future VS Code export
inline const char* syntaxJson = R"JSON({
    "name": "FFmpeg Command",
    "scopeName": "source.ffmpeg",
    "tokens": [
        {
            "scope": "string.quoted",
            "pattern": "\"[^\"]*\"",
            "color": "#97C5F9",
            "comment": "Quoted strings (paths, filter_complex content wrapper)"
        },
        {
            "scope": "stream.input.named",
            "pattern": "\\[[^\\[\\]]+\\]",
            "color": "#ff7979",
            "comment": "All bracketed stream labels default to input"
        },
        {
            "scope": "stream.output",
            "pattern": "\\[[^\\[\\]]+\\](?=(?:\\[[^\\[\\]]+\\])*\\s*[;\"])",
            "color": "#9de291",
            "comment": "Output labels - overrides to orange when followed by ; or quote"
        },
        {
            "scope": "stream.input.numbered",
            "pattern": "\\[\\d+:a\\]",
            "color": "#ff0000",
            "comment": "Numbered input streams [0:a], [1:a]"
        },
        {
            "scope": "keyword.flag.primary",
            "pattern": "(?:^|\\s)-(i|y|filter_complex|map|af|c:a|b:a|ar|ac|f|t|ss|to)(?=\\s|$)",
            "color": "#7D92AF",
            "comment": "Primary FFmpeg flags"
        },
        {
            "scope": "keyword.executable",
            "pattern": "(?:^|/)ffmpeg(?:\\s|$)",
            "color": "#ff00d0",
            "comment": "ffmpeg executable"
        },
        {
            "scope": "entity.filter.name",
            "pattern": "(?<=\\]|;|,|^)\\s*([a-z][a-z0-9_]*)(?==|\\[|;|$)",
            "color": "#dacb7a",
            "comment": "Filter names after stream labels"
        },
        {
            "scope": "punctuation.semicolon",
            "pattern": ";",
            "color": "#eaff00",
            "comment": "Filter chain separator"
        },
        {
            "scope": "comment.continuation",
            "pattern": "\\s\\\\$",
            "color": "#808080",
            "comment": "Line continuation backslash"
        }
    ],
    "lineBreaking": {
        "comment": "Rules for intelligent line breaking",
        "splitBeforeFlags": ["-i", "-filter_complex", "-map", "-y", "-af", "-c:a", "-f"],
        "splitWithinFilterComplex": "semicolon",
        "keepTogether": ["codec flags before path"]
    }
})JSON";

} // namespace FFmpegSyntaxDef


// ============================================================================
// TOKEN RULE STRUCTURE
// ============================================================================

struct SyntaxRule {
    QString scope;
    QRegularExpression pattern;
    QTextCharFormat format;
};


// ============================================================================
// FFMPEG HIGHLIGHTER (QSyntaxHighlighter subclass)
// ============================================================================

class FFmpegHighlighter : public QSyntaxHighlighter {
public:
    explicit FFmpegHighlighter(QTextDocument* parent = nullptr);
    
    // Load syntax from JSON (uses embedded default if not specified)
    void loadSyntax(const QString& json = QString());
    
    // Export current syntax to TextMate JSON format
    QString exportToTextMate() const;
    
protected:
    void highlightBlock(const QString& text) override;
    
private:
    void parseJsonSyntax(const QJsonObject& root);
    QTextCharFormat createFormat(const QString& colorHex);
    
    QVector<SyntaxRule> rules;
};


// ============================================================================
// FFMPEG FORMATTER (Line breaking utility)
// ============================================================================

class FFmpegFormatter {
public:
    /**
     * Format a raw FFmpeg command with intelligent line breaks
     * 
     * Rules:
     * - New line before each major flag (-i, -filter_complex, -map, etc.)
     * - Within -filter_complex, break on semicolons
     * - Keep codec/format flags together, path on its own line
     * - Continuation backslash at end of each line (except last)
     */
    static QString format(const QString& rawCommand);
    
    /**
     * Strip formatting - return to single-line command
     */
    static QString stripFormatting(const QString& formatted);
    
private:
    // Split command respecting quotes
    static QStringList tokenize(const QString& command);
    
    // Format the -filter_complex content specifically
    static QString formatFilterComplex(const QString& content);
};