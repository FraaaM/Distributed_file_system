#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

#include "server.hpp"

FileServer::FileServer(QObject *parent) : QTcpServer(parent), clientSocket(nullptr) {}

void FileServer::startServer(qint16 port) {
	if (this->listen(QHostAddress::Any, port)) {
		qDebug() << "Server started on port" << port;
	} else {
		qDebug() << "Server failed to start!";
	}
}

void FileServer::incomingConnection(qintptr socketDescriptor) {
	clientSocket = new QTcpSocket(this);
	clientSocket->setSocketDescriptor(socketDescriptor);

	connect(clientSocket, &QTcpSocket::readyRead, this, &FileServer::handleClientConnection);
	connect(clientSocket, &QTcpSocket::disconnected, this, &FileServer::onClientDisconnected);

	qDebug() << "Client connected";
}

void FileServer::handleClientConnection() {
	QByteArray receivedData = clientSocket->readAll();

	if (!file.isOpen()) {
		QString fileName = "received_file.txt";
		file.setFileName(QDir::currentPath() + "/" + fileName);
		if (!file.open(QIODevice::WriteOnly)) {
			qDebug() << "Failed to open file for writing!";
			return;
		}
		qDebug() << "File opened for writing:" << fileName;
	}

	if (!receivedData.isEmpty()) {
		file.write(receivedData);
		qDebug() << "Writing data to file, bytes:" << receivedData.size();
	}
}

void FileServer::onClientDisconnected() {
	if (file.isOpen()) {
		file.close();
		qDebug() << "File transfer complete. File saved at" << file.fileName();
	}

	clientSocket->deleteLater();
	qDebug() << "Client disconnected";
}
