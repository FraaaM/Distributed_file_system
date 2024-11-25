#include <QCoreApplication>
#include <QDataStream>
#include <QDir>

#include "replicamacros.hpp"
#include "replicaserver.hpp"

namespace SHIZ {
	ReplicaServer::ReplicaServer(Logger* logger, QObject* parent)
		: logger(logger), QTcpServer(parent)
	{
		QDir dir(QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY);
		if (!dir.exists()) {
			dir.mkpath(".");
		}

		logger->log("Server initialized successfully.");
	}


	void ReplicaServer::incomingConnection(qintptr socketDescriptor) {
		QTcpSocket* newSocket = new QTcpSocket(this);
		newSocket->setSocketDescriptor(socketDescriptor);

		if (newSocket->waitForReadyRead(3000)) {
			QDataStream in(newSocket);
			QString initialMessage;
			in >> initialMessage;

			if (initialMessage == "MAIN_SERVER") {
				connect(newSocket, &QTcpSocket::readyRead, this, &ReplicaServer::handleMainServerData);
				connect(newSocket, &QTcpSocket::disconnected, this, &ReplicaServer::handleMainServerDisconnected);
				logger->log("Main Server connection established.");
				return;
			} else {
				logger->log("Unknown connection type. Closing socket.");
				newSocket->close();
				newSocket->deleteLater();
				return;
			}
		} else {
			logger->log("No data received to identify connection type. Closing socket.");
			newSocket->close();
			newSocket->deleteLater();
		}
	}


	void ReplicaServer::handleMainServerData() {
		QTcpSocket* mainServerSocket = qobject_cast<QTcpSocket*>(sender());
		if (!mainServerSocket) return;

		QDataStream in(mainServerSocket);
		QString command;
		in >> command;
	}

	void ReplicaServer::handleMainServerDisconnected() {
		QTcpSocket* mainServerSocket = qobject_cast<QTcpSocket*>(sender());
		if (mainServerSocket) {
			mainServerSocket->deleteLater();
			logger->log("Main Server disconnected");
		}
	}
}
