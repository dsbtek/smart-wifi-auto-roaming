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

    // Start roaming manager
    RoamingManager *rm = new RoamingManager();
    rm->start();

    MainWindow w;
    w.show();

    int ret = app.exec();

    // clean up
    rm->stop();
    delete rm;
    return ret;
}
