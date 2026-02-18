#pragma once

#include <QString>
#include <QWidget>
#include <QObject>
#include <memory>
#include <QJsonObject>

/**
 * FFAB Filter Naming Conventions:
 * 
 * 1. ff-name.cpp / FFName class
 *    Raw FFmpeg filters with GUI wrapper.
 *    Examples: ff-volume, ff-afir, ff-sidechaincompress
 *    Just exposes FFmpeg parameters, no custom logic.
 * 
 * 2. CamelCaseFilter.cpp / CamelCaseFilter class  
 *    Custom composite filters usually combining 2+ FFmpeg filters.
 *    Examples: (future) LoudnessNormalizer, StereoWidener
 *    Chains multiple FFmpeg filters into one logical operation.
 * 
 * 3. SmartCamelName.cpp / SmartCamelName class
 *    Utility filters using FFmpeg + FFAB core features.
 *    Examples: SmartAuxReturn, AudioInputFilter
 *    Aware of filter chain context, routing, other filters, etc.
*/

class BaseFilter : public QObject {
    Q_OBJECT
public:
    enum class Position {
        INPUT,
        MIDDLE,
        OUTPUT
    };

    virtual ~BaseFilter() = default;

    virtual QString displayName() const = 0;
    virtual QString filterType() const = 0;
    virtual QWidget* getParametersWidget() = 0;
    virtual QString buildFFmpegFlags() const = 0;
    virtual QString getDefaultCustomCommandTemplate() const;
    
    virtual void toJSON(QJsonObject& json) const = 0;
    virtual void fromJSON(const QJsonObject& json) = 0;

    Position getPosition() const { return position; }
    virtual bool isAnalysisTwoInputFilter() const { return false; }
    
    // Filter ID system - each filter gets a unique sequential ID
    int getFilterId() const { return m_filterId; }
    virtual void setFilterId(int id) { m_filterId = id; }
    
    // Custom output stream system
    // When enabled, filter outputs to [HEX_ID] and branches from the main chain
    // This allows multi-input filters to consume this output as a sidechain
    bool usesCustomOutputStream() const { return m_useCustomOutputStream; }
    void setCustomOutputStream(bool enabled) { m_useCustomOutputStream = enabled; }
    
    // Complex routing flag for multi-input filters
    // When true, filter handles its own input label routing internally
    // (e.g., AFIR with amix splits [0:a] internally and only needs [N:a] for IR)
    // Command builder will prepend mainChainInput but NOT all sidechain inputs
    virtual bool handlesOwnInputRouting() const { return false; }

protected:
    Position position = Position::MIDDLE;
    int m_filterId = -1;  // -1 = unassigned
    bool m_useCustomOutputStream = false;  // false = normal chain, true = branch to custom stream
};