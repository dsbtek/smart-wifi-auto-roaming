#pragma once
#include <QObject>
#include <QString>

struct SpeedTestResult {
    double downloadSpeedMbps;  // Download speed in Mbps
    double uploadSpeedMbps;    // Upload speed in Mbps
    double latencyMs;          // Latency in milliseconds
    bool success;              // Whether the test was successful
    QString errorMessage;      // Error message if failed
    
    SpeedTestResult() 
        : downloadSpeedMbps(0.0), uploadSpeedMbps(0.0), 
          latencyMs(0.0), success(false) {}
};

class SpeedTester : public QObject {
    Q_OBJECT
public:
    explicit SpeedTester(QObject *parent = nullptr);
    
    // Perform a quick speed test (download only, smaller file)
    SpeedTestResult quickTest();
    
    // Perform a full speed test (download + upload)
    SpeedTestResult fullTest();
    
    // Get current connection speed from system (link speed, not actual throughput)
    double getLinkSpeed();
    
    // Measure latency only (ping test)
    double measureLatency(const QString &host = "8.8.8.8");
    
private:
    // Download a file and measure speed
    double measureDownloadSpeed(const QString &url, int timeoutMs = 5000);
    
    // Upload data and measure speed
    double measureUploadSpeed(const QString &url, int timeoutMs = 5000);
    
    // Test URLs for speed measurement
    QString getTestUrl() const;
    
    // Maximum time for quick test (ms) - increased for 1MB file
    static constexpr int QUICK_TEST_TIMEOUT = 5000;

    // Maximum time for full test (ms)
    static constexpr int FULL_TEST_TIMEOUT = 15000;
};

