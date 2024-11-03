#include <QFile>
#include <QFileInfo>
#include <QDebug>

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

	void NetworkManager::setHost(const QString& initialhost){
		NetworkManager::host = initialhost;
	}

	QString NetworkManager::getHost() {
		return NetworkManager::host;
	}

	bool NetworkManager::downloadFile(const QString& fileName){
		QString downloadRequest = "DOWNLOAD:" + fileName;
		tcpSocket->write(downloadRequest.toUtf8());
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QString response = QString::fromUtf8(tcpSocket->readAll());
			if (response.startsWith("DOWNLOAD:")) {
				QStringList parts = response.split(":");
				if (parts.size() >= 3) {
					QByteArray fileData = parts[2].toUtf8();

					QFile file(fileName);
					if (file.open(QIODevice::WriteOnly)) {
						file.write(fileData);
						file.close();
						return true;
					}
				}
			}
		}
		return false;
	}

	QStringList NetworkManager::requestFileList(){
		tcpSocket->write("GET_FILES");
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QString response = QString::fromUtf8(tcpSocket->readAll());
			if (response.startsWith("FILES_LIST:")) {
				QString fileData = response.mid(11);
				return fileData.split(";");
			}
		}
		return QStringList();
	}

	bool NetworkManager::sendConnectionRequest (const QString& host) {
		if (tcpSocket->state() == QTcpSocket::UnconnectedState) {
			tcpSocket->connectToHost(host, 1234);
		}

		if (tcpSocket->waitForConnected(3000)) {
			return true;
		}

		return false;
	}

	bool NetworkManager::sendLoginRequest(const QString& login, const QString& password) {
		if (tcpSocket->state() == QTcpSocket::UnconnectedState) {
			qDebug() << "Failed to connect to server";
		}

		if (tcpSocket->waitForConnected(3000)) {
			QString loginData = "LOGIN:" + login + ":" + password;
			tcpSocket->write(loginData.toUtf8());
			tcpSocket->flush();

			if (tcpSocket->waitForReadyRead(3000)) {
				QString response = tcpSocket->readAll();
				return response == "SUCCESS";
			}
		}

		return false;
	}

	bool NetworkManager::sendRegistrationRequest(const QString& login, const QString& password) {
		if (tcpSocket->state() == QTcpSocket::UnconnectedState) {
			qDebug() << "Failed to connect to server";
		}

		if (tcpSocket->waitForConnected(3000)) {
			QString registrationData = "REGISTER:" + login + ":" + password;
			tcpSocket->write(registrationData.toUtf8());
			tcpSocket->flush();

			if (tcpSocket->waitForReadyRead(3000)) {
				QString response = tcpSocket->readAll();
				return response == "SUCCESS";
			}
		}

		return false;
	}

	bool NetworkManager::uploadFile(const QString& filePath, const QString& owner){
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			return false;
		}

		QByteArray fileData = file.readAll();
		QString fileName = QFileInfo(filePath).fileName();
		qint64 fileSize = file.size();

		QString uploadRequest = "UPLOAD:" + fileName + ":" + owner + ":" + QString::fromUtf8(fileData) + ":" + QString::number(fileSize);
		tcpSocket->write(uploadRequest.toUtf8());
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QString response = QString::fromUtf8(tcpSocket->readAll());
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
		if (tcpSocket->state() == QTcpSocket::UnconnectedState || !NetworkManager::host.isEmpty()) {
			tcpSocket->connectToHost(NetworkManager::host, 1234);
		}
	}
}
