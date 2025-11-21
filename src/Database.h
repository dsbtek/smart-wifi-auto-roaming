#pragma once
#include <QSqlDatabase>
#include <QString>

class Database {
public:
    static Database& instance();
    bool initialize();
    QSqlDatabase &db();

private:
    Database();
    QSqlDatabase m_db;
    QString dbPath() const;
};
