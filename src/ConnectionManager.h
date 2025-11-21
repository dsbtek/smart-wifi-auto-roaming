#pragma once
#include <QObject>
#include <QString>

class ConnectionManager : public QObject {
    Q_OBJECT
public:
    explicit ConnectionManager(QObject *parent = nullptr);
    bool connectTo(const QString &ssid);
    bool disconnectDevice();
};
