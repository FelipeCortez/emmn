#include "mainwindow.h"
#include <QApplication>
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[]) {
    qRegisterMetaTypeStreamOperators<Tracker>("Tracker");

    QApplication app(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator qtBaseTranslator;
    qtBaseTranslator.load("qtbase_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtBaseTranslator);

    MainWindow window;
    window.show();

    return app.exec();
}
