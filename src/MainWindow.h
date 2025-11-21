#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include "NetworkScanner.h"
#include "ConnectionManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QChartView;
class QLineSeries;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void appendLog(const QString &message);
    void updateSignalChart(const QString &ssid, int signal);
    void updateSpeedChart(double speedMbps);

private slots:
    void refreshNetworks();
    void on_connectButton_clicked();
    void on_settingsButton_clicked();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleAutoRoaming();
    void showWindow();
    void hideToTray();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    NetworkScanner m_scanner;
    ConnectionManager m_conn;
    QTimer m_timer;

    // Tray icon
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_toggleRoamingAction;
    QAction *m_showAction;
    QAction *m_quitAction;

    // Charts
    QChartView *m_signalChartView;
    QChartView *m_speedChartView;
    QMap<QString, QLineSeries*> m_signalSeries;
    QLineSeries *m_speedSeries;

    void setupTrayIcon();
    void setupCharts();
    void loadSettingsToUi();
    void saveSettingsFromUi();
    void logEvent(const QString &event, const QString &detail = QString());
    void loadRecentLogs();
};
