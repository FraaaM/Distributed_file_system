#include <QFile>
#include <QFileInfo>

#include "networkmanager.hpp"

namespace SHIZ {
	NetworkManager::NetworkManager(QObject* parent) : QObject(parent) {
		tcpSocket = new QTcpSocket(this);
		reconnectTimer = new QTimer(this);

		connect(tcpSocket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
		connect(tcpSocket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
		connect(reconnectTimer, &QTimer::timeout, this, &NetworkManager::onReconnectToServer);

		onReconnectToServer();
	}

	NetworkManager::~NetworkManager() {
		if (tcpSocket->isOpen()) {
			tcpSocket->disconnectFromHost();
		}
	}


	bool NetworkManager::downloadFile(const QString& fileName) {
		QDataStream out(tcpSocket);
		out << QString("DOWNLOAD") << fileName;
		tcpSocket->flush();

		if (!tcpSocket->waitForReadyRead(3000)) {
			qDebug() << "Server did not respond in time.";
			return false;
		}

		QDataStream in(tcpSocket);
		QString response;
		qint64 fileSize = 0;

		in >> response;
		if (response != "DOWNLOAD_READY") {
			qDebug() << "Server is not ready to send the file.";
			return false;
		}

		in >> fileSize;
		qDebug() << "File size:" << fileSize;

		out << QString("READY_TO_RECEIVE");
		tcpSocket->flush();

		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly)) {
			qDebug() << "Cannot open file for writing.";
			return false;
		}

		qint64 totalReceived = 0;
		const qint64 chunkSize = 4096;
		QByteArray chunk;

		while (totalReceived < fileSize) {
			if (!tcpSocket->waitForReadyRead(3000)) {
				qDebug() << "No response from server while downloading.";
				file.close();
				return false;
			}

			in >> chunk;
			file.write(chunk);
			totalReceived += chunk.size();

			out << QString("CHUNK_RECEIVED");
			tcpSocket->flush();
		}

		file.close();
		qDebug() << "File downloaded successfully:" << fileName;
		return true;
	}

	QStringList NetworkManager::requestFileList(){
		QDataStream out(tcpSocket);
		out << QString("GET_FILES");
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			QStringList fileList;

			in >> response;
			if (response == "FILES_LIST") {
				in >> fileList;
				return fileList;
			}
		}
		return QStringList();
	}

	bool NetworkManager::sendLoginRequest(const QString& login, const QString& password) {
		QDataStream out(tcpSocket);
		out << QString("LOGIN") << login << password;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;
			return response == "SUCCESS";
		}

		return false;
	}

	bool NetworkManager::sendRegistrationRequest(const QString& login, const QString& password) {
		QDataStream out(tcpSocket);
		out << QString("REGISTER") << login << password;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;
			return response == "SUCCESS";
		}

		return false;
	}

	bool NetworkManager::uploadFile(const QString& filePath, const QString& owner) {
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			qDebug() << "Cannot open file for reading.";
			return false;
		}

		QString fileName = QFileInfo(filePath).fileName();
		qint64 fileSize = file.size();
		qDebug() << "Uploading file:" << fileName << "Owner:" << owner << "Size:" << fileSize;

		QDataStream out(tcpSocket);
		out << QString("UPLOAD") << fileName << owner << fileSize;
		tcpSocket->flush();

		if (!tcpSocket->waitForReadyRead(3000)) {
			qDebug() << "Server did not respond in time.";
			return false;
		}

		QString response;
		QDataStream in(tcpSocket);
		in >> response;

		if (response != "READY_FOR_DATA") {
			qDebug() << "Server is not ready for data.";
			return false;
		}

		const qint64 chunkSize = 4096;
		while (!file.atEnd()) {
			QByteArray buffer = file.read(chunkSize);
			out << buffer;
			tcpSocket->flush();

			if (!tcpSocket->waitForReadyRead(3000)) {
				qDebug() << "No response from server after sending chunk.";
				return false;
			}

			in >> response;
			if (response != "CHUNK_RECEIVED") {
				qDebug() << "Server did not acknowledge chunk.";
				return false;
			}
		}

		out << QByteArray();
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			in >> response;
			qDebug() << response;
			return response == "UPLOAD_SUCCESS";
		}

		return false;
	}


	void NetworkManager::onConnected() {
		qDebug() << "Connected to server.";
		reconnectTimer->stop();
	}

	void NetworkManager::onDisconnected() {
		qDebug() << "Disconnected from server. Reconnecting...";
		reconnectTimer->start(3000);
	}

	void NetworkManager::onReconnectToServer() {
		if (tcpSocket->state() == QTcpSocket::UnconnectedState) {
			tcpSocket->connectToHost("127.0.0.1", 1234);
		}
	}
}
