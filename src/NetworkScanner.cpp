#include "NetworkScanner.h"
#include <QProcess>
#include <QRegularExpression>
#include <QDebug>

NetworkScanner::NetworkScanner(QObject *parent) : QObject(parent) {}

QVector<WifiNetwork> NetworkScanner::scan() {
    QVector<WifiNetwork> list;

#ifdef Q_OS_LINUX
    // Uses nmcli on Linux — easy and reliable
    QProcess p;
    p.start("nmcli", QStringList() << "-t" << "-f" << "SSID,SIGNAL" << "dev" << "wifi");
    p.waitForFinished(3000);
    QString out = p.readAllStandardOutput();
    for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
        QStringList parts = line.split(':');
        if (parts.size() >= 2) {
            WifiNetwork w;
            w.ssid = parts.at(0).trimmed();
            w.signal = parts.at(1).toInt();
            list.append(w);
        }
    }
#else
    // TODO: implement platform-specific scanning for macOS/Windows (CoreWLAN / Native APIs)
    Q_UNUSED(list);
#endif

    return list;
}
