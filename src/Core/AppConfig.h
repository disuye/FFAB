#pragma once

#include <QColor>
#include <QString>

// Application version
#define VERSION_STR "1.0.3"

// OS Light/Dark mode mid-grey link color
#define LINK_COLOR "#808080"

/**
 * FilterColors - Centralized color generation for filter IDs
 * 
 * Generates consistent HSL-based colors for:
 * - Filter row backgrounds (25% opacity)
 * - Filter ID badge backgrounds (50% opacity)  
 * - Contrast-aware text colors
 */
namespace FilterColors {
    
    // Core HSL components from filter ID
    inline int hue(int filterId) { 
    //    return (filterId * 13) % 360; 
    int rawHue = (filterId * 13) % 200;  // 0-179 range (half HSL wheel)
    return (rawHue + 50) % 360; // blue -> red -> yellow, skip green
    }
    
    inline int saturation(int filterId) { 
        return 15 + ((filterId * 7) % 17); 
    }
    
    inline int lightness(int filterId) { 
        return 20 + ((filterId * 7) % 20); 
    }
    
    // Row background (25% opacity) - used for filter chain rows
    inline QColor rowBackground(int filterId) {
        return QColor::fromHslF(
            hue(filterId) / 360.0,
            saturation(filterId) / 100.0,
            lightness(filterId) / 100.0,
            0.2
        );
    }
    
    // Label/badge background (50% opacity) - used for ID badges
    inline QColor labelBackground(int filterId) {
        return QColor::fromHslF(
            hue(filterId) / 360.0,
            saturation(filterId) / 100.0,
            lightness(filterId) / 100.0,
            0.5
        );
    }
    
    // Contrast-aware text color (white on dark, black on light)
    inline QColor textColor(int filterId) {
        return (lightness(filterId) > 50) 
            ? QColor(0, 0, 0, 204)       // Dark text (80% opacity)
            : QColor(255, 255, 255, 230); // Light text (90% opacity)
    }
    
    // Hex ID string (e.g., "002A")
    inline QString hexId(int filterId) {
        return QString("%1").arg(filterId, 4, 16, QChar('0')).toUpper();
    }
}
