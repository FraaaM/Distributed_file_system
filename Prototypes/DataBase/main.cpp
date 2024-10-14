#include <QApplication>

#include "filemanager.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	FileManager client;
	client.show();

	return app.exec();
}
