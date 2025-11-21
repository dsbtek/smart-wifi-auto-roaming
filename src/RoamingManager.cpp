#include "RoamingManager.h"
#include "NetworkScanner.h"
#include "ConnectionManager.h"
#include "ConfigManager.h"
#include "Database.h"
#include "SpeedTester.h"
#include <QSqlQuery>
#include <QProcess>
#include <QDateTime>
#include <QDebug>

RoamingManager::RoamingManager(QObject *parent)
    : QObject(parent),
      m_timer(new QTimer(nullptr)),
      m_running(false),
      m_lastBest(QString()),
      m_stableCount(0),
      m_currentSpeed(0.0),
      m_lastTestedSpeed(0.0)
{
    // Timer is created on the heap with no parent so we can move it to the worker thread.
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout, this, &RoamingManager::onTick);

    // Move this manager and timer to the worker thread
    this->moveToThread(&m_workerThread);
    m_timer->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, m_timer, &QObject::deleteLater);
    connect(&m_workerThread, &QThread::started, this, [&](){ emit started(); });
    connect(&m_workerThread, &QThread::finished, this, [&](){ emit stopped(); });
}

RoamingManager::~RoamingManager() {
    stop();
    if (m_workerThread.isRunning()) {
        m_workerThread.quit();
        m_workerThread.wait();
    }
}

void RoamingManager::start() {
    if (m_running) return;
    m_running = true;
    m_workerThread.start();
    QMetaObject::invokeMethod(m_timer, "start", Qt::QueuedConnection, Q_ARG(int, scanIntervalSec() * 1000));
    logEvent("ROAM_START", "Roaming manager started");
}

void RoamingManager::stop() {
    if (!m_running) return;
    m_running = false;
    QMetaObject::invokeMethod(m_timer, "stop", Qt::QueuedConnection);
    m_workerThread.quit();
    m_workerThread.wait();
    logEvent("ROAM_STOP", "Roaming manager stopped");
}

int RoamingManager::scanIntervalSec() const {
    return ConfigManager::instance().get("scan_interval", 30).toInt();
}
int RoamingManager::minSignalDiff() const {
    return ConfigManager::instance().get("min_signal_diff", 8).toInt(); // use 8% default
}
int RoamingManager::switchSignalThreshold() const {
    return ConfigManager::instance().get("switch_signal_threshold", 30).toInt(); // percent
}
double RoamingManager::minSpeedImprovementPercent() const {
    return ConfigManager::instance().get("min_speed_improvement", 20.0).toDouble(); // 20% improvement
}
double RoamingManager::minAcceptableSpeedMbps() const {
    return ConfigManager::instance().get("min_acceptable_speed", 5.0).toDouble(); // 5 Mbps minimum
}

// simple ping to check connectivity
bool RoamingManager::pingCheck() const {
#ifdef Q_OS_WIN
    QString prog = "ping";
    QStringList args = { "-n", "1", "-w", "1000", "8.8.8.8" };
#else
    QString prog = "ping";
    QStringList args = { "-c", "1", "-W", "1", "8.8.8.8" };
#endif
    QProcess p;
    p.start(prog, args);
    bool ok = p.waitForFinished(1500);
    if (!ok) return false;
    return p.exitCode() == 0;
}

double RoamingManager::measureCurrentSpeed(const QString &ssid) {
    SpeedTester tester;
    SpeedTestResult result = tester.quickTest();

    if (result.success) {
        m_lastTestedSpeed = result.downloadSpeedMbps;
        logEvent("SPEED_TEST", QString("Current speed: %1 Mbps, Latency: %2 ms")
                 .arg(result.downloadSpeedMbps, 0, 'f', 2)
                 .arg(result.latencyMs, 0, 'f', 1));
        emit speedMeasured(ssid, result.downloadSpeedMbps);
        return result.downloadSpeedMbps;
    }

    logEvent("SPEED_TEST_FAIL", "Failed to measure current speed");
    return -1.0;
}

double RoamingManager::estimateSpeedForNetwork(const QString &ssid, int signal) {
    // This is a heuristic estimation based on signal strength
    // In a real scenario, we would need to actually connect and test
    // For now, we use signal strength as a proxy
    // Signal is 0-100, we estimate max speed based on signal quality

    Q_UNUSED(ssid);

    if (signal >= 80) {
        return 100.0; // Excellent signal, estimate high speed
    } else if (signal >= 60) {
        return 50.0;  // Good signal
    } else if (signal >= 40) {
        return 20.0;  // Fair signal
    } else if (signal >= 20) {
        return 10.0;  // Poor signal
    } else {
        return 5.0;   // Very poor signal
    }
}

bool RoamingManager::shouldSwitchBasedOnSpeed(const QString &currentSSID,
                                               const QString &candidateSSID,
                                               int currentSignal,
                                               int candidateSignal) {
    Q_UNUSED(candidateSSID);

    // Measure current actual speed
    double currentSpeed = measureCurrentSpeed(currentSSID);

    if (currentSpeed < 0) {
        // Speed test failed, fall back to signal-based decision
        logEvent("FALLBACK_SIGNAL", "Speed test failed, using signal strength");
        int diff = candidateSignal - currentSignal;
        return (diff >= minSignalDiff() && candidateSignal >= switchSignalThreshold());
    }

    m_currentSpeed = currentSpeed;

    // Check if current speed is below acceptable threshold
    if (currentSpeed < minAcceptableSpeedMbps()) {
        logEvent("SPEED_LOW", QString("Current speed %1 Mbps is below threshold %2 Mbps")
                 .arg(currentSpeed, 0, 'f', 2)
                 .arg(minAcceptableSpeedMbps(), 0, 'f', 2));

        // If candidate has significantly better signal, it's worth trying
        if (candidateSignal > currentSignal + minSignalDiff()) {
            logEvent("SWITCH_DECISION", QString("Low speed + better signal (%1 vs %2) - switching")
                     .arg(candidateSignal).arg(currentSignal));
            return true;
        }
    }

    // Estimate potential speed improvement based on signal difference
    double estimatedCandidateSpeed = estimateSpeedForNetwork(candidateSSID, candidateSignal);
    double speedImprovement = ((estimatedCandidateSpeed - currentSpeed) / currentSpeed) * 100.0;

    logEvent("SPEED_ANALYSIS", QString("Current: %1 Mbps, Estimated candidate: %2 Mbps, Improvement: %3%")
             .arg(currentSpeed, 0, 'f', 2)
             .arg(estimatedCandidateSpeed, 0, 'f', 2)
             .arg(speedImprovement, 0, 'f', 1));

    // Switch if estimated improvement is significant
    if (speedImprovement >= minSpeedImprovementPercent()) {
        logEvent("SWITCH_DECISION", QString("Speed improvement %1% exceeds threshold %2%")
                 .arg(speedImprovement, 0, 'f', 1)
                 .arg(minSpeedImprovementPercent(), 0, 'f', 1));
        return true;
    }

    return false;
}

void RoamingManager::onTick() {
    NetworkScanner scanner;
    ConnectionManager conn;

    auto nets = scanner.scan(); // list of WifiNetwork (ssid, signal)

    if (nets.isEmpty()) {
        logEvent("SCAN_EMPTY", "No networks visible");
        return;
    }

    // Emit network signals for chart updates
    for (const auto &n : nets) {
        emit networkSignal(n.ssid, n.signal);
    }

    // select best visible network by raw signal (0..100)
    struct Candidate { QString ssid; int sig; };
    Candidate best = { QString(), -1 };
    for (const auto &n : nets) {
        if (n.signal > best.sig) {
            best.ssid = n.ssid;
            best.sig = n.signal;
        }
    }

    // find active
    QString connected;
    {
        // use nmcli to see active
        QProcess p;
        p.start("nmcli", QStringList() << "-t" << "-f" << "ACTIVE,SSID" << "dev" << "wifi");
        p.waitForFinished(500);
        QString out = p.readAllStandardOutput().trimmed();
        for (const QString &line : out.split('\n')) {
            if (line.startsWith("yes:")) {
                QString ssid = line.section(':', 1);
                connected = ssid;
                break;
            }
        }
    }

    int currentSig = 0;
    for (const auto &n : nets) {
        if (n.ssid == connected) { currentSig = n.signal; break; }
    }

    logEvent("STATUS", QString("Connected=%1(sig:%2) Best=%3(sig:%4)")
             .arg(connected.isEmpty() ? "none" : connected)
             .arg(currentSig)
             .arg(best.ssid)
             .arg(best.sig));

    // decide to switch?
    if (best.ssid.isEmpty()) return;

    if (connected.isEmpty()) {
        // no connection -> connect to best immediately (but check ping first optional)
        logEvent("ACTION", QString("No connection -> connecting to %1").arg(best.ssid));
        bool ok = conn.connectTo(best.ssid);
        logEvent(ok ? "CONNECTED" : "CONNECT_FAILED", best.ssid);
        return;
    }

    // if best == connected -> nothing to do, but measure speed periodically
    if (best.ssid == connected) {
        m_stableCount = 0;
        m_lastBest = best.ssid;

        // Periodically measure current speed to keep track
        static int speedCheckCounter = 0;
        speedCheckCounter++;
        if (speedCheckCounter >= 5) { // Every 5 scans
            measureCurrentSpeed(connected);
            speedCheckCounter = 0;
        }
        return;
    }

    int diff = best.sig - currentSig;

    // Use speed-based decision making instead of just signal strength
    // Check if candidate network is worth considering based on signal first
    if (diff >= minSignalDiff() && best.sig >= switchSignalThreshold()) {
        m_stableCount++;
        logEvent("CANDIDATE", QString("%1 has better signal by %2%% (%3/%4 checks)")
                 .arg(best.ssid).arg(diff).arg(m_stableCount).arg(3));

        if (m_stableCount >= 3) {
            // Signal is consistently better, now check if speed justifies switching
            logEvent("SPEED_CHECK", QString("Evaluating if switch from %1 to %2 is worth it")
                     .arg(connected).arg(best.ssid));

            bool shouldSwitch = shouldSwitchBasedOnSpeed(connected, best.ssid,
                                                          currentSig, best.sig);

            if (shouldSwitch) {
                // Do a final ping check to ensure connectivity is OK before switching
                if (!pingCheck()) {
                    logEvent("PING_FAIL", "Current connection unstable; proceeding with switch");
                }

                logEvent("SWITCH", QString("Switching from %1 to %2 (Signal Δ=%3%%, Speed-based decision)")
                         .arg(connected, best.ssid).arg(diff));
                bool ok = conn.connectTo(best.ssid);
                logEvent(ok ? "SWITCH_OK" : "SWITCH_FAIL", best.ssid);

                // After switching, measure new speed
                if (ok) {
                    QThread::sleep(2); // Wait for connection to stabilize
                    measureCurrentSpeed(best.ssid);
                }
            } else {
                logEvent("SWITCH_REJECTED", QString("Speed test indicates %1 is not worth switching to")
                         .arg(best.ssid));
            }

            m_stableCount = 0;
            m_lastBest = best.ssid;
        }
    } else {
        // Signal difference not significant enough
        m_stableCount = 0;
    }
}

void RoamingManager::logEvent(const QString &event, const QString &detail) {
    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO logs (event, detail) VALUES (:e, :d)");
    q.bindValue(":e", event);
    q.bindValue(":d", detail);
    q.exec();
    emit logMessage(QString("[%1] %2").arg(event, detail));
}
