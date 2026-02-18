#pragma once

#include <QSettings>
#include <QByteArray>
#include <QString>
#include <QStringList>

/**
 * Preferences - Centralized user preferences using QSettings
 * 
 * Stores:
 * - Window geometry (MainWindow, CommandViewWindow)
 * - Last used directories for file browsers
 * - Recent presets list
 * 
 * Storage location:
 * - macOS: ~/Library/Preferences/com.ffab.FFAB.plist
 * - Linux: ~/.config/FFAB/FFAB.conf
 * - Windows: Registry HKEY_CURRENT_USER/Software/FFAB/FFAB
 */
class Preferences {
public:
    static Preferences& instance();
    
    // ========== Window Geometry ==========
    
    // MainWindow
    void setMainWindowGeometry(const QByteArray& geometry);
    QByteArray mainWindowGeometry() const;
    
    void setMainWindowState(const QByteArray& state);
    QByteArray mainWindowState() const;
    
    // CommandViewWindow
    void setCommandWindowGeometry(const QByteArray& geometry);
    QByteArray commandWindowGeometry() const;
    
    void setCommandWindowVisible(bool visible);
    bool commandWindowVisible() const;
    
    // RegionPreviewWindow
    void setRegionWindowGeometry(const QByteArray& geometry);
    QByteArray regionWindowGeometry() const;
    
    // ========== File Browser Directories ==========
    
    // Main input file list
    void setLastInputDirectory(const QString& path);
    QString lastInputDirectory() const;
    
    // Output folder
    void setLastOutputDirectory(const QString& path);
    QString lastOutputDirectory() const;
    
    // AudioInputFilter file browser (shared across all instances)
    void setLastAudioInputDirectory(const QString& path);
    QString lastAudioInputDirectory() const;
    
    // Preset browser
    void setLastPresetDirectory(const QString& path);
    QString lastPresetDirectory() const;
    
    // ========== Recent Presets ==========
    
    void addRecentPreset(const QString& path);
    QStringList recentPresets() const;
    void clearRecentPresets();
    
    // Force sync to disk
    void sync();
    
private:
    Preferences();
    ~Preferences() = default;
    
    // Prevent copying
    Preferences(const Preferences&) = delete;
    Preferences& operator=(const Preferences&) = delete;
    
    QSettings m_settings;
    
    static constexpr int MAX_RECENT_PRESETS = 10;
};
