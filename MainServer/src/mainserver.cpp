#include <QDateTime>
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
		query.prepare("SELECT data FROM files WHERE filename = ?");
		query.addBindValue(fileName);
		if (query.exec() && query.next()) {
			QByteArray fileData = query.value(0).toByteArray();
			QString response = "DOWNLOAD:" + fileName + ":" + QString::fromUtf8(fileData);
			clientSocket->write(response.toUtf8());
		} else {
			clientSocket->write("DOWNLOAD_FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processFileListRequest(QTcpSocket* clientSocket) {
		QSqlQuery query("SELECT filename, owner, size, upload_date FROM files");
		QStringList fileList;

		while (query.next()) {
			QString fileInfo = query.value(0).toString() + "|" + query.value(1).toString() + "|" +
							   query.value(2).toString() + "|" + query.value(3).toString();
			fileList << fileInfo;
		}

		QString response = "FILES_LIST:" + fileList.join(";");
		clientSocket->write(response.toUtf8());
		clientSocket->flush();
	}

	void MainServer::processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		if (!username.isEmpty() && !password.isEmpty()) {
			clientSocket->write("SUCCESS");
		} else {
			clientSocket->write("FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

		if (!username.isEmpty() && !password.isEmpty()) {
			clientSocket->write("SUCCESS");
		} else {
			clientSocket->write("FAILED");
		}
		clientSocket->flush();
	}

	void MainServer::processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, const QByteArray& fileData, qint64 fileSize) {
		QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

		QSqlQuery query;
		query.prepare("INSERT INTO files (filename, owner, size, upload_date, data) VALUES (?, ?, ?, ?, ?)");
		query.addBindValue(fileName);
		query.addBindValue(owner);
		query.addBindValue(fileSize);
		query.addBindValue(uploadDate);
		query.addBindValue(fileData);

		if (query.exec()) {
			clientSocket->write("UPLOAD_SUCCESS");
		} else {
			clientSocket->write("UPLOAD_FAILED");
		}
		clientSocket->flush();
	}


	void MainServer::handleClientData() {
		QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
		if (!clientSocket) return;

		QByteArray data = clientSocket->readAll();
		QString request = QString::fromUtf8(data);
		qDebug() << "Received request:" << request;

		QStringList parts = request.split(":");
		if (parts.isEmpty()) return;

		if (request.startsWith("DOWNLOAD:")) {
			processDownloadRequest(clientSocket, parts.value(1));
		} else if (request.startsWith("GET_FILES")) {
			processFileListRequest(clientSocket);
		} else if (request.startsWith("LOGIN:")) {
			processLoginRequest(clientSocket, parts);
		} else if (request.startsWith("REGISTER:")) {
			processRegistrationRequest(clientSocket, parts);
		} else   if (request.startsWith("UPLOAD:")) {
			QString fileName = parts.value(1);
			QString owner = parts.value(2);
			QByteArray fileData = parts.value(3).toUtf8();
			qint64 fileSize = parts.value(4).toLongLong();
			processUploadRequest(clientSocket, fileName, owner, fileData, fileSize);
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
