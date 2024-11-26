#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

#include "mainserver.hpp"
#include "servermacros.hpp"

namespace SHIZ {
	MainServer::MainServer(Logger* logger, QObject* parent)
		: logger(logger), QTcpServer(parent)
	{
		dataBase = QSqlDatabase::addDatabase(DATABASE_TYPE);
		dataBase.setDatabaseName(DATABASE_NAME);

		if (!dataBase.open()) {
			logger->log("Failed to connect to database.");
			return;
		}

		QSqlQuery query;
		query.exec("CREATE TABLE IF NOT EXISTS " TABLE_USERS " ("
				   FIELD_USER_ID " INTEGER PRIMARY KEY AUTOINCREMENT, "
				   FIELD_USER_USERNAME " TEXT UNIQUE, "
				   FIELD_USER_PASSWORD " TEXT)");
		query.exec("CREATE TABLE IF NOT EXISTS " TABLE_FILES " ("
				   FIELD_FILE_ID " INTEGER PRIMARY KEY AUTOINCREMENT, "
				   FIELD_FILE_FILENAME " TEXT, "
				   FIELD_FILE_OWNER " TEXT, "
				   FIELD_FILE_SIZE " INTEGER, "
				   FIELD_FILE_UPLOAD_DATE " TEXT, "
				   FIELD_FILE_PATH " TEXT)");
		query.exec("CREATE TABLE IF NOT EXISTS " TABLE_FILE_REPLICAS " ("
				   FIELD_REPLICA_ID " INTEGER PRIMARY KEY AUTOINCREMENT, "
				   FIELD_FILE_FILENAME " TEXT, "
				   FIELD_REPLICA_ADDRESS " TEXT, "
				   FIELD_REPLICA_PORT " INTEGER)");

		QDir dir(QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY);
		if (!dir.exists()) {
			dir.mkpath(".");
		}

		logger->log("Server initialized successfully.");
	}

	MainServer::~MainServer() {
		closeServer();
	}


	void MainServer::closeServer() {
		close();
		for (QTcpSocket* client : activeClients) {
			client->disconnectFromHost();
			if (client->state() != QAbstractSocket::UnconnectedState) {
				client->waitForDisconnected();
			}
			client->deleteLater();
		}
		activeClients.clear();
		logger->log("All clients disconnected and server stopped.");
	}

	bool MainServer::connectToHost(const QString& host, quint16 port) {
		QTcpSocket* replicaSocket = new QTcpSocket(this);
		replicaSocket->connectToHost(host, port);

		if (replicaSocket->waitForConnected(3000)) {
			QDataStream out(replicaSocket);
			out << QString(MAIN_SERVER);
			replicaSocket->flush();

			replicaSockets.append(replicaSocket);
			logger->log("Connected to replica at " + host + ":" + QString::number(port));

			connect(replicaSocket, &QTcpSocket::connected, this, &MainServer::onReplicaConnected);
			connect(replicaSocket, &QTcpSocket::disconnected, this, &MainServer::onReplicaDisconnected);
			return true;
		} else {
			logger->log("Failed to connect to replica at " + host + ":" + QString::number(port));
			replicaSocket->deleteLater();
			return false;
		}
	}

	void MainServer::disconnectFromHost(const QString& host, quint16 port) {
		for (auto socket : replicaSockets) {
			if (socket->peerAddress().toString() == host && socket->peerPort() == port) {
				disconnect(socket, nullptr, this, nullptr);
				socket->disconnectFromHost();
				if (socket->state() == QAbstractSocket::UnconnectedState || socket->waitForDisconnected(3000)) {
					replicaSockets.removeOne(socket);
					logger->log("Disconnected from replica at " + host + ":" + QString::number(port));
					socket->deleteLater();
					return;
				}
			}
		}
		logger->log("No active replica connection found at " + host + ":" + QString::number(port));
	}


	void MainServer::incomingConnection(qintptr socketDescriptor) {
		QTcpSocket* newSocket = new QTcpSocket(this);
		newSocket->setSocketDescriptor(socketDescriptor);

		if (newSocket->waitForReadyRead(3000)) {
			QDataStream in(newSocket);
			QString initialMessage;
			in >> initialMessage;

			if (initialMessage == CLIENT) {
				activeClients.append(newSocket);
				connect(newSocket, &QTcpSocket::readyRead, this, &MainServer::handleClientData);
				connect(newSocket, &QTcpSocket::disconnected, this, &MainServer::handleClientDisconnected);
				logger->log("New client connection established.");
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


	bool MainServer::distributeFileToReplicas(const QString& fileName, const QByteArray& fileData) {
		bool atLeastOneSuccess = false;
		for (QTcpSocket* replicaSocket : replicaSockets) {
			QDataStream out(replicaSocket);
			out << QString(COMMAND_REPLICA_UPLOAD) << fileName << fileData.size();
			replicaSocket->flush();

			if (!replicaSocket->waitForReadyRead(3000)) {
				logger->log("No response from replica for upload.");
				continue;
			}

			QDataStream in(replicaSocket);
			QString response;
			in >> response;

			if (response != RESPONSE_READY_FOR_DATA) {
				logger->log("Replica is not ready for upload: " + response);
				continue;
			}

			const qint64 chunkSize = CHUNK_SIZE;
			qint64 bytesSent = 0;
			bool uploadSuccessful = true;

			while (bytesSent < fileData.size()) {
				QByteArray chunk = fileData.mid(bytesSent, chunkSize);
				out << chunk;
				replicaSocket->flush();
				bytesSent += chunk.size();

				if (!replicaSocket->waitForReadyRead(3000)) {
					uploadSuccessful = false;
					logger->log("No response from replica during upload.");
					break;
				}

				in >> response;
				if (response != RESPONSE_CHUNK_RECEIVED) {
					uploadSuccessful = false;
					logger->log("Replica failed to acknowledge chunk.");
					break;
				}
			}

			if (uploadSuccessful && bytesSent >= fileData.size()) {
				QString replicaAddress = replicaSocket->peerAddress().toString();
				quint16 replicaPort = replicaSocket->peerPort();

				QSqlQuery query;
				query.prepare("INSERT INTO " TABLE_FILE_REPLICAS " (" FIELD_FILE_FILENAME ", " FIELD_REPLICA_ADDRESS ", " FIELD_REPLICA_PORT ") VALUES (?, ?, ?)");
				query.addBindValue(fileName);
				query.addBindValue(replicaAddress);
				query.addBindValue(replicaPort);

				if (!query.exec()) {
					logger->log("Failed to save replica file info: " + query.lastError().text());
				} else {
					atLeastOneSuccess = true;
					logger->log("File info saved for replica: " + replicaAddress + ":" + QString::number(replicaPort));
				}
			} else {
				logger->log("File upload to replica failed: " + replicaSocket->peerAddress().toString());
			}
		}
		return atLeastOneSuccess;
	}

	void MainServer::processDeleteFileRequest(QTcpSocket* clientSocket, const QString& fileName) {
		QSqlQuery query;
		query.prepare("SELECT " FIELD_FILE_PATH " FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
		query.bindValue(":filename", fileName);

		QDataStream out(clientSocket);
		if (query.exec() && query.next()) {
			QString filePath = query.value(FIELD_FILE_PATH).toString();

			if (QFile::exists(filePath) && !QFile::remove(filePath)) {
				logger->log("Failed to delete file from disk:");
				out << QString(RESPONSE_DELETE_FAILED);
				clientSocket->flush();
				return;
			}

			query.prepare("DELETE FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
			query.bindValue(":filename", fileName);

			if (query.exec()) {
				out << QString(RESPONSE_DELETE_SUCCESS);
			} else {
				logger->log("Failed to delete file from database:" + query.lastError().text());
				out << QString(RESPONSE_DELETE_FAILED);
			}
		} else {
			logger->log("File not found in database or query failed:" + query.lastError().text());
			out << QString(RESPONSE_DELETE_FAILED);
		}
		clientSocket->flush();
	}

	void MainServer::processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName) {
		QSqlQuery query;
		query.prepare("SELECT " FIELD_FILE_PATH ", " FIELD_FILE_SIZE " FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
		query.bindValue(":filename", fileName);

		QDataStream out(clientSocket);
		if (!query.exec() || !query.next()) {
			out << QString(RESPONSE_DOWNLOAD_FAILED);
			clientSocket->flush();
			logger->log("File not found in database or query failed.");
			return;
		}

		QString filePath = query.value(FIELD_FILE_PATH).toString();
		qint64 fileSize = query.value(FIELD_FILE_SIZE).toLongLong();

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			out << QString(RESPONSE_DOWNLOAD_FAILED);
			clientSocket->flush();
			logger->log("Failed to open file for reading:" + file.errorString());
			return;
		}

		QByteArray fileData = file.readAll();
		file.close();

		out << QString(RESPONSE_DOWNLOAD_READY) << fileSize;
		clientSocket->flush();

		if (!clientSocket->waitForReadyRead(3000)) {
			logger->log("No response from client for download request.");
			return;
		}

		QDataStream in(clientSocket);
		QString clientResponse;
		in >> clientResponse;

		if (clientResponse != RESPONSE_READY_FOR_DATA) {
			logger->log("Client is not ready to receive file.");
			return;
		}

		const qint64 chunkSize = CHUNK_SIZE;
		qint64 bytesSent = 0;

		while (bytesSent < fileSize) {
			QByteArray chunk = fileData.mid(bytesSent, chunkSize);
			out << chunk;
			clientSocket->flush();
			bytesSent += chunk.size();

			if (!clientSocket->waitForReadyRead(3000)) {
				logger->log("No response from client after sending chunk.");
				return;
			}

			in >> clientResponse;
			if (clientResponse != RESPONSE_CHUNK_RECEIVED) {
				logger->log("Client did not acknowledge chunk.");
				return;
			}
		}
		logger->log("File sent successfully: " + fileName);
	}

	void MainServer::processFileListRequest(QTcpSocket* clientSocket) {
		QSqlQuery query("SELECT " FIELD_FILE_FILENAME ", " FIELD_FILE_OWNER ", " FIELD_FILE_SIZE ", " FIELD_FILE_UPLOAD_DATE " FROM " TABLE_FILES);
		QStringList fileList;

		while (query.next()) {
			QString fileInfo = query.value(0).toString() + "|" + query.value(1).toString() + "|" +
							   query.value(2).toString() + "|" + query.value(3).toString();
			fileList << fileInfo;
		}

		QDataStream out(clientSocket);
		out << QString(RESPONSE_FILES_LIST) << fileList;
		clientSocket->flush();
	}

	void MainServer::processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		QDataStream out(clientSocket);

		QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

		QSqlQuery query;
		query.prepare("SELECT " FIELD_USER_PASSWORD " FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
		query.bindValue(":username", username);

		if (query.exec() && query.next()) {
			QString storedPassword = query.value(0).toString();
			out << (storedPassword == hashedPassword ? QString(RESPONSE_LOGIN_SUCCESS) : QString(RESPONSE_LOGIN_FAILED));
		} else {
			out << QString(RESPONSE_LOGIN_FAILED);
			logger->log("Login failed: " + query.lastError().text());
		}
		clientSocket->flush();
	}

	void MainServer::processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		QDataStream out(clientSocket);

		QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

		QSqlQuery checkQuery;
		checkQuery.prepare("SELECT " FIELD_USER_ID " FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
		checkQuery.bindValue(":username", username);

		if (checkQuery.exec() && checkQuery.next()) {
			out << QString(RESPONSE_REGISTER_USER_EXISTS);
			logger->log("Registration failed: user already exists.");
		} else {
			QSqlQuery insertQuery;
			insertQuery.prepare("INSERT INTO " TABLE_USERS " (" FIELD_USER_USERNAME ", " FIELD_USER_PASSWORD ") VALUES (?, ?)");
			insertQuery.addBindValue(username);
			insertQuery.addBindValue(hashedPassword);

			out << (insertQuery.exec() ? QString(RESPONSE_REGISTER_SUCCESS) : QString(RESPONSE_REGISTER_FAILED));
			if (insertQuery.lastError().isValid()) qDebug() << "Registration failed: " << insertQuery.lastError();
		}
		clientSocket->flush();
	}

	void MainServer::processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize) {
		QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
		QString filePath = QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY "/" + fileName;

		QSqlQuery checkQuery;
		checkQuery.prepare("SELECT " FIELD_FILE_PATH " FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
		checkQuery.bindValue(":filename", fileName);

		if (checkQuery.exec() && checkQuery.next()) {
			QString existingFilePath = checkQuery.value(FIELD_FILE_PATH).toString();
			if (QFile::exists(existingFilePath)) {
				QFile::remove(existingFilePath);
			}

			QSqlQuery deleteQuery;
			deleteQuery.prepare("DELETE FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
			deleteQuery.bindValue(":filename", fileName);
			if (!deleteQuery.exec()) {
				logger->log("Error deleting an existing file:" + deleteQuery.lastError().text());
				QDataStream out(clientSocket);
				out << QString(RESPONSE_UPLOAD_FAILED);
				clientSocket->flush();
				return;
			}
		}

		QByteArray fileData;
		qint64 totalReceived = 0;

		QDataStream out(clientSocket);
		out << QString(RESPONSE_READY_FOR_DATA);
		clientSocket->flush();

		while (totalReceived < fileSize) {
			if (clientSocket->waitForReadyRead(3000)) {
				QDataStream in(clientSocket);
				QByteArray chunk;
				in >> chunk;
				if (chunk.isEmpty()) {
					logger->log("Received empty chunk, ending upload.");
					out << QString(RESPONSE_UPLOAD_FAILED);
					return;
				}
				fileData.append(chunk);
				totalReceived += chunk.size();

				out << QString(RESPONSE_CHUNK_RECEIVED);
				clientSocket->flush();
			} else {
				logger->log("No data received from client.");
				out << QString(RESPONSE_UPLOAD_FAILED);
				return;
			}
		}

		QFile file(filePath);
		if (file.open(QIODevice::WriteOnly)) {
			file.write(fileData);
			file.close();
		} else {
			logger->log("Failed to write file to disk:" + file.errorString());
			out << QString(RESPONSE_UPLOAD_FAILED);
			clientSocket->flush();
			return;
		}

		QSqlQuery query;
		query.prepare("INSERT INTO " TABLE_FILES " (" FIELD_FILE_FILENAME ", " FIELD_FILE_OWNER ", " FIELD_FILE_SIZE ", " FIELD_FILE_UPLOAD_DATE ", " FIELD_FILE_PATH ") VALUES (?, ?, ?, ?, ?)");
		query.addBindValue(fileName);
		query.addBindValue(owner);
		query.addBindValue(fileSize);
		query.addBindValue(uploadDate);
		query.addBindValue(filePath);

		if (!query.exec()) {
			out << QString(RESPONSE_UPLOAD_FAILED);
			clientSocket->flush();
			return;
		}

		out << (distributeFileToReplicas(fileName, fileData) ? QString(RESPONSE_UPLOAD_SUCCESS) : QString(RESPONSE_UPLOAD_FAILED));
		clientSocket->flush();
		logger->log("File received successfully: " + fileName);
	}



	void MainServer::handleClientData() {
		QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
		if (!clientSocket) return;

		QDataStream in(clientSocket);
		QString command;
		in >> command;

		if (command == COMMAND_DOWNLOAD) {
			QString fileName;
			in >> fileName;
			processDownloadRequest(clientSocket, fileName);
		} else if (command == COMMAND_GET_FILES) {
			processFileListRequest(clientSocket);
		} else if (command == COMMAND_LOGIN) {
			QStringList parts;
			QString login, password;
			in >> login >> password;
			parts << COMMAND_LOGIN << login << password;
			processLoginRequest(clientSocket, parts);
		} else if (command == COMMAND_REGISTER) {
			QStringList parts;
			QString login, password;
			in >> login >> password;
			parts << COMMAND_REGISTER << login << password;
			processRegistrationRequest(clientSocket, parts);
		} else if (command == COMMAND_UPLOAD) {
			QString fileName, owner;
			qint64 fileSize;
			in >> fileName >> owner >> fileSize;
			processUploadRequest(clientSocket, fileName, owner, fileSize);
		} else if (command == COMMAND_DELETE) {
			QString fileName;
			in >> fileName;
			processDeleteFileRequest(clientSocket, fileName);
		}
	}

	void MainServer::handleClientDisconnected() {
		QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
		if (clientSocket) {
			activeClients.removeAll(clientSocket);
			clientSocket->deleteLater();
			logger->log("Client disconnected");
		}
	}

	void MainServer::onReplicaConnected() {
		logger->log("Replica connection established.");
		emit statusMessage("Replica connected.");
	}

	void MainServer::onReplicaDisconnected() {
		QTcpSocket* replicaSocket = qobject_cast<QTcpSocket*>(sender());
		if (replicaSocket) {
			QString replicaAddress = replicaSocket->peerAddress().toString() + ":" + QString::number(replicaSocket->peerPort());
			replicaSockets.removeOne(replicaSocket);
			logger->log("Replica disconnected: " + replicaAddress);
			replicaSocket->deleteLater();
			emit replicaDisconnected(replicaAddress);
			emit statusMessage("Replica disconnected.");
		}
	}
}
