#pragma once
#include <QMainWindow>
#include <QTimer>
#include "NetworkScanner.h"
#include "ConnectionManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshNetworks();
    void on_connectButton_clicked();
    void on_settingsButton_clicked();

private:
    Ui::MainWindow *ui;
    NetworkScanner m_scanner;
    ConnectionManager m_conn;
    QTimer m_timer;
    void loadSettingsToUi();
    void saveSettingsFromUi();
    void logEvent(const QString &event, const QString &detail = QString());
};
