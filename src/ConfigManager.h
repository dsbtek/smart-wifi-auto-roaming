#pragma once
#include <QString>
#include <QVariant>

class ConfigManager {
public:
    static ConfigManager& instance();
    void load();
    QVariant get(const QString &key, const QVariant &def = QVariant());
    void set(const QString &key, const QVariant &value);

private:
    ConfigManager();
};
