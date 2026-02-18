#include "Metadata.h"

void Metadata::readFromFile(const QString& filepath) {
}

void Metadata::writeToFile(const QString& filepath) {
}

void Metadata::setMetadata(const QString& key, const QString& value) {
    data[key] = value;
}

QString Metadata::getMetadata(const QString& key) const {
    return data.value(key, "");
}
