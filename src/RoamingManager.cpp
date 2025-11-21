#include "RoamingManager.h"
#include "NetworkScanner.h"
#include "ConnectionManager.h"
#include "ConfigManager.h"
#include "Database.h"
#include <QSqlQuery>
#include <QProcess>
#include <QDateTime>
#include <QDebug>

RoamingManager::RoamingManager(QObject *parent)
    : QObject(parent),
      m_timer(new QTimer(nullptr)),
      m_running(false),
      m_lastBest(QString()),
      m_stableCount(0)
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

void RoamingManager::onTick() {
    NetworkScanner scanner;
    ConnectionManager conn;

    auto nets = scanner.scan(); // list of WifiNetwork (ssid, signal)

    if (nets.isEmpty()) {
        logEvent("SCAN_EMPTY", "No networks visible");
        return;
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

    logEvent("STATUS", QString("Connected=%1(%2) Best=%3(%4)")
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

    // if best == connected -> nothing
    if (best.ssid == connected) {
        m_stableCount = 0;
        m_lastBest = best.ssid;
        return;
    }

    int diff = best.sig - currentSig;
    int threshold = minSignalDiff();

    // require diff >= threshold and stable across consecutive ticks
    if (diff >= threshold && best.sig >= switchSignalThreshold()) {
        m_stableCount++;
        logEvent("CANDIDATE", QString("%1 is %2 stronger (%3/%4)")
                 .arg(best.ssid).arg(diff).arg(m_stableCount).arg(3));
        if (m_stableCount >= 3) {
            // do a ping check to ensure connectivity is OK before switching
            if (!pingCheck()) {
                logEvent("PING_FAIL", "Current connection unstable; proceeding with cautious switch");
            }
            logEvent("SWITCH", QString("Switching from %1 to %2 (Δ=%3)").arg(connected, best.ssid).arg(diff));
            bool ok = conn.connectTo(best.ssid);
            logEvent(ok ? "SWITCH_OK" : "SWITCH_FAIL", best.ssid);
            m_stableCount = 0;
            m_lastBest = best.ssid;
        }
    } else {
        // not strong enough or not above threshold
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
