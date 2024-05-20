#include <QApplication>
#include <QMessageBox>

#include "RedmiOSD.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) 
    {
        QMessageBox::critical(nullptr, QObject::tr("RedmiOSD"), QObject::tr("I couldn't detect any system tray on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    RedmiOSD osd;
    return app.exec();
}