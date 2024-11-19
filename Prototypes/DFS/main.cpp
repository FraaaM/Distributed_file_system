#include <QApplication>

#include "networkmanager.hpp"
#include "window.hpp"

int main(int argc, char *argv[]){
	QApplication app(argc, argv);

	SHIZ::NetworkManager networkManager;
	SHIZ::Window window(&networkManager);

	window.show();
	return app.exec();
}
