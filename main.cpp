#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("FocusForge");
    app.setOrganizationDomain("focusforge.local");
    app.setApplicationName("FocusForge");

    const QIcon appIcon(QStringLiteral(":/icons/focusforge.ico"));
    app.setWindowIcon(appIcon);

    MainWindow window;
    window.setWindowIcon(appIcon);
    window.show();

    return app.exec();
}