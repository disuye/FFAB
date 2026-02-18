#ifndef COLLAPSIBLEHELPSECTION_H
#define COLLAPSIBLEHELPSECTION_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

/**
 * CollapsibleHelpSection - Reusable "How To" section for filter parameters
 * 
 * Displays help text in a collapsible section at the bottom of filter UIs.
 * Toggle button shows: "▶ How To" (collapsed) or "▼ How To" (expanded)
 */
class CollapsibleHelpSection : public QWidget {
    Q_OBJECT
    
public:
    explicit CollapsibleHelpSection(const QString& helpText, QWidget* parent = nullptr);
    
    void setHelpText(const QString& text);
    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }
    
private slots:
    void toggleExpanded();
    
private:
    QPushButton* toggleButton;
    QLabel* helpLabel;
    bool m_expanded;
    
    void updateToggleButton();
};

#endif // COLLAPSIBLEHELPSECTION_H
