#pragma once
#include <QString>
#include <QMap>

class Metadata {
public:
    void readFromFile(const QString& filepath);
    void writeToFile(const QString& filepath);
    void setMetadata(const QString& key, const QString& value);
    QString getMetadata(const QString& key) const;
private:
    QMap<QString, QString> data;
};
