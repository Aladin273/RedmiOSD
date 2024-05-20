#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include "RedmiOSD.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    const QString memKey = "RedmiOSDSharedMemoryKey";
    QSharedMemory sharedMemory(memKey);

    if (!sharedMemory.create(1))
    {
        QMessageBox::critical(nullptr, QObject::tr("RedmiOSD"), QObject::tr("Another instance of RedmiOSD is already running."));
        return 1;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable()) 
    {
        QMessageBox::critical(nullptr, QObject::tr("RedmiOSD"), QObject::tr("I couldn't detect any system tray on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    RedmiOSD osd;
    return app.exec();
}