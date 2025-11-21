#include "ConfigManager.h"
#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager c;
    return c;
}

ConfigManager::ConfigManager() {}

void ConfigManager::load() {
    // Nothing heavy here; DB exists already. Could preload defaults.
    // Ensure default settings exist:
    QSqlQuery q(Database::instance().db());
    struct Default { const char* k; const char* v; } defaults[] = {
        {"scan_interval", "30"},
        {"min_signal_diff", "10"},
        {"switch_signal_threshold", "-75"},
        {"preferred_networks", ""}, // comma-separated
    };
    for (auto &d : defaults) {
        QSqlQuery get(Database::instance().db());
        get.prepare("SELECT value FROM settings WHERE key = :k");
        get.bindValue(":k", d.k);
        if (!get.exec() || !get.next()) {
            QSqlQuery ins(Database::instance().db());
            ins.prepare("INSERT INTO settings (key, value) VALUES (:k, :v)");
            ins.bindValue(":k", d.k);
            ins.bindValue(":v", d.v);
            ins.exec();
        }
    }
}

QVariant ConfigManager::get(const QString &key, const QVariant &def) {
    QSqlQuery q(Database::instance().db());
    q.prepare("SELECT value FROM settings WHERE key=:k");
    q.bindValue(":k", key);
    if (q.exec() && q.next()) {
        return q.value(0);
    }
    return def;
}

void ConfigManager::set(const QString &key, const QVariant &value) {
    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO settings (key, value) VALUES (:k,:v) "
              "ON CONFLICT(key) DO UPDATE SET value=:v");
    q.bindValue(":k", key);
    q.bindValue(":v", value.toString());
    q.exec();
}
