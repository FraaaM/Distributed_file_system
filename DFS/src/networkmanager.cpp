#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "macros.hpp"
#include "networkmanager.hpp"

namespace SHIZ {
	NetworkManager::NetworkManager(Logger* logger, QObject* parent)
		: logger(logger), QObject(parent), host(""), port(DEFAULT_PORT)
	{
		tcpSocket = new QTcpSocket(this);
		reconnectTimer = new QTimer(this);

		connect(tcpSocket, &QTcpSocket::connected, this, &NetworkManager::onConnected);
		connect(tcpSocket, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
		connect(reconnectTimer, &QTimer::timeout, this, &NetworkManager::onReconnectToServer);
	}

	NetworkManager::~NetworkManager() {
		onDisconnectRequest();
	}


	void NetworkManager::setHostAndPort(const QString& host, quint16 port) {
		this->host = host;
		this->port = port;
	}


	void NetworkManager::onConnectRequest(const QString& host, quint16 port, bool isReconnrection) {
		this->host = host;
		this->port = port;
		tcpSocket->connectToHost(host, port);
		if (tcpSocket->waitForConnected(RESPONSE_TIMEOUT)) {
			logger->log("Connected to main server successfully");
			QDataStream out(tcpSocket);
			out << QString(CLIENT);
			if(!isReconnrection)
				emit connectResult(true);
		} else {
			logger->log("Failed to main server connect");
			if(!isReconnrection)
				emit connectResult(false);
		}
	}

    void NetworkManager::onDeleteFileRequest(const QString& fileName, const QString& userName) {
		emit statusMessage("Requesting file deletion...");
		logger->log("Requesting file deletion...");

		QDataStream out(tcpSocket);
        out << QString(COMMAND_DELETE) << fileName << userName;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

            if(response == RESPONSE_DELETE_NOT_ALLOW){
			emit deleteFileResult(RESPONSE_DELETE_NOT_ALLOW);
			emit statusMessage("File deletion is not allowed.");
			logger->log("File deletion is not allowed.");
			return;
            }else{
			bool success = (response == RESPONSE_DELETE_SUCCESS);
			emit deleteFileResult(response);
			emit statusMessage(success ? "File deleted successfully." : "File deletion failed.");
			logger->log(success ? "File deleted successfully." : "File deletion failed.");
			return;
            }
		} else {
			emit deleteFileResult(RESPONSE_DELETE_FAILED);
			emit statusMessage("Server response timed out.");
			logger->log("Server response timed out.");
		}
	}

	void NetworkManager::onDeleteUserRequest(const QString& userName) {
		emit statusMessage("Requesting user deletion...");
		logger->log("Requesting user deletion...");

		QDataStream out(tcpSocket);
		out << QString(COMMAND_DELETE_USER) << userName;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

			bool success = (response == RESPONSE_DELETE_SUCCESS);
			emit deleteUserResult(false);
			emit statusMessage(success ? "User deleted successfully." : "User deletion failed.");
			logger->log(success ? "User deleted successfully." : "User deletion failed.");

		} else {
			emit deleteUserResult(false);
			emit statusMessage("Server response timed out.");
			logger->log("Server response timed out.");
		}
	}

	void NetworkManager::onDisconnectRequest() {
		if (tcpSocket->isOpen()) {
			tcpSocket->disconnectFromHost();
		}
		reconnectTimer->stop();
		logger->log("Disconnected from server and stopped reconnect attempts.");
	}

    void NetworkManager::onDownloadFileRequest(const QString& filePath, const QString& userName) {
		QString fileName = QFileInfo(filePath).fileName();
		emit statusMessage("Requesting file download: " + fileName);
		logger->log("Requesting file download: " + fileName);

		QDataStream out(tcpSocket);
		out << QString(COMMAND_DOWNLOAD) << fileName << userName;
		tcpSocket->flush();

		if (!tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT * 10)) {
			emit statusMessage("Server did not respond in time for download.");
			logger->log("Server did not respond in time for download.");
			emit downloadFileResult(RESPONSE_DOWNLOAD_FAILED);
			return;
		}

		QDataStream in(tcpSocket);
		QString response;
		qint64 fileSize = 0;

		in >> response;
		if (response == RESPONSE_READ_NOT_ALLOW) {
			emit downloadFileResult(RESPONSE_READ_NOT_ALLOW);
			emit statusMessage("User is not allowed to download files.");
			logger->log("User is not allowed to download files.");
			return;
		}
		if (response != RESPONSE_DOWNLOAD_READY) {
			emit statusMessage("Server not ready for download.");
			logger->log("Server not ready for download.");
			emit downloadFileResult(RESPONSE_DOWNLOAD_FAILED);
			return;
		}

		in >> fileSize;
		emit statusMessage("Downloading " + fileName +
						   " (" + QString::number(fileSize) + " bytes)...");
		logger->log("Downloading " + fileName +
					" (" + QString::number(fileSize) + " bytes)...");

		out << QString(RESPONSE_READY_FOR_DATA);
		tcpSocket->flush();

		QFile file(filePath);
		if (!file.open(QIODevice::WriteOnly)) {
			emit statusMessage("Failed to open file for writing.");
			logger->log("Failed to open file for writing.");
			emit downloadFileResult(RESPONSE_DOWNLOAD_FAILED);
			return;
		}

		qint64 totalReceived = 0;
		const qint64 chunkSize = CHUNK_SIZE;
		QByteArray chunk;

		while (totalReceived < fileSize) {
			if (!tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT * 10)) {
				emit statusMessage("No response from server during download.");
				logger->log("No response from server during download.");
				file.close();
				emit downloadFileResult(RESPONSE_DOWNLOAD_FAILED);
				return;
			}

			in >> chunk;
			file.write(chunk);
			totalReceived += chunk.size();

			emit statusMessage("Downloading " + fileName + ": " +
							   QString::number(totalReceived) + " of " +
							   QString::number(fileSize) + " bytes received");
			logger->log("Downloading " + fileName + ": " +
						QString::number(totalReceived) + " of " +
						QString::number(fileSize) + " bytes received");

			out << QString(RESPONSE_CHUNK_RECEIVED);
			tcpSocket->flush();
		}

		file.close();
		emit statusMessage("Download complete: " + fileName);
		logger->log("Download complete: " + fileName);
		emit downloadFileResult(RESPONSE_DOWNLOAD_READY);
	}

	void NetworkManager::onListFileRequest(const QString& userName) {
		QDataStream out(tcpSocket);
		out << QString(COMMAND_GET_FILES) << userName;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			QDataStream in(tcpSocket);
			QString response;
			QStringList fileList;

			in >> response;
			if (response == RESPONSE_FILES_LIST) {
				in >> fileList;
				emit listFileResult(fileList);
				return;
			} else if (response == RESPONSE_USER_DOES_NOT_EXIST) {
				emit listFileResult(QStringList() << RESPONSE_USER_DOES_NOT_EXIST);
				return;
			}
		} else {
			emit listFileResult(QStringList());
		}
	}

	void NetworkManager::onLoginRequest(const QString& login, const QString& password) {
		QDataStream out(tcpSocket);
		out << QString(COMMAND_LOGIN) << login << password;
		tcpSocket->flush();

		QString success = "fail";
		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

			if (response == RESPONSE_LOGIN_USER_SUCCESS) {
				success = USER;
			} else if (response == RESPONSE_LOGIN_ADMIN_SUCCESS) {
				success = ADMIN;
			} else {
				success = "Incorrect username or password.";
				logger->log("Login error: " + success);
			}
		} else {
			success = "Failed to connect to the server.";
			logger->log("Login error: " + success);
		}

		emit loginResult(success, login);
	}

	void NetworkManager::onRegistrationRequest(const QString& login, const QString& password) {
		QDataStream out(tcpSocket);
		out << QString(COMMAND_REGISTER) << login << password;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

			if (response == RESPONSE_REGISTER_SUCCESS) {
				emit registrationResult(true, "Registration was successful.");
			} else if (response == RESPONSE_REGISTER_USER_EXISTS) {
				emit registrationResult(false, "A user with this username already exists.");
			} else {
				emit registrationResult(false, "An unknown error occurred during registration.");
			}
		} else {
			emit registrationResult(false, "Failed to connect to the server.");
		}
	}

	void NetworkManager::onUpdateUserRequest(const QString &userName, const QString &key, const QString &value) {
		emit statusMessage("Requesting user updating...");
		logger->log("Requesting user updating...");

		QDataStream out(tcpSocket);
		out << QString(COMMAND_UPDATE_USER) << userName << key << value;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

			bool success = (response == RESPONSE_UPDATE_USER_SUCCESS);
			emit updateUserResult(success);
			emit statusMessage(success ? "User updated successfully." : "User update failed.");
			logger->log(success ? "User updated successfully." : "User update failed.");
			return;
		} else {
			emit updateUserResult(false);
			emit statusMessage("Server response timed out.");
			logger->log("Server response timed out.");
		}
	}

	void NetworkManager::onUploadFileRequest(const QString& filePath, const QString& owner) {
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			emit statusMessage("Cannot open file for reading.");
			logger->log("Cannot open file for reading.");
			emit uploadFileResult(RESPONSE_UPLOAD_FAILED);
			return;
		}

		QString fileName = QFileInfo(filePath).fileName();
		qint64 fileSize = file.size();
		emit statusMessage("Uploading file: " + fileName);
		logger->log("Uploading file:" + fileName + " Owner:" + owner + " Size:" + QString::number(fileSize));

		QDataStream out(tcpSocket);
		out << QString(COMMAND_UPLOAD) << fileName << owner << fileSize;
		tcpSocket->flush();

		if (!tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT * 10)) {
			emit statusMessage("Server did not respond in time for upload.");
			logger->log("Server did not respond in time for upload.");
			emit uploadFileResult(RESPONSE_UPLOAD_FAILED);
			return;
		}

		QDataStream in(tcpSocket);
		QString response;
		in >> response;

		if (response == RESPONSE_WRITE_NOT_ALLOW) {
			emit uploadFileResult(RESPONSE_WRITE_NOT_ALLOW);
			emit statusMessage("User is not allowed to upload files.");
			logger->log("User is not allowed to upload files.");
			return;
		}

		if (response != RESPONSE_READY_FOR_DATA) {
			emit statusMessage("Server is not ready for data.");
			logger->log("Server is not ready for data.");
			emit uploadFileResult(RESPONSE_UPLOAD_FAILED);
			return;
		}

		const qint64 chunkSize = CHUNK_SIZE;
		qint64 totalSent = 0;

		while (!file.atEnd()) {
			QByteArray buffer = file.read(chunkSize);
			out << buffer;
			tcpSocket->flush();
			totalSent += buffer.size();

			emit statusMessage("Uploading " + fileName + ": " +
							   QString::number(totalSent) + " of " +
							   QString::number(fileSize) + " bytes sent");
			logger->log("Uploading " + fileName + ": " +
						QString::number(totalSent) + " of " +
						QString::number(fileSize) + " bytes sent");

			if (!tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT * 10)) {
				emit statusMessage("No response from server during upload.");
				logger->log("No response from server after sending chunk.");
				emit uploadFileResult(RESPONSE_UPLOAD_FAILED);
				return;
			}

			in >> response;
			if (response != RESPONSE_CHUNK_RECEIVED) {
				emit statusMessage("Server did not acknowledge chunk.");
				logger->log("Server did not acknowledge chunk.");
				emit uploadFileResult(RESPONSE_UPLOAD_FAILED);
				return;
			}
		}

		out << QByteArray();
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT * 10)) {
			in >> response;
			bool success = (response == RESPONSE_UPLOAD_SUCCESS);
			emit statusMessage(success ? "File uploaded successfully." : "File upload failed.");
			logger->log(success ? "File uploaded successfully." : "File upload failed.");
			emit uploadFileResult(response);
			return;
		}

		emit statusMessage("No response from server after upload.");
		logger->log("No response from server after upload.");
		emit uploadFileResult(RESPONSE_UPLOAD_FAILED);
	}

	void NetworkManager::onUserListRequest() {
		QDataStream out(tcpSocket);
		out << QString(COMMAND_GET_USERS);
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(RESPONSE_TIMEOUT * 10)) {
			QDataStream in(tcpSocket);
			QString response;
			QStringList userList;

			in >> response;
			if (response == RESPONSE_USERS_LIST) {
				in >> userList;
				emit userListResult(userList);
			}
		} else {
			emit userListResult(QStringList());
		}
	}


	void NetworkManager::onConnected() {
		reconnectTimer->stop();
		emit statusMessage("Connected to server.");
		logger->log("Connected to server.");
	}

	void NetworkManager::onDisconnected() {
		reconnectTimer->start(RECONNECT_INTERVAL);
		emit statusMessage("Disconnected from server.");
		logger->log("Disconnected from server. Reconnecting...");
	}

	void NetworkManager::onReconnectToServer() {
		if (host.isEmpty() || tcpSocket->state() != QTcpSocket::UnconnectedState) {
			return;
		}
		onConnectRequest(host, port, true);
	}
}
