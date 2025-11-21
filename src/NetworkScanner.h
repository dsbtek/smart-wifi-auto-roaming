#pragma once
#include <QObject>
#include <QVector>
#include <QString>

struct WifiNetwork {
    QString ssid;
    int signal; // 0..100
};

class NetworkScanner : public QObject {
    Q_OBJECT
public:
    explicit NetworkScanner(QObject *parent = nullptr);
    QVector<WifiNetwork> scan(); // blocking quick scan
};
