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
    double m_currentSpeed;  // Current connection speed in Mbps
    double m_lastTestedSpeed;  // Last measured speed

    // helpers
    int scanIntervalSec() const;
    int minSignalDiff() const;          // in percent points (legacy, for fallback)
    int switchSignalThreshold() const;  // signal percent (0..100, legacy)
    double minSpeedImprovementPercent() const;  // Minimum speed improvement to trigger switch
    double minAcceptableSpeedMbps() const;      // Minimum acceptable speed before considering switch
    bool pingCheck() const;

    // Speed-based decision making
    bool shouldSwitchBasedOnSpeed(const QString &currentSSID, const QString &candidateSSID,
                                   int currentSignal, int candidateSignal);
    double measureCurrentSpeed();
    double estimateSpeedForNetwork(const QString &ssid, int signal);

    void logEvent(const QString &event, const QString &detail = QString());
};
