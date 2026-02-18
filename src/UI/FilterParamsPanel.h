#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

/**
 * FilterParamsPanel - Displays the parameter widget for the selected filter
 * 
 * When a filter is selected in FilterChainWidget, this panel shows
 * that filter's getParametersWidget() result.
 */
class FilterParamsPanel : public QWidget {
    Q_OBJECT
public:
    explicit FilterParamsPanel(QWidget* parent = nullptr);
    ~FilterParamsPanel() override;
    
    void setFilterWidget(QWidget* widget);
    void clearFilterWidget();
    
private:
    QVBoxLayout* mainLayout;
    QWidget* currentFilterWidget = nullptr;
    QLabel* noSelectionLabel = nullptr;
};
