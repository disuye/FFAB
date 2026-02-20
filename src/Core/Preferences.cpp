#include "Preferences.h"
#include <QStandardPaths>
#include <QFileInfo>

Preferences& Preferences::instance() {
    static Preferences instance;
    return instance;
}

Preferences::Preferences()
    : m_settings() {
}

// ========== Window Geometry ==========

void Preferences::setMainWindowGeometry(const QByteArray& geometry) {
    m_settings.setValue("MainWindow/geometry", geometry);
}

QByteArray Preferences::mainWindowGeometry() const {
    return m_settings.value("MainWindow/geometry").toByteArray();
}

void Preferences::setMainWindowState(const QByteArray& state) {
    m_settings.setValue("MainWindow/state", state);
}

QByteArray Preferences::mainWindowState() const {
    return m_settings.value("MainWindow/state").toByteArray();
}

void Preferences::setCommandWindowGeometry(const QByteArray& geometry) {
    m_settings.setValue("CommandWindow/geometry", geometry);
}

QByteArray Preferences::commandWindowGeometry() const {
    return m_settings.value("CommandWindow/geometry").toByteArray();
}

void Preferences::setCommandWindowVisible(bool visible) {
    m_settings.setValue("CommandWindow/visible", visible);
}

bool Preferences::commandWindowVisible() const {
    return m_settings.value("CommandWindow/visible", false).toBool();
}

void Preferences::setRegionWindowGeometry(const QByteArray& geometry) {
    m_settings.setValue("RegionWindow/geometry", geometry);
}

QByteArray Preferences::regionWindowGeometry() const {
    return m_settings.value("RegionWindow/geometry").toByteArray();
}

// ========== File Browser Directories ==========

void Preferences::setLastInputDirectory(const QString& path) {
    m_settings.setValue("Directories/lastInput", path);
}

QString Preferences::lastInputDirectory() const {
    QString path = m_settings.value("Directories/lastInput").toString();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
    return path;
}

void Preferences::setLastOutputDirectory(const QString& path) {
    m_settings.setValue("Directories/lastOutput", path);
}

QString Preferences::lastOutputDirectory() const {
    QString path = m_settings.value("Directories/lastOutput").toString();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
    return path;
}

void Preferences::setLastAudioInputDirectory(const QString& path) {
    m_settings.setValue("Directories/lastAudioInput", path);
}

QString Preferences::lastAudioInputDirectory() const {
    QString path = m_settings.value("Directories/lastAudioInput").toString();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    }
    return path;
}

void Preferences::setLastPresetDirectory(const QString& path) {
    m_settings.setValue("Directories/lastPreset", path);
}

QString Preferences::lastPresetDirectory() const {
    QString path = m_settings.value("Directories/lastPreset").toString();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    return path;
}

// ========== Recent Presets ==========

void Preferences::addRecentPreset(const QString& path) {
    QStringList recent = recentPresets();
    
    // Remove if already exists (will re-add at top)
    recent.removeAll(path);
    
    // Add to front
    recent.prepend(path);
    
    // Trim to max size
    while (recent.size() > MAX_RECENT_PRESETS) {
        recent.removeLast();
    }
    
    m_settings.setValue("RecentPresets/list", recent);
}

QStringList Preferences::recentPresets() const {
    return m_settings.value("RecentPresets/list").toStringList();
}

void Preferences::clearRecentPresets() {
    m_settings.setValue("RecentPresets/list", QStringList());
}

void Preferences::sync() {
    m_settings.sync();
}
