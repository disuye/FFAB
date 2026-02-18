#pragma once

#include <QMenu>
#include <QWidget>
#include <QString>
#include <QMap>
#include <QStringList>

class FilterChain;

/**
 * FilterMenuBuilder - Builds the "+ Add Filter" menu with categories
 * 
 * Organizes filters into logical categories:
 * - Volume & Dynamics
 * - Time & Pitch
 * - Visualization
 * - Custom Recipes
 * - Utility
 * 
 * As more filters are added, just register them in buildFilterRegistry()
 */
class FilterMenuBuilder {
public:
    struct FilterInfo {
        QString type;           // e.g. "ff-volume"
        QString displayName;    // e.g. "Volume"
        QString category;       // e.g. "Volume & Dynamics"
        QString description;    // e.g. "Adjust audio volume in dB"
    };
    
    // Create the complete filter menu
    static QMenu* createFilterMenu(QWidget* parent);
    
    // Get all available filters organized by category
    static QMap<QString, QList<FilterInfo>> getFiltersByCategory();
    
private:
    // Build the registry of all available filters
    static QList<FilterInfo> buildFilterRegistry();
    
    // Add filters from a category to a submenu
    static void addCategoryToMenu(QMenu* parentMenu, const QString& category, 
                                  const QList<FilterInfo>& filters);
};
