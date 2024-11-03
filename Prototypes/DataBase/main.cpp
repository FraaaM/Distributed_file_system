#include <QApplication>


#include "mainwindow.hpp"
#include "databasemanager.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
