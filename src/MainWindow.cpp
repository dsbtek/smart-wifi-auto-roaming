#include "MainWindow.h"
#include "ui_MainWindow.h" // generated from MainWindow.ui by uic
#include "Database.h"
#include "ConfigManager.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Database::instance().initialize();
    ConfigManager::instance().load();
    loadSettingsToUi();

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::refreshNetworks);
    int interval = ConfigManager::instance().get("scan_interval", 30).toInt();
    m_timer.start(interval * 1000);
    refreshNetworks();
}

MainWindow::~MainWindow() {
    delete ui;
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
    }
}

void MainWindow::on_connectButton_clicked() {
    auto sel = ui->networkTable->selectedItems();
    if (sel.isEmpty()) return;
    QString ssid = sel.first()->text();
    bool ok = m_conn.connectTo(ssid);
    if (ok) {
        logEvent("CONNECT", ssid);
        QMessageBox::information(this, "Connected", QString("Connected to %1").arg(ssid));
    } else {
        logEvent("CONNECT_FAILED", ssid);
        QMessageBox::warning(this, "Failed", QString("Failed to connect to %1").arg(ssid));
    }
}

void MainWindow::on_settingsButton_clicked() {
    saveSettingsFromUi();
    QMessageBox::information(this, "Settings", "Settings saved.");
}

void MainWindow::logEvent(const QString &event, const QString &detail) {
    QSqlQuery q(Database::instance().db());
    q.prepare("INSERT INTO logs (event, detail) VALUES (:e, :d)");
    q.bindValue(":e", event);
    q.bindValue(":d", detail);
    q.exec();
}
