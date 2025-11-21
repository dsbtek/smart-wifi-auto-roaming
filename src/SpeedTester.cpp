#include "SpeedTester.h"
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

SpeedTester::SpeedTester(QObject *parent) : QObject(parent) {}

SpeedTestResult SpeedTester::quickTest() {
    SpeedTestResult result;
    
    // Measure latency first (quick)
    result.latencyMs = measureLatency();
    if (result.latencyMs < 0) {
        result.success = false;
        result.errorMessage = "Failed to measure latency";
        return result;
    }
    
    // Measure download speed with a small file
    result.downloadSpeedMbps = measureDownloadSpeed(getTestUrl(), QUICK_TEST_TIMEOUT);
    
    if (result.downloadSpeedMbps > 0) {
        result.success = true;
    } else {
        result.success = false;
        result.errorMessage = "Failed to measure download speed";
    }
    
    return result;
}

SpeedTestResult SpeedTester::fullTest() {
    SpeedTestResult result;
    
    result.latencyMs = measureLatency();
    result.downloadSpeedMbps = measureDownloadSpeed(getTestUrl(), FULL_TEST_TIMEOUT);
    // Upload test can be added here if needed
    result.uploadSpeedMbps = 0.0; // Not implemented yet
    
    result.success = (result.downloadSpeedMbps > 0);
    if (!result.success) {
        result.errorMessage = "Speed test failed";
    }
    
    return result;
}

double SpeedTester::getLinkSpeed() {
#ifdef Q_OS_LINUX
    // Get link speed from iwconfig or iw
    QProcess p;
    p.start("iwconfig", QStringList());
    p.waitForFinished(1000);
    QString output = p.readAllStandardOutput();
    
    // Parse "Bit Rate=XX Mb/s"
    QRegularExpression re("Bit Rate[=:]\\s*(\\d+(?:\\.\\d+)?)\\s*Mb/s");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    
    // Try alternative method with iw
    p.start("bash", QStringList() << "-c" << "iw dev | grep -A 10 'Interface' | grep 'tx bitrate'");
    p.waitForFinished(1000);
    output = p.readAllStandardOutput();
    
    // Parse "tx bitrate: XX.X MBit/s"
    QRegularExpression re2("tx bitrate:\\s*(\\d+(?:\\.\\d+)?)\\s*MBit/s");
    match = re2.match(output);
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
#endif
    return -1.0; // Unable to determine
}

double SpeedTester::measureLatency(const QString &host) {
#ifdef Q_OS_WIN
    QString prog = "ping";
    QStringList args = { "-n", "3", "-w", "1000", host };
#else
    QString prog = "ping";
    QStringList args = { "-c", "3", "-W", "1", host };
#endif

    QProcess p;
    p.start(prog, args);
    if (!p.waitForFinished(4000)) {
        qDebug() << "SpeedTester: Ping timeout for" << host;
        return -1.0;
    }

    if (p.exitCode() != 0) {
        QString err = p.readAllStandardError();
        qDebug() << "SpeedTester: Ping failed for" << host << ":" << err;
        return -1.0;
    }

    QString output = p.readAllStandardOutput();

    // Parse average latency from ping output
    // Linux: "rtt min/avg/max/mdev = 10.123/15.456/20.789/5.123 ms"
    // Windows: "Average = 15ms"
#ifdef Q_OS_WIN
    QRegularExpression re("Average\\s*=\\s*(\\d+)ms");
#else
    QRegularExpression re("rtt min/avg/max/mdev = [\\d.]+/([\\d.]+)/");
#endif

    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        double latency = match.captured(1).toDouble();
        qDebug() << "SpeedTester: Latency to" << host << ":" << latency << "ms";
        return latency;
    }

    qDebug() << "SpeedTester: Could not parse ping output:" << output;
    return -1.0;
}

double SpeedTester::measureDownloadSpeed(const QString &url, int timeoutMs) {
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "SmartAutoRoam/1.0");

    QElapsedTimer timer;
    timer.start();

    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeoutTimer.start(timeoutMs);
    loop.exec();

    qint64 elapsedMs = timer.elapsed();

    if (!reply->isFinished()) {
        qDebug() << "SpeedTester: Download timed out after" << elapsedMs << "ms";
        reply->abort();
        reply->deleteLater();
        return -1.0;
    }

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "SpeedTester: Download error:" << reply->errorString();
        reply->deleteLater();
        return -1.0;
    }

    // Read all data to get total bytes received
    QByteArray data = reply->readAll();
    qint64 bytesReceived = data.size();

    reply->deleteLater();

    if (elapsedMs == 0 || bytesReceived == 0) {
        qDebug() << "SpeedTester: Invalid measurement - elapsed:" << elapsedMs << "bytes:" << bytesReceived;
        return -1.0;
    }

    // Calculate speed in Mbps
    double speedMbps = (bytesReceived * 8.0) / (elapsedMs * 1000.0);
    qDebug() << "SpeedTester: Downloaded" << bytesReceived << "bytes in" << elapsedMs << "ms =" << speedMbps << "Mbps";
    return speedMbps;
}

double SpeedTester::measureUploadSpeed(const QString &url, int timeoutMs) {
    // Upload speed test not implemented yet
    // Would require a server endpoint that accepts POST/PUT requests
    Q_UNUSED(url);
    Q_UNUSED(timeoutMs);
    return -1.0;
}

QString SpeedTester::getTestUrl() const {
    // Use a small file from a reliable source for testing
    // Using a 100KB file for quick tests (adjust size based on needs)
    // Alternative URLs if one fails:
    // - "http://ipv4.download.thinkbroadband.com/100MB.zip" (larger)
    // - "http://proof.ovh.net/files/1Mb.dat"
    // - "http://speedtest.ftp.otenet.gr/files/test1Mb.db"
    return "http://ipv4.download.thinkbroadband.com/10MB.zip";
}

