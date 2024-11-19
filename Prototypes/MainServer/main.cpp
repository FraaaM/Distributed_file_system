#include <QApplication>

#include "mainserver.hpp"

int main(int argc, char* argv[]) {
	QCoreApplication app(argc, argv);

	SHIZ::MainServer server;
	if (!server.listen(QHostAddress::Any, 1234)) {
		qDebug() << "Server could not start!";
		return 1;
	}

	qDebug() << "Server started on port" << server.serverPort();
	return app.exec();
}
