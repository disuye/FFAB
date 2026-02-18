#include "FilterParamsPanel.h"
#include <QLabel>

FilterParamsPanel::FilterParamsPanel(QWidget* parent) : QWidget(parent) {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    
    noSelectionLabel = new QLabel("Select a filter to edit parameters");
    noSelectionLabel->setAlignment(Qt::AlignCenter);
    noSelectionLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(noSelectionLabel);
    // Remove: mainLayout->addStretch();  ← This was pushing content up
}

FilterParamsPanel::~FilterParamsPanel() = default;

void FilterParamsPanel::setFilterWidget(QWidget* widget) {
    // Remove previous widget
    clearFilterWidget();
    
    if (!widget) {
        noSelectionLabel->show();
        return;
    }
    
    // Hide the "no selection" label
    noSelectionLabel->hide();
    
    // Add new filter widget WITH STRETCH FACTOR
    currentFilterWidget = widget;
    mainLayout->insertWidget(0, currentFilterWidget, 1);  // ← Add stretch factor 1
    currentFilterWidget->show();
}

void FilterParamsPanel::clearFilterWidget() {
    if (currentFilterWidget) {
        mainLayout->removeWidget(currentFilterWidget);
        currentFilterWidget->hide();
        currentFilterWidget->setParent(nullptr);
        currentFilterWidget = nullptr;
    }
    noSelectionLabel->show();
}