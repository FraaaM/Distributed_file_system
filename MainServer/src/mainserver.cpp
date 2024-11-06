#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

#include "mainserver.hpp"

namespace SHIZ{
	MainServer::MainServer(QObject* parent): QTcpServer(parent) {
		dataBase = QSqlDatabase::addDatabase("QSQLITE");
		dataBase.setDatabaseName("files.db");

		if (!dataBase.open()) {
			qDebug() << "Failed to connect to database!";
			return;
		}

		QSqlQuery query;

		query.exec("CREATE TABLE IF NOT EXISTS users ("
				   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
				   "username TEXT UNIQUE, "
				   "password TEXT)");

		query.exec("CREATE TABLE IF NOT EXISTS files ("
				   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
				   "filename TEXT, "
				   "owner TEXT, "
				   "size INTEGER, "
				   "upload_date TEXT, "
				   "filepath TEXT)");

		QDir dir(QCoreApplication::applicationDirPath() + "/Uploaded_files");
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
		query.prepare("SELECT filepath FROM files WHERE filename = :filename");
		query.bindValue(":filename", fileName);

		QDataStream out(clientSocket);
		if (query.exec() && query.next()) {
			QString filePath = query.value("filepath").toString();

			if (QFile::exists(filePath) && !QFile::remove(filePath)) {
				qDebug() << "Failed to delete file from disk:" << filePath;
				out << QString("DELETE_FAILED");
				clientSocket->flush();
				return;
			}

			query.prepare("DELETE FROM files WHERE filename = :filename");
			query.bindValue(":filename", fileName);

			if (query.exec()) {
				out << QString("DELETE_SUCCESS");
			} else {
				qDebug() << "srgaaggr" << query.lastError();
				out << QString("DELETE_FAILED");
			}
		} else {
			qDebug() << "File not found in database or query failed:" << query.lastError();
			out << QString("DELETE_FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName) {
		QSqlQuery query;
		query.prepare("SELECT filepath, size FROM files WHERE filename = :filename");
		query.bindValue(":filename", fileName);

		if (!query.exec() || !query.next()) {
			QDataStream out(clientSocket);
			out << QString("DOWNLOAD_FAILED");
			clientSocket->flush();
			qDebug() << "File not found in database or query failed.";
			return;
		}

		QString filePath = query.value("filepath").toString();
		qint64 fileSize = query.value("size").toLongLong();

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			QDataStream out(clientSocket);
			out << QString("DOWNLOAD_FAILED");
			clientSocket->flush();
			qDebug() << "Failed to open file for reading:" << file.errorString();
			return;
		}
		QByteArray fileData = file.readAll();
		file.close();

		QDataStream out(clientSocket);
		out << QString("DOWNLOAD_READY") << fileSize;
		clientSocket->flush();

		if (!clientSocket->waitForReadyRead(3000)) {
			qDebug() << "No response from client for download request.";
			return;
		}

		QDataStream in(clientSocket);
		QString clientResponse;
		in >> clientResponse;

		if (clientResponse != "READY_TO_RECEIVE") {
			qDebug() << "Client is not ready to receive file.";
			return;
		}

		const qint64 chunkSize = 1024;
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
			if (clientResponse != "CHUNK_RECEIVED") {
				qDebug() << "Client did not acknowledge chunk.";
				return;
			}
		}
		qDebug() << "File sent successfully:" << fileName;
	}

	void MainServer::processFileListRequest(QTcpSocket* clientSocket) {
		QSqlQuery query("SELECT filename, owner, size, upload_date FROM files");
		QStringList fileList;

		while (query.next()) {
			QString fileInfo = query.value(0).toString() + "|" + query.value(1).toString() + "|" +
							   query.value(2).toString() + "|" + query.value(3).toString();
			fileList << fileInfo;
		}

		QDataStream out(clientSocket);
		out << QString("FILES_LIST") << fileList;
		clientSocket->flush();
	}

	void MainServer::processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		QDataStream out(clientSocket);

		QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

		QSqlQuery query;
		query.prepare("SELECT password FROM users WHERE username = :username");
		query.bindValue(":username", username);

		if (query.exec() && query.next()) {
			QString storedPassword = query.value(0).toString();
			if (storedPassword == hashedPassword) {
				out << QString("LOGIN_SUCCESS");
			} else {
				out << QString("LOGIN_FAILED");
			}
		} else {
			out << QString("LOGIN_FAILED");
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
		checkQuery.prepare("SELECT id FROM users WHERE username = :username");
		checkQuery.bindValue(":username", username);

		if (checkQuery.exec() && checkQuery.next()) {
			out << QString("REGISTER_USER_EXISTS");
			qDebug() << "Registration failed: user already exists.";
		} else {
			QSqlQuery insertQuery;
			insertQuery.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
			insertQuery.addBindValue(username);
			insertQuery.addBindValue(hashedPassword);

			if (insertQuery.exec()) {
				out << QString("REGISTER_SUCCESS");
			} else {
				out << QString("REGISTER_FAILED");
				qDebug() << "Registration failed: " << insertQuery.lastError();
			}
		}
		clientSocket->flush();
	}

	void MainServer::processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize) {
		QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
		QString filePath = QCoreApplication::applicationDirPath() + "/Uploaded_files/" + fileName;

		QSqlQuery checkQuery;
		checkQuery.prepare("SELECT filepath  FROM files WHERE filename = :filename");
		checkQuery.bindValue(":filename", fileName);

		if (checkQuery.exec() && checkQuery.next()) {
			QString existingFilePath = checkQuery.value("filepath").toString();
			if (QFile::exists(existingFilePath)) {
				QFile::remove(existingFilePath);
			}

			QSqlQuery deleteQuery;
			deleteQuery.prepare("DELETE FROM files WHERE filename = :filename");
			deleteQuery.bindValue(":filename", fileName);
			if (!deleteQuery.exec()) {
				qDebug() << "Error deleting an existing file:" << deleteQuery.lastError();
				QDataStream out(clientSocket);
				out << QString("UPLOAD_FAILED");
				clientSocket->flush();
				return;
			}
		}

		QByteArray fileData;
		qint64 totalReceived = 0;

		QDataStream out(clientSocket);
		out << QString("READY_FOR_DATA");
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

				out << QString("CHUNK_RECEIVED");
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
			out << QString("UPLOAD_FAILED");
			clientSocket->flush();
			return;
		}

		QSqlQuery query;
		query.prepare("INSERT INTO files (filename, owner, size, upload_date, filepath) VALUES (?, ?, ?, ?, ?)");
		query.addBindValue(fileName);
		query.addBindValue(owner);
		query.addBindValue(fileSize);
		query.addBindValue(uploadDate);
		query.addBindValue(filePath);

		if (query.exec()) {
			out << QString("UPLOAD_SUCCESS");
		} else {
			qDebug() << query.lastError();
			out << QString("UPLOAD_FAILED");
		}
		clientSocket->flush();
	}


	void MainServer::handleClientData() {
		QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
		if (!clientSocket) return;

		QDataStream in(clientSocket);
		QString command;
		in >> command;

		if (command == "DOWNLOAD") {
			QString fileName;
			in >> fileName;
			processDownloadRequest(clientSocket, fileName);
		} else if (command == "GET_FILES") {
			processFileListRequest(clientSocket);
		} else if (command == "LOGIN") {
			QStringList parts;
			QString login, password;
			in >> login >> password;
			parts << "LOGIN" << login << password;
			processLoginRequest(clientSocket, parts);
		} else if (command == "REGISTER") {
			QStringList parts;
			QString login, password;
			in >> login >> password;
			parts << "REGISTER" << login << password;
			processRegistrationRequest(clientSocket, parts);
		} else if (command == "UPLOAD") {
			QString fileName, owner;
			qint64 fileSize;
			in >> fileName >> owner >> fileSize;
			processUploadRequest(clientSocket, fileName, owner, fileSize);
		} else if (command == "DELETE") {
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
