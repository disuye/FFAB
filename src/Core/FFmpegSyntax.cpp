#include "FFmpegSyntax.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

// ============================================================================
// FFMPEG HIGHLIGHTER IMPLEMENTATION
// ============================================================================

FFmpegHighlighter::FFmpegHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    loadSyntax();  // Load default embedded syntax
}

void FFmpegHighlighter::loadSyntax(const QString& json) {
    rules.clear();
    
    QString syntaxSource = json.isEmpty() 
        ? QString::fromUtf8(FFmpegSyntaxDef::syntaxJson) 
        : json;
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(syntaxSource.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "FFmpegHighlighter: JSON parse error:" << error.errorString();
        return;
    }
    
    parseJsonSyntax(doc.object());
}

void FFmpegHighlighter::parseJsonSyntax(const QJsonObject& root) {
    QJsonArray tokens = root["tokens"].toArray();
    
    for (const QJsonValue& tokenVal : tokens) {
        QJsonObject token = tokenVal.toObject();
        
        QString scope = token["scope"].toString();
        QString pattern = token["pattern"].toString();
        QString color = token["color"].toString();
        
        if (pattern.isEmpty()) continue;
        
        SyntaxRule rule;
        rule.scope = scope;
        rule.pattern = QRegularExpression(pattern);
        rule.format = createFormat(color);
        
        if (rule.pattern.isValid()) {
            rules.append(rule);
        } else {
            qWarning() << "FFmpegHighlighter: Invalid pattern for scope" << scope 
                       << ":" << rule.pattern.errorString();
        }
    }
    
    qDebug() << "FFmpegHighlighter: Loaded" << rules.size() << "syntax rules";
}

QTextCharFormat FFmpegHighlighter::createFormat(const QString& colorHex) {
    QTextCharFormat format;
    format.setForeground(QColor(colorHex));
    return format;
}

void FFmpegHighlighter::highlightBlock(const QString& text) {
    // Apply each rule in order (later rules can override earlier)
    for (const SyntaxRule& rule : rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int start = match.capturedStart();
            int length = match.capturedLength();
            setFormat(start, length, rule.format);
        }
    }
}

QString FFmpegHighlighter::exportToTextMate() const {
    // Build TextMate-compatible JSON
    QJsonObject root;
    root["name"] = "FFmpeg Command";
    root["scopeName"] = "source.ffmpeg";
    root["fileTypes"] = QJsonArray({"ffmpeg"});
    
    QJsonArray patterns;
    for (const SyntaxRule& rule : rules) {
        QJsonObject pattern;
        pattern["name"] = rule.scope;
        pattern["match"] = rule.pattern.pattern();
        patterns.append(pattern);
    }
    root["patterns"] = patterns;
    
    // Note: TextMate themes handle colors separately via scope names
    // This export provides the grammar; colors come from theme
    
    QJsonDocument doc(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}


// ============================================================================
// FFMPEG FORMATTER IMPLEMENTATION
// ============================================================================

QString FFmpegFormatter::format(const QString& rawCommand) {
    if (rawCommand.isEmpty()) return rawCommand;
    
    QStringList outputLines;
    
    // Simple approach: scan for flags, only break when NOT inside quotes
    int i = 0;
    int len = rawCommand.length();
    QString currentLine;
    bool inQuotes = false;
    
    while (i < len) {
        QChar c = rawCommand[i];
        
        // Track quote state
        if (c == '"') {
            inQuotes = !inQuotes;
            currentLine += c;
            i++;
            continue;
        }
        
        // Only check for flags when not in quotes
        if (!inQuotes && c == '-') {
            QString remaining = rawCommand.mid(i);
            
            // Check for flags that should start a new line
            QStringList newLineFlags = {
                "-i ", "-y ", "-filter_complex ", "-map "
            };
            
            bool shouldBreak = false;
            for (const QString& flag : newLineFlags) {
                if (remaining.startsWith(flag)) {
                    shouldBreak = true;
                    break;
                }
            }
            
            if (shouldBreak && !currentLine.trimmed().isEmpty()) {
                outputLines.append(currentLine.trimmed());
                currentLine.clear();
            }
        }
        
        currentLine += c;
        i++;
    }
    
    // Don't forget last segment
    if (!currentLine.trimmed().isEmpty()) {
        outputLines.append(currentLine.trimmed());
    }
    
    // Post-process -filter_complex to add breaks at semicolons
    for (int j = 0; j < outputLines.size(); ++j) {
        QString& line = outputLines[j];
        if (line.startsWith("-filter_complex ")) {
            line = formatFilterComplex(line);
        }
    }
    
    // Post-process -map lines to put output path on its own line
    QStringList finalLines;
    for (const QString& line : outputLines) {
        if (line.startsWith("-map ")) {
            // Pattern: -map "[stream]" [codec flags] "output/path.wav"
            // Find all quoted strings in this line
            QList<QPair<int, int>> quotes; // start, end pairs
            int pos = 0;
            while (pos < line.length()) {
                int start = line.indexOf('"', pos);
                if (start < 0) break;
                int end = line.indexOf('"', start + 1);
                if (end < 0) break;
                quotes.append({start, end});
                pos = end + 1;
            }
            
            // If we have at least 2 quoted strings, the last one is the output path
            if (quotes.size() >= 2) {
                int pathStart = quotes.last().first;
                QString beforePath = line.left(pathStart).trimmed();
                QString pathPart = line.mid(pathStart);
                finalLines.append(beforePath);
                finalLines.append(pathPart);
                continue;
            }
        }
        finalLines.append(line);
    }
    
    // Join with continuation backslashes
    QString result;
    for (int j = 0; j < finalLines.size(); ++j) {
        result += finalLines[j];
        if (j < finalLines.size() - 1) {
            result += " \\\n";
        }
    }
    
    return result;
}

QString FFmpegFormatter::formatFilterComplex(const QString& content) {
    // Input: -filter_complex "...;...;..."
    // Output: same but with \n after semicolons
    
    int quoteStart = content.indexOf('"');
    int quoteEnd = content.lastIndexOf('"');
    
    if (quoteStart < 0 || quoteEnd <= quoteStart) {
        return content;  // Malformed, return as-is
    }
    
    QString prefix = content.left(quoteStart + 1);  // -filter_complex "
    QString suffix = "\"";                           // closing quote only
    QString inner = content.mid(quoteStart + 1, quoteEnd - quoteStart - 1);
    
    // Split on semicolons (filter chain separators)
    QStringList chains;
    QString currentChain;
    
    for (int i = 0; i < inner.length(); ++i) {
        QChar c = inner[i];
        if (c == ';') {
            chains.append(currentChain.trimmed());
            currentChain.clear();
        } else {
            currentChain += c;
        }
    }
    if (!currentChain.trimmed().isEmpty()) {
        chains.append(currentChain.trimmed());
    }
    
    if (chains.size() <= 1) {
        return content;  // No splitting needed
    }
    
    // Rejoin with line breaks after semicolons
    QString result = prefix;
    for (int i = 0; i < chains.size(); ++i) {
        result += chains[i];
        if (i < chains.size() - 1) {
            result += "; \\\n";
        }
    }
    result += suffix;
    
    return result;
}

QString FFmpegFormatter::stripFormatting(const QString& formatted) {
    QString result = formatted;
    
    // Remove backslash-newline sequences
    result.replace(QRegularExpression(" \\\\\\n\\s*"), " ");
    
    // Collapse multiple spaces
    result.replace(QRegularExpression("\\s+"), " ");
    
    return result.trimmed();
}

QStringList FFmpegFormatter::tokenize(const QString& command) {
    QStringList tokens;
    QString current;
    bool inQuotes = false;
    
    for (int i = 0; i < command.length(); ++i) {
        QChar c = command[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
        } else if (c.isSpace() && !inQuotes) {
            if (!current.isEmpty()) {
                tokens.append(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.isEmpty()) {
        tokens.append(current);
    }
    
    return tokens;
}