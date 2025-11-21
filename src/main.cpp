#include <QApplication>
#include "MainWindow.h"
#include "Database.h"
#include "ConfigManager.h"
#include "RoamingManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SmartAutoRoam");

    // Initialize DB and config
    Database::instance().initialize();
    ConfigManager::instance().load();

    // Create main window
    MainWindow w;

    // Start roaming manager and connect its signals to the main window
    RoamingManager *rm = new RoamingManager();

    // Connect log messages
    QObject::connect(rm, &RoamingManager::logMessage, &w, [&w](const QString &msg) {
        QMetaObject::invokeMethod(&w, "appendLog", Qt::QueuedConnection, Q_ARG(QString, msg));
    });

    // Connect speed measurements for chart updates
    QObject::connect(rm, &RoamingManager::speedMeasured, &w, [&w](double speed) {
        QMetaObject::invokeMethod(&w, "updateSpeedChart", Qt::QueuedConnection, Q_ARG(double, speed));
    });

    // Connect network signal updates for chart updates
    QObject::connect(rm, &RoamingManager::networkSignal, &w, [&w](const QString &ssid, int signal) {
        QMetaObject::invokeMethod(&w, "updateSignalChart", Qt::QueuedConnection,
                                  Q_ARG(QString, ssid), Q_ARG(int, signal));
    });

    rm->start();

    w.show();

    int ret = app.exec();

    // clean up
    rm->stop();
    delete rm;
    return ret;
}
