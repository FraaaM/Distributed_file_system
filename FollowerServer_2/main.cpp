#include <QApplication>

#include "logger.hpp"
#include "mainwindow.hpp"

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	SHIZ::Logger logger;
	logger.log("Application started.");

	SHIZ::MainWindow mainWindow(&logger);
	mainWindow.show();

	int result = app.exec();
	logger.log("Application exited.");
	return result;
}
