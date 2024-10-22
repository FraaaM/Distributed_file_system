#include "mainserver.hpp"

namespace SHIZ{
	MainServer::MainServer(QObject* parent): QTcpServer(parent) {}

	void MainServer::incomingConnection(qintptr socketDescriptor) {
		QTcpSocket *clientSocket = new QTcpSocket(this);
		clientSocket->setSocketDescriptor(socketDescriptor);

		connect(clientSocket, &QTcpSocket::readyRead, this, &MainServer::handleClientData);
		connect(clientSocket, &QTcpSocket::disconnected, this, &MainServer::handleClientDisconnected);

		qDebug() << "New connection established";
	}

	void MainServer::handleClientData() {
		QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
		if (!clientSocket) return;

		QByteArray data = clientSocket->readAll();
		QString request = QString::fromUtf8(data);
		qDebug() << "Received request:" << request;

		QStringList parts = request.split(":");
		if (parts.isEmpty()) return;

		if (request.startsWith("LOGIN:")) {
			processLoginRequest(clientSocket, parts);
		} else if (request.startsWith("REGISTER:")) {
			processRegistrationRequest(clientSocket, parts);
		}
	}

	void MainServer::handleClientDisconnected() {
		QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
		if (clientSocket) {
			qDebug() << "Client disconnected";
			clientSocket->deleteLater();
		}
	}

	void MainServer::processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		if (!username.isEmpty() && !password.isEmpty()) {
			clientSocket->write("SUCCESS");
		} else {
			clientSocket->write("FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		if (!username.isEmpty() && !password.isEmpty()) {
			clientSocket->write("SUCCESS");
		} else {
			clientSocket->write("FAILED");
		}
		clientSocket->flush();
	}
}
