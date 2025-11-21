#include "Database.h"
#include <QStandardPaths>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Database& Database::instance() {
    static Database inst;
    return inst;
}

Database::Database() {}

QString Database::dbPath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/smart_auto_roam";
    QDir().mkpath(dir);
    return dir + "/auto_roam.db";
}

bool Database::initialize() {
    QString path = dbPath();
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    if (!m_db.open()) {
        qCritical() << "Failed to open DB:" << m_db.lastError().text();
        return false;
    }
    QSqlQuery q(m_db);
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS networks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ssid TEXT UNIQUE NOT NULL,
            last_signal INTEGER,
            preferred INTEGER DEFAULT 0,
            last_connected DATETIME
        );
    )");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT
        );
    )");
    q.exec(R"(
        CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
            event TEXT,
            detail TEXT
        );
    )");
    return true;
}

QSqlDatabase &Database::db() {
    return m_db;
}
