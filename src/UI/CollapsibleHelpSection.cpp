#include "CollapsibleHelpSection.h"
#include "Utils/UnicodeSymbols.h"
#include <QVBoxLayout>

CollapsibleHelpSection::CollapsibleHelpSection(const QString& helpText, QWidget* parent)
    : QWidget(parent), m_expanded(false) {
    
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    
    // Toggle button
    toggleButton = new QPushButton();
    toggleButton->setFlat(true);
    toggleButton->setCursor(Qt::PointingHandCursor);
    toggleButton->setStyleSheet("QPushButton { text-align: left; padding: 4px; }");
    connect(toggleButton, &QPushButton::clicked, this, &CollapsibleHelpSection::toggleExpanded);
    layout->addWidget(toggleButton);
    
    // Help content label
    helpLabel = new QLabel(helpText);
    helpLabel->setWordWrap(true);
    helpLabel->setTextFormat(Qt::RichText);
    helpLabel->setStyleSheet("QLabel { padding: 8px; border: 1px solid #808080; border-radius: 4px; }");
    helpLabel->setVisible(false);  // Start collapsed
    layout->addWidget(helpLabel);
    
    updateToggleButton();
}

void CollapsibleHelpSection::setHelpText(const QString& text) {
    helpLabel->setText(text);
}

void CollapsibleHelpSection::setExpanded(bool expanded) {
    if (m_expanded != expanded) {
        m_expanded = expanded;
        helpLabel->setVisible(m_expanded);
        updateToggleButton();
    }
}

void CollapsibleHelpSection::toggleExpanded() {
    setExpanded(!m_expanded);
}

void CollapsibleHelpSection::updateToggleButton() {
    QString arrow = m_expanded ? Sym::TriangleDown : Sym::Play;
    toggleButton->setText(arrow + " How To");
}
