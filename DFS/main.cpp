#include <QApplication>
#include <QThread>

#include "logger.hpp"
#include "networkmanager.hpp"
#include "window.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	SHIZ::Logger logger;
	logger.log("Application started.");

	SHIZ::NetworkManager* networkManager = new SHIZ::NetworkManager(&logger);

	QThread* networkThread = new QThread;

	networkManager->moveToThread(networkThread);
	QObject::connect(networkThread, &QThread::finished, networkManager, &QObject::deleteLater);
	networkThread->start();

	SHIZ::Window window(&logger, networkManager);
	window.show();

	int result = app.exec();
	logger.log("Application exited.");

	networkThread->quit();
	networkThread->wait();

	return result;
}
