#include "ConnectionManager.h"
#include <QProcess>
#include <QDebug>

ConnectionManager::ConnectionManager(QObject *parent) : QObject(parent) {}

bool ConnectionManager::connectTo(const QString &ssid) {
#ifdef Q_OS_LINUX
    // Use nmcli to connect to a known connection/profile named exactly as SSID
    QProcess p;
    QStringList args;
    args << "connection" << "up" << ssid;
    p.start("nmcli", args);
    if (!p.waitForFinished(10000)) return false;
    QString out = p.readAllStandardOutput();
    QString err = p.readAllStandardError();
    Q_UNUSED(err);
    return p.exitCode() == 0;
#else
    Q_UNUSED(ssid);
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
