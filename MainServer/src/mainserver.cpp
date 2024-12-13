#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include<QDebug>

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
                        FIELD_USER_PASSWORD " TEXT, "
                        FIELD_USER_IS_ADMIN " NUMERIC,"
                       FIELD_USER_GROUP_ID " TEXT,"
                       FIELD_USER_RIGHTS " TEXT)"
                   );
		query.exec("CREATE TABLE IF NOT EXISTS " TABLE_FILES " ("
				   FIELD_FILE_ID " INTEGER PRIMARY KEY AUTOINCREMENT, "
				   FIELD_FILE_FILENAME " TEXT, "
				   FIELD_FILE_OWNER " TEXT, "
				   FIELD_FILE_SIZE " INTEGER, "
                   FIELD_FILE_UPLOAD_DATE " TEXT, "
                   FIELD_FILE_GROUP_ID     " INTEGER)"
                   );
		query.exec("CREATE TABLE IF NOT EXISTS " TABLE_FILE_REPLICAS " ("
				   FIELD_REPLICA_ID " INTEGER PRIMARY KEY AUTOINCREMENT, "
				   FIELD_FILE_FILENAME " TEXT, "
				   FIELD_REPLICA_ADDRESS " TEXT, "
				   FIELD_REPLICA_PORT " INTEGER,"
				   FIELD_FILE_UPLOAD_DATE " TEXT)");
		logger->log("Query CREATE TABLE last error: " + query.lastError().text());

        QString username("admin");
        QString password("!For@each");
        QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
        QString isAdmin("true");
        QString group("1");
        QString rights("rwd");


        query.prepare("INSERT INTO " TABLE_USERS " (" FIELD_USER_USERNAME ", " FIELD_USER_PASSWORD ", " FIELD_USER_IS_ADMIN ", " FIELD_FILE_GROUP_ID ", " FIELD_USER_RIGHTS ") VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(username);
        query.addBindValue(hashedPassword);
        query.addBindValue(isAdmin);
        query.addBindValue(group);
        query.addBindValue(rights);

        if(!query.exec()){
            logger->log("Query INSERT ADMIN last error: " + query.lastError().text());
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


	bool MainServer::distributeFileToReplicas(const QString& fileName, const QByteArray& fileData, const QString& uploadDate) {
		bool atLeastOneSuccess = false;
		for (QTcpSocket* replicaSocket : replicaSockets) {
			QString replicaAddress = replicaSocket->peerAddress().toString();
			quint16 replicaPort = replicaSocket->peerPort();

			QSqlQuery checkQuery;
			checkQuery.prepare("SELECT " FIELD_REPLICA_ID " FROM " TABLE_FILE_REPLICAS
							   " WHERE " FIELD_FILE_FILENAME " = :filename AND "
							   FIELD_REPLICA_ADDRESS " = :address AND "
							   FIELD_REPLICA_PORT " = :port");
			checkQuery.bindValue(":filename", fileName);
			checkQuery.bindValue(":address", replicaAddress);
			checkQuery.bindValue(":port", replicaPort);

			if (checkQuery.exec() && checkQuery.next()) {
				QDataStream out(replicaSocket);
				out << QString(COMMAND_REPLICA_DELETE) << fileName;
				replicaSocket->flush();

				if (!replicaSocket->waitForReadyRead(3000)) {
					logger->log("No response from replica for delete.");
				} else {
					QDataStream in(replicaSocket);
					QString response;
					in >> response;

					if (response != RESPONSE_DELETE_SUCCESS) {
						logger->log("Replica failed to delete existing file: " + fileName);
					}
				}

				QSqlQuery deleteQuery;
				deleteQuery.prepare("DELETE FROM " TABLE_FILE_REPLICAS
									" WHERE " FIELD_FILE_FILENAME " = :filename AND "
									FIELD_REPLICA_ADDRESS " = :address AND "
									FIELD_REPLICA_PORT " = :port");
				deleteQuery.bindValue(":filename", fileName);
				deleteQuery.bindValue(":address", replicaAddress);
				deleteQuery.bindValue(":port", replicaPort);
				deleteQuery.exec();
			}

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
				query.prepare("INSERT INTO " TABLE_FILE_REPLICAS " ("
							  FIELD_FILE_FILENAME ", "
							  FIELD_REPLICA_ADDRESS ", "
							  FIELD_REPLICA_PORT ", "
							  FIELD_FILE_UPLOAD_DATE ") VALUES (?, ?, ?, ?)");
				query.addBindValue(fileName);
				query.addBindValue(replicaAddress);
				query.addBindValue(replicaPort);
				query.addBindValue(uploadDate);

				if (!query.exec()) {
					logger->log("Failed to save replica file info: " + query.lastError().text() + replicaAddress + ":" + QString::number(replicaPort));
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
		QDataStream out(clientSocket);

		QSqlQuery query;
		query.prepare("DELETE FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename");
		query.bindValue(":filename", fileName);
		if (!query.exec()) {
			logger->log("Failed to delete file from database: " + query.lastError().text());
			out << QString(RESPONSE_DELETE_FAILED);
			clientSocket->flush();
			return;
		}

		query.prepare("SELECT " FIELD_REPLICA_ADDRESS ", " FIELD_REPLICA_PORT " FROM " TABLE_FILE_REPLICAS " WHERE " FIELD_FILE_FILENAME " = :filename");
		query.bindValue(":filename", fileName);

		QList<QPair<QString, quint16>> replicaList;
		if (query.exec()) {
			while (query.next()) {
				QString address = query.value(FIELD_REPLICA_ADDRESS).toString();
				quint16 port = query.value(FIELD_REPLICA_PORT).toUInt();
				replicaList.append(qMakePair(address, port));
			}

			for (const auto& replica : replicaList) {
				for (QTcpSocket* replicaSocket : replicaSockets) {
					if (replicaSocket->peerAddress().toString() == replica.first && replicaSocket->peerPort() == replica.second) {
						QDataStream replicaOut(replicaSocket);
						replicaOut << QString(COMMAND_REPLICA_DELETE) << fileName;
						replicaSocket->flush();
						logger->log("Sent delete request to replica: " + replica.first + ":" + QString::number(replica.second));
					}
				}
			}

			query.prepare("DELETE FROM " TABLE_FILE_REPLICAS " WHERE " FIELD_FILE_FILENAME " = :filename");
			query.bindValue(":filename", fileName);
			if (!query.exec()) {
				logger->log("Failed to delete file replicas info: " + query.lastError().text());
				out << QString(RESPONSE_DELETE_FAILED);
				clientSocket->flush();
				return;
			}
		} else {
			logger->log("Failed to retrieve replica list: " + query.lastError().text());
		}

		out << QString(RESPONSE_DELETE_SUCCESS);
		clientSocket->flush();
	}

    void MainServer::processDeleteUserRequest(QTcpSocket* clientSocket, const QString& userName) {
        QDataStream out(clientSocket);

        QSqlQuery query;
        query.prepare("DELETE FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
        query.bindValue(":username", userName);
        if (!query.exec()) {
            logger->log("Failed to delete user from database: " + query.lastError().text());
            out << QString(RESPONSE_DELETE_FAILED);
            clientSocket->flush();
            return;
        }

        out << QString(RESPONSE_DELETE_SUCCESS);
        clientSocket->flush();
    }

	void MainServer::processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName) {
		QSqlQuery query;
		query.prepare("SELECT " FIELD_REPLICA_ADDRESS ", " FIELD_REPLICA_PORT " FROM " TABLE_FILE_REPLICAS
					  " WHERE " FIELD_FILE_FILENAME " = :filename");
		query.bindValue(":filename", fileName);

		QDataStream out(clientSocket);

		if (!query.exec()) {
			out << QString(RESPONSE_DOWNLOAD_FAILED);
			clientSocket->flush();
			logger->log("Failed to query replicas for file: " + query.lastError().text());
			return;
		}

		QList<QPair<QString, quint16>> replicaList;
		while (query.next()) {
			QString address = query.value(FIELD_REPLICA_ADDRESS).toString();
			quint16 port = query.value(FIELD_REPLICA_PORT).toUInt();
			replicaList.append(qMakePair(address, port));
		}

		if (replicaList.isEmpty()) {
			out << QString(RESPONSE_DOWNLOAD_FAILED);
			clientSocket->flush();
			logger->log("No replicas found for file: " + fileName);
			return;
		}

		for (const auto& replica : replicaList) {
			if (tryDownloadFromReplica(clientSocket, fileName, replica.first, replica.second)) {
				return;
			}
		}

		out << QString(RESPONSE_DOWNLOAD_FAILED);
		clientSocket->flush();
		logger->log("Failed to fetch file from all replicas: " + fileName);
    }

    void MainServer::processFileListRequest(QTcpSocket* clientSocket, const QString& userName) {
        QDataStream out(clientSocket);

        QSqlQuery userGroupId;
        userGroupId.prepare("SELECT " FIELD_USER_GROUP_ID " FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
        userGroupId.bindValue(":username", userName);

        if (!userGroupId.exec()) {
            logger->log("Failed to get user groups: " + userGroupId.lastError().text());
            return;
        }

        QStringList groupId;

        if (!userGroupId.next()) {
            out << QString(RESPONSE_USER_DOES_NOT_EXIST);
            clientSocket->flush();
            return;
        }

        do {
            groupId << userGroupId.value(0).toString().split(",");
        } while (userGroupId.next());


        QString q = "SELECT " FIELD_FILE_FILENAME ", " FIELD_FILE_OWNER ", " FIELD_FILE_SIZE ", " FIELD_FILE_UPLOAD_DATE ", " FIELD_FILE_GROUP_ID " FROM " TABLE_FILES " WHERE ";

        QStringList conditions;
        for(const QString& elem : groupId){
            conditions << QString(FIELD_FILE_GROUP_ID " LIKE '%,%1'").arg(elem);
            conditions << QString(FIELD_FILE_GROUP_ID " = %1").arg(elem);
        }

        q += conditions.join(" OR ");

        QSqlQuery query;
        query.prepare(q);


        if (!query.exec()) {
             qDebug() <<  query.lastError().text();
            logger->log("Failed to execute query: " + query.lastError().text());
            return;
        }

        QStringList fileList;

		while (query.next()) {
			QString fileInfo = query.value(0).toString() + "|" + query.value(1).toString() + "|" +
                               query.value(2).toString() + "|" + query.value(3).toString() + "|" +
                               query.value(4).toString();
			fileList << fileInfo;
		}

		out << QString(RESPONSE_FILES_LIST) << fileList;
		clientSocket->flush();
	}

    void MainServer::processUserListRequest(QTcpSocket* clientSocket) {
        QSqlQuery query("SELECT " FIELD_USER_USERNAME ", " FIELD_USER_IS_ADMIN ", " FIELD_USER_GROUP_ID ", " FIELD_USER_RIGHTS " FROM " TABLE_USERS);
        QStringList userList;

        while (query.next()) {
            QString userInfo = query.value(0).toString() + "|" + query.value(1).toString() + "|" +
                               query.value(2).toString() + "|" + query.value(3).toString();
            userList << userInfo;
        }

        QDataStream out(clientSocket);
        out << QString(RESPONSE_USERS_LIST) << userList;
        clientSocket->flush();
    }

	void MainServer::processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts) {
		QString username = parts.value(1);
		QString password = parts.value(2);

        QDataStream out(clientSocket);

		QString hashedPassword = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

		QSqlQuery query;
        query.prepare("SELECT " FIELD_USER_PASSWORD ", " FIELD_USER_IS_ADMIN " FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
		query.bindValue(":username", username);

		if (query.exec() && query.next()) {
			QString storedPassword = query.value(0).toString();
                QString isAdmin = query.value(1).toString();
            out << (storedPassword == hashedPassword ? (isAdmin == QString("true") ? QString(RESPONSE_LOGIN_ADMIN_SUCCESS) : QString(RESPONSE_LOGIN_USER_SUCCESS))  : QString(RESPONSE_LOGIN_FAILED));
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
            insertQuery.prepare("INSERT INTO " TABLE_USERS " (" FIELD_USER_USERNAME ", " FIELD_USER_PASSWORD ", " FIELD_USER_GROUP_ID ", " FIELD_USER_RIGHTS ") VALUES (?, ?, 1, 'rwd')");
			insertQuery.addBindValue(username);
			insertQuery.addBindValue(hashedPassword);

			out << (insertQuery.exec() ? QString(RESPONSE_REGISTER_SUCCESS) : QString(RESPONSE_REGISTER_FAILED));
			if (insertQuery.lastError().isValid()) qDebug() << "Registration failed: " << insertQuery.lastError();
		}
		clientSocket->flush();
	}

    void MainServer::processUpdateUserRequest(QTcpSocket *clientSocket, const QString &userName, const QString &key, const QString& value){
        QDataStream out(clientSocket);

        QSqlQuery updateUserQuery;
        if(key == FIELD_USER_GROUP_ID)
            updateUserQuery.prepare("UPDATE " TABLE_USERS " SET " FIELD_USER_GROUP_ID " = :value WHERE " FIELD_USER_USERNAME " = :username");
        else if(key == FIELD_USER_RIGHTS)
            updateUserQuery.prepare("UPDATE " TABLE_USERS " SET " FIELD_USER_RIGHTS " = :value WHERE " FIELD_USER_USERNAME " = :username");
        else if(key == FIELD_USER_IS_ADMIN){
            updateUserQuery.prepare("UPDATE " TABLE_USERS " SET " FIELD_USER_IS_ADMIN " = :value WHERE " FIELD_USER_USERNAME " = :username");
        }
        updateUserQuery.bindValue(":value", value);
        updateUserQuery.bindValue(":username", userName);

        if (!updateUserQuery.exec()) {
            logger->log("Error updating an user data:" + updateUserQuery.lastError().text());

            out << QString(RESPONSE_UPDATE_USER_FAILED);
            clientSocket->flush();
        }else{
            out << QString(RESPONSE_UPDATE_USER_SUCCESS);
            clientSocket->flush();
        }
    }
    void MainServer::processGetUserInfoRequest(QTcpSocket *clientSocket, const QString &userName){
        QDataStream out(clientSocket);

        QSqlQuery getUserInfoQuery;
        getUserInfoQuery.prepare("SELECT " FIELD_USER_RIGHTS ", " FIELD_USER_GROUP_ID " FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
        getUserInfoQuery.bindValue(":username", userName);

        if (!getUserInfoQuery.exec()) {
            logger->log("Error getting rights of user:" + getUserInfoQuery.lastError().text());
            return;
        }

        if(getUserInfoQuery.next()) {
            QString userInfo = getUserInfoQuery.value(0).toString() + "|"
                               + getUserInfoQuery.value(1).toString();
            out  << QString(RESPONSE_USER_INFO) << userInfo;
            clientSocket->flush();
        }
    }
    void MainServer::processGetFileInfoRequest(QTcpSocket *clientSocket, const QString &fileName){
        QDataStream out(clientSocket);

        QSqlQuery getGroupOfFile;
        getGroupOfFile.prepare("SELECT " FIELD_FILE_GROUP_ID " FROM " TABLE_FILES " WHERE " FIELD_FILE_FILENAME " = :filename AND ");
        getGroupOfFile.bindValue(":filename", fileName);
        if (!getGroupOfFile.exec()) {
            logger->log("Error getting group of file:" + getGroupOfFile.lastError().text());

            return;
        }
        if(getGroupOfFile.next()) {
            QString fileInfo = getGroupOfFile.value(0).toString();

            out  << QString(RESPONSE_FILE_INFO) << fileInfo;
            clientSocket->flush();
        }
    }

	void MainServer::processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize) {
		QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

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

		if (!distributeFileToReplicas(fileName, fileData, uploadDate)) {
			out << QString(RESPONSE_UPLOAD_FAILED);
			clientSocket->flush();
			logger->log("Failed to distribute file to replicas: " + fileName);
			return;
		}

        QSqlQuery userGroupQuery;
        userGroupQuery.prepare("SELECT " FIELD_USER_GROUP_ID " FROM " TABLE_USERS " WHERE " FIELD_USER_USERNAME " = :username");
        userGroupQuery.bindValue(":username", owner);

        if (!userGroupQuery.exec()) {
            out << QString(RESPONSE_UPLOAD_FAILED);
            clientSocket->flush();
            logger->log("Failed to find group of user in database: " + userGroupQuery.lastError().text());
            return;
        }

        QString groupId;
        while(userGroupQuery.next()){
            groupId = userGroupQuery.value(0).toString();
        }

		QSqlQuery query;
        query.prepare("INSERT INTO " TABLE_FILES " (" FIELD_FILE_FILENAME ", " FIELD_FILE_OWNER ", " FIELD_FILE_SIZE ", " FIELD_FILE_UPLOAD_DATE ", " FIELD_FILE_GROUP_ID ") VALUES (?, ?, ?, ?, ?)");
		query.addBindValue(fileName);
		query.addBindValue(owner);
		query.addBindValue(fileSize);
		query.addBindValue(uploadDate);
        query.addBindValue(groupId);

		if (!query.exec()) {
			out << QString(RESPONSE_UPLOAD_FAILED);
			clientSocket->flush();
			logger->log("Failed to insert file record into database: " + query.lastError().text());
			return;
		}

		out << QString(RESPONSE_UPLOAD_SUCCESS);
		clientSocket->flush();
		logger->log("File received and processed successfully: " + fileName);
	}

	bool MainServer::tryDownloadFromReplica(QTcpSocket* clientSocket, const QString& fileName, const QString& address, quint16 port) {
		QTcpSocket* replicaSocket = nullptr;

		for (QTcpSocket* socket : replicaSockets) {
			if (socket->peerAddress().toString() == address && socket->peerPort() == port) {
				replicaSocket = socket;
				break;
			}
		}

		if (!replicaSocket) {
			logger->log("There is no connected replica with  file: " + fileName);
            return false;
		}

		QDataStream out(replicaSocket);
		out << QString(COMMAND_REPLICA_DOWNLOAD) << fileName;
		replicaSocket->flush();

		if (!replicaSocket->waitForReadyRead(3000)) {
			logger->log("No response from replica at " + address + ":" + QString::number(port));
			return false;
		}

		QDataStream replicaIn(replicaSocket);
		QString response;
		replicaIn >> response;

		if (response != RESPONSE_DOWNLOAD_READY) {
			logger->log("Replica not ready for download or file not found: " + fileName);
			return false;
		}

		QDataStream replicaOut(replicaSocket);
		replicaOut << QString(RESPONSE_READY_FOR_DATA);
		replicaSocket->flush();

		qint64 fileSize;
		replicaIn >> fileSize;

		QDataStream clientOut(clientSocket);
		clientOut << QString(RESPONSE_DOWNLOAD_READY) << fileSize;
		clientSocket->flush();

		if (!clientSocket->waitForReadyRead(3000)) {
			logger->log("Client not ready to receive file.");
			return false;
		}

		QString clientResponse;
		QDataStream clientIn(clientSocket);
		clientIn >> clientResponse;

		if (clientResponse != RESPONSE_READY_FOR_DATA) {
			logger->log("Client rejected file download.");
			return false;
		}

		qint64 bytesReceived = 0;
		const qint64 chunkSize = CHUNK_SIZE;

		while (bytesReceived < fileSize) {
			if (!replicaSocket->waitForReadyRead(3000)) {
				logger->log("Timeout while receiving file data from replica.");
				return false;
			}

			QByteArray chunk;
			replicaIn >> chunk;

			clientOut << chunk;
			clientSocket->flush();

			bytesReceived += chunk.size();

			replicaOut << QString(RESPONSE_CHUNK_RECEIVED);
			replicaSocket->flush();

			if (!clientSocket->waitForReadyRead(3000)) {
				logger->log("Client did not acknowledge chunk.");
				return false;
			}

			clientIn >> clientResponse;
			if (clientResponse != RESPONSE_CHUNK_RECEIVED) {
				logger->log("Client failed to acknowledge received chunk.");
				return false;
			}
		}

		logger->log("File fetched successfully from replica and sent to client: " + fileName);
		return true;
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
            QString userName;
            in >> userName;
            processFileListRequest(clientSocket, userName);
        } else if(command == COMMAND_GET_USERS){
            processUserListRequest(clientSocket);
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
        }else if(command == COMMAND_DELETE_USER){
            QString userName;
            in >> userName;
            processDeleteUserRequest(clientSocket, userName);
        }else if(command == COMMAND_UPDATE_USER){
            QString userName, key, value;
            in >> userName >> key >> value;
            processUpdateUserRequest(clientSocket, userName, key, value);
        }else if(command == COMMAND_GET_FILE_INFO){
            QString fileName;
            in >> fileName;
            processGetFileInfoRequest(clientSocket, fileName);
        }else if(command == COMMAND_GET_USER_INFO){
            QString userName;
            in >> userName;
            processGetUserInfoRequest(clientSocket, userName);
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
