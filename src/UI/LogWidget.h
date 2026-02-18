#pragma once
#include <QWidget>

class LogWidget : public QWidget {
    Q_OBJECT
public:
    explicit LogWidget(QWidget* parent = nullptr);
    ~LogWidget() override;
    void addLogMessage(const QString& message);
};
