#include <QDebug>
#include <QCoreApplication>

#include "client.hpp"

FileClient::FileClient(QObject *parent) : QObject(parent), socket(new QTcpSocket(this)), bytesSent(0) {
	connect(socket, &QTcpSocket::connected, this, &FileClient::onConnected);
	connect(socket, &QTcpSocket::bytesWritten, this, &FileClient::onBytesWritten);
}

void FileClient::connectToServer(const QString &host, qint16 port) {
	socket->connectToHost(host, port);
}

void FileClient::uploadFile(const QString &filePath) {
	qDebug() << "Trying to open file:" << filePath;

	file.setFileName(filePath);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "Failed to open file!";
		return;
	}

	connectToServer("127.0.0.1", 12345);
}

void FileClient::onConnected() {
	qDebug() << "Connected to server. Starting file upload.";
	QByteArray fileData = file.readAll();
	socket->write(fileData);
}

void FileClient::onBytesWritten(qint64 bytes) {
	bytesSent += bytes;
	qDebug() << "Sent" << bytesSent << "bytes of file";

	if (bytesSent >= file.size()) {
		qDebug() << "File upload complete.";
		file.close();
		socket->disconnectFromHost();
		qDebug() << "Disconected.";
	}
}
