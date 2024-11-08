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
	MainServer::MainServer(QObject* parent) : QTcpServer(parent) {
		dataBase = QSqlDatabase::addDatabase(DATABASE_TYPE);
		dataBase.setDatabaseName(DATABASE_NAME);

		if (!dataBase.open()) {
			qDebug() << "Failed to connect to database!";
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

		QDir dir(QCoreApplication::applicationDirPath() + UPLOAD_DIRECTORY);
		if (!dir.exists()) {
			dir.mkpath(".");
		}
	}


	void MainServer::incomingConnection(qintptr socketDescriptor) {
		QTcpSocket *clientSocket = new QTcpSocket(this);
		clientSocket->setSocketDescriptor(socketDescriptor);

		connect(clientSocket, &QTcpSocket::readyRead, this, &MainServer::handleClientData);
		connect(clientSocket, &QTcpSocket::disconnected, this, &MainServer::handleClientDisconnected);

		qDebug() << "New connection established";
	}


	void MainServer::processDeleteFileRequest(QTcpSocket* clientSocket, const QString& fileName) {
		QSqlQuery query;
		query.prepare("SELECT " FIELD_FILE_PATH " FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
		query.bindValue(":filename", fileName);

		QDataStream out(clientSocket);
		if (query.exec() && query.next()) {
			QString filePath = query.value(FIELD_FILE_PATH).toString();

			if (QFile::exists(filePath) && !QFile::remove(filePath)) {
				qDebug() << "Failed to delete file from disk:" << filePath;
				out << QString(RESPONSE_DELETE_FAILED);
				clientSocket->flush();
				return;
			}

			query.prepare("DELETE FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
			query.bindValue(":filename", fileName);

			if (query.exec()) {
				out << QString(RESPONSE_DELETE_SUCCESS);
			} else {
				qDebug() << "Failed to delete file from database:" << query.lastError();
				out << QString(RESPONSE_DELETE_FAILED);
			}
		} else {
			qDebug() << "File not found in database or query failed:" << query.lastError();
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
			qDebug() << "File not found in database or query failed.";
			return;
		}

		QString filePath = query.value(FIELD_FILE_PATH).toString();
		qint64 fileSize = query.value(FIELD_FILE_SIZE).toLongLong();

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			out << QString(RESPONSE_DOWNLOAD_FAILED);
			clientSocket->flush();
			qDebug() << "Failed to open file for reading:" << file.errorString();
			return;
		}

		QByteArray fileData = file.readAll();
		file.close();

		out << QString(RESPONSE_DOWNLOAD_READY) << fileSize;
		clientSocket->flush();

		if (!clientSocket->waitForReadyRead(3000)) {
			qDebug() << "No response from client for download request.";
			return;
		}

		QDataStream in(clientSocket);
		QString clientResponse;
		in >> clientResponse;

		if (clientResponse != RESPONSE_READY_FOR_DATA) {
			qDebug() << "Client is not ready to receive file.";
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
				qDebug() << "No response from client after sending chunk.";
				return;
			}

			in >> clientResponse;
			if (clientResponse != RESPONSE_CHUNK_RECEIVED) {
				qDebug() << "Client did not acknowledge chunk.";
				return;
			}
		}
		qDebug() << "File sent successfully:" << fileName;
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
			qDebug() << "Login failed: " << query.lastError();
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
			qDebug() << "Registration failed: user already exists.";
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
				qDebug() << "Error deleting an existing file:" << deleteQuery.lastError();
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
					qDebug() << "Received empty chunk, ending upload.";
					break;
				}
				fileData.append(chunk);
				totalReceived += chunk.size();

				out << QString(RESPONSE_CHUNK_RECEIVED);
				clientSocket->flush();
			} else {
				qDebug() << "No data received from client.";
				break;
			}
		}

		QFile file(filePath);
		if (file.open(QIODevice::WriteOnly)) {
			file.write(fileData);
			file.close();
		} else {
			qDebug() << "Failed to write file to disk:" << file.errorString();
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

		out << (query.exec() ? QString(RESPONSE_UPLOAD_SUCCESS) : QString(RESPONSE_UPLOAD_FAILED));
		if (query.lastError().isValid()) qDebug() << query.lastError();
		clientSocket->flush();
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
			qDebug() << "Client disconnected";
			clientSocket->deleteLater();
		}
	}
}
