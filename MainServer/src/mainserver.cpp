#include <QDateTime>
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
		query.exec("CREATE TABLE IF NOT EXISTS files ("
				   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
				   "filename TEXT, "
				   "owner TEXT, "
				   "size INTEGER, "
				   "upload_date TEXT, "
				   "data BLOB)");
		query.exec("PRAGMA max_page_count = 2147483646;");
		qDebug() << query.lastError();
	}


	void MainServer::incomingConnection(qintptr socketDescriptor) {
		QTcpSocket *clientSocket = new QTcpSocket(this);
		clientSocket->setSocketDescriptor(socketDescriptor);

		connect(clientSocket, &QTcpSocket::readyRead, this, &MainServer::handleClientData);
		connect(clientSocket, &QTcpSocket::disconnected, this, &MainServer::handleClientDisconnected);

		qDebug() << "New connection established";
	}


	void MainServer::processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName) {
		QSqlQuery query;
		query.prepare("SELECT data, size FROM files WHERE filename = :filename");
		query.bindValue(":filename", fileName);

		if (!query.exec() || !query.next()) {
			QDataStream out(clientSocket);
			out << QString("DOWNLOAD_FAILED");
			clientSocket->flush();
			qDebug() << "File not found in database or query failed.";
			return;
		}

		QByteArray fileData = query.value("data").toByteArray();
		qint64 fileSize = query.value("size").toLongLong();

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
		if (!username.isEmpty() && !password.isEmpty()) {
			out << QString("SUCCESS");
		} else {
			out << QString("FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		QDataStream out(clientSocket);
		if (!username.isEmpty() && !password.isEmpty()) {
			out << QString("SUCCESS");
		} else {
			out << QString("FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize) {
		QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

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

		QSqlQuery query;
		query.prepare("INSERT INTO files (filename, owner, size, upload_date, data) VALUES (?, ?, ?, ?, ?)");
		query.addBindValue(fileName);
		query.addBindValue(owner);
		query.addBindValue(fileSize);
		query.addBindValue(uploadDate);
		query.addBindValue(fileData);

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
