#pragma once
#include <QWidget>

class BatchAdminPanel : public QWidget {
    Q_OBJECT
public:
    explicit BatchAdminPanel(QWidget* parent = nullptr);
    ~BatchAdminPanel() override;
};
