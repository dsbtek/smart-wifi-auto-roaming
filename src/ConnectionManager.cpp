#include "ConnectionManager.h"
#include <QProcess>
#include <QDebug>

ConnectionManager::ConnectionManager(QObject *parent) : QObject(parent) {}

bool ConnectionManager::connectTo(const QString &ssid) {
#ifdef Q_OS_LINUX
    // First, try to connect using existing connection profile
    QProcess p;
    QStringList args;
    args << "connection" << "up" << ssid;
    p.start("nmcli", args);
    if (!p.waitForFinished(10000)) {
        qDebug() << "ConnectionManager: Timeout waiting for nmcli";
        return false;
    }

    QString out = p.readAllStandardOutput();
    QString err = p.readAllStandardError();

    if (p.exitCode() == 0) {
        qDebug() << "ConnectionManager: Successfully connected to" << ssid;
        return true;
    }

    // If connection profile doesn't exist, try to connect directly to the network
    qDebug() << "ConnectionManager: Connection profile not found, trying device wifi connect...";
    qDebug() << "ConnectionManager: Error was:" << err;

    QProcess p2;
    QStringList args2;
    args2 << "device" << "wifi" << "connect" << ssid;
    p2.start("nmcli", args2);
    if (!p2.waitForFinished(15000)) {
        qDebug() << "ConnectionManager: Timeout waiting for device wifi connect";
        return false;
    }

    QString out2 = p2.readAllStandardOutput();
    QString err2 = p2.readAllStandardError();

    if (p2.exitCode() == 0) {
        qDebug() << "ConnectionManager: Successfully connected to" << ssid << "via device wifi connect";
        return true;
    } else {
        qDebug() << "ConnectionManager: Failed to connect to" << ssid;
        qDebug() << "ConnectionManager: Output:" << out2;
        qDebug() << "ConnectionManager: Error:" << err2;
        return false;
    }
#else
    Q_UNUSED(ssid);
    return false;
#endif
}

bool ConnectionManager::connectTo(const QString &ssid, const QString &password) {
#ifdef Q_OS_LINUX
    // Connect to a WiFi network with password
    QProcess p;
    QStringList args;
    args << "device" << "wifi" << "connect" << ssid << "password" << password;
    p.start("nmcli", args);
    if (!p.waitForFinished(15000)) {
        qDebug() << "ConnectionManager: Timeout waiting for connection with password";
        return false;
    }

    QString out = p.readAllStandardOutput();
    QString err = p.readAllStandardError();

    if (p.exitCode() == 0) {
        qDebug() << "ConnectionManager: Successfully connected to" << ssid << "with password";
        return true;
    } else {
        qDebug() << "ConnectionManager: Failed to connect to" << ssid << "with password";
        qDebug() << "ConnectionManager: Output:" << out;
        qDebug() << "ConnectionManager: Error:" << err;
        return false;
    }
#else
    Q_UNUSED(ssid);
    Q_UNUSED(password);
    return false;
#endif
}

bool ConnectionManager::disconnectDevice() {
#ifdef Q_OS_LINUX
    QProcess p;
    p.start("nmcli", QStringList() << "device" << "disconnect" << "wlan0");
    p.waitForFinished(5000);
    return p.exitCode() == 0;
#else
    return false;
#endif
}
