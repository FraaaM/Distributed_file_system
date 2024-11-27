#include <QCoreApplication>
#include <QDataStream>
#include <QDir>

#include "replicamacros.hpp"
#include "replicaserver.hpp"

namespace SHIZ {
	ReplicaServer::ReplicaServer(Logger* logger, QObject* parent)
		: mainServer(nullptr), logger(logger), QTcpServer(parent)
	{
		QDir dir(QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY);
		if (!dir.exists()) {
			dir.mkpath(".");
		}

		logger->log("Server initialized successfully.");
	}

	ReplicaServer::~ReplicaServer(){
		closeServer();
	}


	void ReplicaServer::closeServer() {
		close();
		if (mainServer) {
			disconnect(mainServer, nullptr, this, nullptr);
			mainServer->disconnectFromHost();
			if (mainServer->state() != QAbstractSocket::UnconnectedState) {
				mainServer->waitForDisconnected();
			}
			mainServer->deleteLater();
			mainServer = nullptr;
			logger->log("Disconnected from Main Server.");
		}
		logger->log("Replica server stopped.");
	}

	void ReplicaServer::incomingConnection(qintptr socketDescriptor) {
		if (mainServer) {
			logger->log("Main Server connection already exists. Rejecting new connection.");
			QTcpSocket tempSocket;
			tempSocket.setSocketDescriptor(socketDescriptor);
			tempSocket.disconnectFromHost();
			return;
		}

		QTcpSocket* newSocket = new QTcpSocket(this);
		newSocket->setSocketDescriptor(socketDescriptor);

		if (newSocket->waitForReadyRead(3000)) {
			QDataStream in(newSocket);
			QString initialMessage;
			in >> initialMessage;

			if (initialMessage == MAIN_SERVER) {
				mainServer = newSocket;
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

		if (command == COMMAND_REPLICA_UPLOAD) {
			QString fileName;
			qint64 fileSize;
			in >> fileName >> fileSize;

			QString filePath = QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY "/" + fileName;

			QDataStream out(mainServerSocket);
			out << QString(RESPONSE_READY_FOR_DATA);
			mainServerSocket->flush();

			QByteArray fileData;
			qint64 totalReceived = 0;

			while (totalReceived < fileSize) {
				if (mainServerSocket->waitForReadyRead(3000)) {
					QByteArray chunk;
					in >> chunk;
					fileData.append(chunk);
					totalReceived += chunk.size();

					out << QString(RESPONSE_CHUNK_RECEIVED);
					mainServerSocket->flush();
				} else {
					logger->log("Timeout while receiving file: " + fileName);
					return;
				}
			}

			QFile file(filePath);
			if (file.open(QIODevice::WriteOnly)) {
				file.write(fileData);
				file.close();
				logger->log("File saved successfully: " + fileName);
			} else {
				logger->log("Failed to save file: " + file.errorString());
				return;
			}
		} else if (command == COMMAND_REPLICA_DELETE) {
			QString fileName;
			in >> fileName;

			QString filePath = QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY "/" + fileName;
			if (QFile::exists(filePath) && QFile::remove(filePath)) {
				logger->log("File deleted successfully: " + fileName);
			} else {
				logger->log("Failed to delete file: " + filePath);
			}
		} else if (command == COMMAND_REPLICA_DOWNLOAD) {
			QString fileName;
			in >> fileName;

			QString filePath = QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY "/" + fileName;

			QDataStream out(mainServerSocket);
			if (!QFile::exists(filePath)) {
				logger->log("File not found for download: " + filePath);
				out << QString(RESPONSE_DOWNLOAD_FAILED);
				return;
			}

			QFile file(filePath);
			if (!file.open(QIODevice::ReadOnly)) {
				logger->log("Failed to open file for download: " + file.errorString());
				out << QString(RESPONSE_DOWNLOAD_FAILED);
				return;
			}

			QByteArray fileData = file.readAll();
			file.close();

			out << QString(RESPONSE_DOWNLOAD_READY) << fileData.size();
			mainServerSocket->flush();

			if (!mainServerSocket->waitForReadyRead(3000)) {
				logger->log("Main server not ready to receive file.");
				return;
			}

			QDataStream in(mainServerSocket);
			QString serverResponse;
			in >> serverResponse;

			if (serverResponse != RESPONSE_READY_FOR_DATA) {
				logger->log("Main server rejected file transfer.");
				return;
			}

			const qint64 chunkSize = CHUNK_SIZE;
			qint64 bytesSent = 0;

			while (bytesSent < fileData.size()) {
				QByteArray chunk = fileData.mid(bytesSent, chunkSize);
				out << chunk;
				mainServerSocket->flush();
				bytesSent += chunk.size();

				if (!mainServerSocket->waitForReadyRead(3000)) {
					logger->log("Timeout while sending file to main server.");
					return;
				}

				in >> serverResponse;
				if (serverResponse != RESPONSE_CHUNK_RECEIVED) {
					logger->log("Main server did not acknowledge chunk.");
					return;
				}
			}

			logger->log("File sent successfully to main server: " + fileName);
		}
	}

	void ReplicaServer::handleMainServerDisconnected() {
		if (mainServer) {
			mainServer->deleteLater();
			mainServer = nullptr;
			logger->log("Main Server disconnected.");
		}
	}
}
