#include "MainWindow.h"
#include "ui_MainWindow.h" // generated from MainWindow.ui by uic
#include "Database.h"
#include "ConfigManager.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDateTime>
#include <QInputDialog>
#include <QScrollBar>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QPixmap>

// QtCharts is optional - charts will be disabled if not available
// To enable charts, install: sudo apt install qt6-charts-dev
#ifdef HAVE_QTCHARTS
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_trayIcon(nullptr),
      m_trayMenu(nullptr),
      m_signalChartView(nullptr),
      m_speedChartView(nullptr),
      m_speedSeries(nullptr)
{
    ui->setupUi(this);
    Database::instance().initialize();
    ConfigManager::instance().load();

    setupTrayIcon();
    setupCharts();

    loadSettingsToUi();
    loadRecentLogs();

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::refreshNetworks);
    int interval = ConfigManager::instance().get("scan_interval", 30).toInt();
    m_timer.start(interval * 1000);
    refreshNetworks();
}

MainWindow::~MainWindow() {
    delete ui;
    if (m_trayIcon) {
        delete m_trayIcon;
    }
}

void MainWindow::loadSettingsToUi() {
    ui->spinScanInterval->setValue(ConfigManager::instance().get("scan_interval", 30).toInt());
    ui->spinMinDiff->setValue(ConfigManager::instance().get("min_signal_diff", 10).toInt());
    ui->spinThreshold->setValue(ConfigManager::instance().get("switch_signal_threshold", -75).toInt());
    ui->spinMinSpeedImprovement->setValue(ConfigManager::instance().get("min_speed_improvement", 20.0).toDouble());
    ui->spinMinAcceptableSpeed->setValue(ConfigManager::instance().get("min_acceptable_speed", 5.0).toDouble());
    // load preferred networks (comma separated)
    QString prefs = ConfigManager::instance().get("preferred_networks", "").toString();
    ui->linePreferred->setText(prefs);
}

void MainWindow::saveSettingsFromUi() {
    ConfigManager::instance().set("scan_interval", ui->spinScanInterval->value());
    ConfigManager::instance().set("min_signal_diff", ui->spinMinDiff->value());
    ConfigManager::instance().set("switch_signal_threshold", ui->spinThreshold->value());
    ConfigManager::instance().set("min_speed_improvement", ui->spinMinSpeedImprovement->value());
    ConfigManager::instance().set("min_acceptable_speed", ui->spinMinAcceptableSpeed->value());
    ConfigManager::instance().set("preferred_networks", ui->linePreferred->text());
    m_timer.start(ui->spinScanInterval->value() * 1000);
}

void MainWindow::refreshNetworks() {
    ui->networkTable->setRowCount(0);
    QVector<WifiNetwork> networks = m_scanner.scan();
    int row = 0;
    for (const WifiNetwork &n : networks) {
        ui->networkTable->insertRow(row);
        ui->networkTable->setItem(row, 0, new QTableWidgetItem(n.ssid));
        ui->networkTable->setItem(row, 1, new QTableWidgetItem(QString::number(n.signal)));
        row++;

        // Update signal chart
        updateSignalChart(n.ssid, n.signal);
    }
}

void MainWindow::on_connectButton_clicked() {
    auto sel = ui->networkTable->selectedItems();
    if (sel.isEmpty()) return;
    QString ssid = sel.first()->text();

    // First try to connect without password (for open networks or saved connections)
    bool ok = m_conn.connectTo(ssid);

    // If that fails, prompt for password
    if (!ok) {
        bool passwordOk;
        QString password = QInputDialog::getText(this,
                                                  "WiFi Password",
                                                  QString("Enter password for %1:").arg(ssid),
                                                  QLineEdit::Password,
                                                  QString(),
                                                  &passwordOk);

        if (passwordOk && !password.isEmpty()) {
            ok = m_conn.connectTo(ssid, password);
        }
    }

    if (ok) {
        logEvent("CONNECT", ssid);
        QMessageBox::information(this, "Connected", QString("Connected to %1").arg(ssid));
    } else {
        logEvent("CONNECT_FAILED", ssid);
        QMessageBox::warning(this, "Failed", QString("Failed to connect to %1.\n\nCheck the password or network settings.").arg(ssid));
    }
}

void MainWindow::on_settingsButton_clicked() {
    saveSettingsFromUi();
    QMessageBox::information(this, "Settings", "Settings saved.");
}

void MainWindow::logEvent(const QString &event, const QString &detail) {
    // Save to database
    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO logs (event, detail) VALUES (:e, :d)");
    q.bindValue(":e", event);
    q.bindValue(":d", detail);
    q.exec();

    // Display in UI
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString logMessage = QString("[%1] %2: %3").arg(timestamp, event, detail);
    appendLog(logMessage);
}

void MainWindow::appendLog(const QString &message) {
    ui->logView->append(message);
    // Auto-scroll to bottom
    ui->logView->verticalScrollBar()->setValue(ui->logView->verticalScrollBar()->maximum());
}

void MainWindow::setupTrayIcon() {
    // Check if system tray is available
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "Warning: System tray is not available on this system!";
        appendLog("[WARNING] System tray not available. Tray icon will not be shown.");
        return;
    }

    // Create tray icon
    m_trayIcon = new QSystemTrayIcon(this);

    // Create icon with multiple sizes for better tray compatibility
    // PNG works better than SVG for system tray icons on Linux
    QIcon trayIcon;

    // Add pre-rendered PNG icons at different sizes
    trayIcon.addFile(":/resources/icons/wifi_16x16.png", QSize(16, 16));
    trayIcon.addFile(":/resources/icons/wifi_22x22.png", QSize(22, 22));
    trayIcon.addFile(":/resources/icons/wifi_24x24.png", QSize(24, 24));
    trayIcon.addFile(":/resources/icons/wifi_32x32.png", QSize(32, 32));
    trayIcon.addFile(":/resources/icons/wifi_48x48.png", QSize(48, 48));

    // Fallback to main PNG if specific sizes not found
    if (trayIcon.isNull()) {
        trayIcon.addFile(":/resources/icons/wifi.png");
    }

    // Last resort fallback: XPM
    if (trayIcon.isNull()) {
        trayIcon.addFile(":/resources/icons/wifi.xpm");
    }

    // Ultimate fallback: create a simple colored pixmap
    if (trayIcon.isNull()) {
        QPixmap fallback(22, 22);
        fallback.fill(Qt::transparent);
        QPainter painter(&fallback);
        painter.setBrush(QColor("#2196F3"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(6, 6, 10, 10);
        trayIcon = QIcon(fallback);
    }

    m_trayIcon->setIcon(trayIcon);
    m_trayIcon->setToolTip("SmartAutoRoam - WiFi Auto-Roaming");

    // Debug: Check if icon is valid
    if (trayIcon.isNull()) {
        qDebug() << "Warning: Tray icon is null!";
    } else {
        qDebug() << "Tray icon loaded successfully";
        qDebug() << "Available sizes:" << trayIcon.availableSizes();
    }

    // Create tray menu
    m_trayMenu = new QMenu(this);

    m_showAction = m_trayMenu->addAction("Show Window");
    connect(m_showAction, &QAction::triggered, this, &MainWindow::showWindow);

    m_trayMenu->addSeparator();

    m_toggleRoamingAction = m_trayMenu->addAction("Enable Auto-Roaming");
    m_toggleRoamingAction->setCheckable(true);
    m_toggleRoamingAction->setChecked(true);
    connect(m_toggleRoamingAction, &QAction::triggered, this, &MainWindow::toggleAutoRoaming);

    m_trayMenu->addSeparator();

    m_quitAction = m_trayMenu->addAction("Quit");
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);

    // Connect activation signal
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);

    m_trayIcon->show();

    // Show notification
    m_trayIcon->showMessage("SmartAutoRoam", "Application started. Auto-roaming is active.",
                            QSystemTrayIcon::Information, 3000);
}

void MainWindow::setupCharts() {
#ifdef HAVE_QTCHARTS
    // Setup Signal Strength Chart
    QChart *signalChart = new QChart();
    signalChart->setTitle("Signal Strength Over Time");
    signalChart->setAnimationOptions(QChart::SeriesAnimations);

    QDateTimeAxis *signalAxisX = new QDateTimeAxis;
    signalAxisX->setFormat("HH:mm:ss");
    signalAxisX->setTitleText("Time");
    signalChart->addAxis(signalAxisX, Qt::AlignBottom);

    QValueAxis *signalAxisY = new QValueAxis;
    signalAxisY->setRange(0, 100);
    signalAxisY->setTitleText("Signal (%)");
    signalChart->addAxis(signalAxisY, Qt::AlignLeft);

    m_signalChartView = new QChartView(signalChart);
    m_signalChartView->setRenderHint(QPainter::Antialiasing);

    // Add chart to container
    QVBoxLayout *signalLayout = new QVBoxLayout(ui->signalChartContainer);
    signalLayout->setContentsMargins(0, 0, 0, 0);
    signalLayout->addWidget(m_signalChartView);

    // Setup Speed Chart
    QChart *speedChart = new QChart();
    speedChart->setTitle("Download Speed Over Time");
    speedChart->setAnimationOptions(QChart::SeriesAnimations);

    m_speedSeries = new QLineSeries();
    m_speedSeries->setName("Speed (Mbps)");
    speedChart->addSeries(m_speedSeries);

    QDateTimeAxis *speedAxisX = new QDateTimeAxis;
    speedAxisX->setFormat("HH:mm:ss");
    speedAxisX->setTitleText("Time");
    speedChart->addAxis(speedAxisX, Qt::AlignBottom);
    m_speedSeries->attachAxis(speedAxisX);

    QValueAxis *speedAxisY = new QValueAxis;
    speedAxisY->setRange(0, 100);
    speedAxisY->setTitleText("Speed (Mbps)");
    speedChart->addAxis(speedAxisY, Qt::AlignLeft);
    m_speedSeries->attachAxis(speedAxisY);

    m_speedChartView = new QChartView(speedChart);
    m_speedChartView->setRenderHint(QPainter::Antialiasing);

    // Add chart to container
    QVBoxLayout *speedLayout = new QVBoxLayout(ui->speedChartContainer);
    speedLayout->setContentsMargins(0, 0, 0, 0);
    speedLayout->addWidget(m_speedChartView);
#else
    // Charts not available
    QLabel *label1 = new QLabel("QtCharts not available. Install Qt6 Charts module.", ui->signalChartContainer);
    QVBoxLayout *layout1 = new QVBoxLayout(ui->signalChartContainer);
    layout1->addWidget(label1);

    QLabel *label2 = new QLabel("QtCharts not available. Install Qt6 Charts module.", ui->speedChartContainer);
    QVBoxLayout *layout2 = new QVBoxLayout(ui->speedChartContainer);
    layout2->addWidget(label2);
#endif
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) {
            hide();
        } else {
            showWindow();
        }
    }
}

void MainWindow::toggleAutoRoaming() {
    bool enabled = m_toggleRoamingAction->isChecked();
    QString message = enabled ? "Auto-roaming enabled" : "Auto-roaming disabled";
    m_trayIcon->showMessage("SmartAutoRoam", message, QSystemTrayIcon::Information, 2000);
    appendLog(QString("[SYSTEM] %1").arg(message));

    // This will be connected to RoamingManager in main.cpp
    // For now, just log the state change
}

void MainWindow::showWindow() {
    show();
    raise();
    activateWindow();
}

void MainWindow::hideToTray() {
    hide();
    m_trayIcon->showMessage("SmartAutoRoam", "Running in background. Click tray icon to show.",
                            QSystemTrayIcon::Information, 2000);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hideToTray();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::updateSignalChart(const QString &ssid, int signal) {
#ifdef HAVE_QTCHARTS
    if (!m_signalChartView) return;

    // Get or create series for this SSID
    if (!m_signalSeries.contains(ssid)) {
        QLineSeries *series = new QLineSeries();
        series->setName(ssid);
        m_signalSeries[ssid] = series;

        QChart *chart = m_signalChartView->chart();
        chart->addSeries(series);

        // Attach to existing axes
        if (!chart->axes(Qt::Horizontal).isEmpty() && !chart->axes(Qt::Vertical).isEmpty()) {
            series->attachAxis(chart->axes(Qt::Horizontal).first());
            series->attachAxis(chart->axes(Qt::Vertical).first());
        }
    }

    QLineSeries *series = m_signalSeries[ssid];
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    series->append(timestamp, signal);

    // Keep only last 100 points
    if (series->count() > 100) {
        series->remove(0);
    }

    // Update X axis range to show last 5 minutes
    QChart *chart = m_signalChartView->chart();
    if (!chart->axes(Qt::Horizontal).isEmpty()) {
        QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).first());
        if (axisX) {
            axisX->setRange(QDateTime::currentDateTime().addSecs(-300), QDateTime::currentDateTime());
        }
    }
#else
    Q_UNUSED(ssid);
    Q_UNUSED(signal);
#endif
}

void MainWindow::updateSpeedChart(double speedMbps) {
#ifdef HAVE_QTCHARTS
    if (!m_speedSeries) return;

    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    m_speedSeries->append(timestamp, speedMbps);

    // Keep only last 100 points
    if (m_speedSeries->count() > 100) {
        m_speedSeries->remove(0);
    }

    // Update axes ranges
    QChart *chart = m_speedChartView->chart();

    // Update X axis to show last 5 minutes
    if (!chart->axes(Qt::Horizontal).isEmpty()) {
        QDateTimeAxis *axisX = qobject_cast<QDateTimeAxis*>(chart->axes(Qt::Horizontal).first());
        if (axisX) {
            axisX->setRange(QDateTime::currentDateTime().addSecs(-300), QDateTime::currentDateTime());
        }
    }

    // Auto-scale Y axis based on data
    if (!chart->axes(Qt::Vertical).isEmpty()) {
        QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
        if (axisY && m_speedSeries->count() > 0) {
            double maxSpeed = 0;
            for (const QPointF &point : m_speedSeries->points()) {
                maxSpeed = qMax(maxSpeed, point.y());
            }
            axisY->setRange(0, qMax(10.0, maxSpeed * 1.2)); // 20% headroom
        }
    }
#else
    Q_UNUSED(speedMbps);
#endif
}

void MainWindow::loadRecentLogs() {
    // Load last 50 log entries from database
    QSqlQuery q(Database::instance().db());
    q.exec("SELECT timestamp, event, detail FROM logs ORDER BY id DESC LIMIT 50");

    QStringList logs;
    while (q.next()) {
        QString timestamp = q.value(0).toString();
        QString event = q.value(1).toString();
        QString detail = q.value(2).toString();
        QString logMessage = QString("[%1] %2: %3").arg(timestamp, event, detail);
        logs.prepend(logMessage); // Prepend to reverse the order (oldest first)
    }

    // Display all logs
    for (const QString &log : logs) {
        ui->logView->append(log);
    }

    if (!logs.isEmpty()) {
        ui->logView->append("--- Session started ---");
    }
}
