#pragma once
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QString>
#include <QAtomicInt>

class NetworkScanner;
class ConnectionManager;

class RoamingManager : public QObject {
    Q_OBJECT
public:
    explicit RoamingManager(QObject *parent = nullptr);
    ~RoamingManager();

    // call to start the roaming loop (will run in its own thread)
    void start();
    void stop();

signals:
    void started();
    void stopped();
    void logMessage(const QString &msg);

private slots:
    void onTick();

private:
    QTimer *m_timer;
    QThread m_workerThread;
    bool m_running;

    // logic state
    QString m_lastBest;
    int m_stableCount;

    // helpers
    int scanIntervalSec() const;
    int minSignalDiff() const;          // in percent points
    int switchSignalThreshold() const;  // signal percent (0..100)
    bool pingCheck() const;

    void logEvent(const QString &event, const QString &detail = QString());
};
