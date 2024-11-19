#include <QApplication>

#include "logger.hpp"
#include "networkmanager.hpp"
#include "window.hpp"

int main(int argc, char *argv[]){
	QApplication app(argc, argv);

	SHIZ::Logger logger;
	logger.log("Application started.");

	SHIZ::NetworkManager networkManager(&logger);

	SHIZ::Window window(&logger, &networkManager);
	window.show();

	int result = app.exec();
	logger.log("Application exited.");
	return result;
}
