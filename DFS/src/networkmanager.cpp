#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "clientmacros.hpp"
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
		disconnectFromHost();
	}


	bool NetworkManager::connectToHost(const QString& host, quint16 port) {
		this->host = host;
		this->port = port;
		tcpSocket->connectToHost(host, port);
		if (tcpSocket->waitForConnected(3000)) {
			logger->log("Connected to main server.");
			QDataStream out(tcpSocket);
			out << QString(CLIENT);
			tcpSocket->flush();
			return true;
		}
		return false;
	}

	void NetworkManager::disconnectFromHost() {
		if (tcpSocket->isOpen()) {
			tcpSocket->disconnectFromHost();
		}
		reconnectTimer->stop();
		logger->log("Disconnected from server and stopped reconnect attempts.");
	}

	bool NetworkManager::deleteFile(const QString& fileName) {
		emit statusMessage("Requesting file deletion...");
		logger->log("Requesting file deletion...");


		QDataStream out(tcpSocket);
		out << QString(COMMAND_DELETE) << fileName;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

			bool success = response == RESPONSE_DELETE_SUCCESS;
			emit statusMessage(success ? "File deleted successfully." : "File deletion failed.");
			logger->log(success ? "File deleted successfully." : "File deletion failed.");
			return success;
		}
		emit statusMessage("Server response timed out.");
		logger->log("Server response timed out.");
		return false;
	}

	bool NetworkManager::downloadFile(const QString& filePath) {
		QString fileName = QFileInfo(filePath).fileName();
		emit statusMessage("Requesting file download: " + fileName);
		logger->log("Requesting file download: " + fileName);

		QDataStream out(tcpSocket);
		out << QString(COMMAND_DOWNLOAD) << fileName;
		tcpSocket->flush();

		if (!tcpSocket->waitForReadyRead(3000)) {
			emit statusMessage("Server did not respond in time for download.");
			logger->log("Server did not respond in time for download.");
			return false;
		}

		QDataStream in(tcpSocket);
		QString response;
		qint64 fileSize = 0;

		in >> response;
		if (response != RESPONSE_DOWNLOAD_READY) {
			emit statusMessage("Server not ready for download.");
			logger->log("Server not ready for download.");
			return false;
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
			return false;
		}

		qint64 totalReceived = 0;
		const qint64 chunkSize = 1024;
		QByteArray chunk;

		while (totalReceived < fileSize) {
			if (!tcpSocket->waitForReadyRead(3000)) {
				emit statusMessage("No response from server during download.");
				logger->log("No response from server during download.");
				file.close();
				return false;
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
		return true;
	}

    bool NetworkManager::deleteUser(const QString& userName){
        emit statusMessage("Requesting user deletion...");
        logger->log("Requesting user deletion...");


        QDataStream out(tcpSocket);
        out << QString(COMMAND_DELETE_USER) << userName;
        tcpSocket->flush();

        if (tcpSocket->waitForReadyRead(3000)) {
            QDataStream in(tcpSocket);
            QString response;
            in >> response;

            bool success = response == RESPONSE_DELETE_SUCCESS;
            emit statusMessage(success ? "User deleted successfully." : "User deletion failed.");
            logger->log(success ? "User deleted successfully." : "User deletion failed.");
            return success;
        }
        emit statusMessage("Server response timed out.");
        logger->log("Server response timed out.");
        return false;
    }

    bool NetworkManager::updateUser(const QString &userName, const QString &key, const QString &value){
        emit statusMessage("Requesting user updating...");
        logger->log("Requesting user updating...");


        QDataStream out(tcpSocket);
        out << QString(COMMAND_UPDATE_USER) << userName << key << value;
        tcpSocket->flush();

        if (tcpSocket->waitForReadyRead(3000)) {
            QDataStream in(tcpSocket);
            QString response;
            in >> response;

            bool success = response == RESPONSE_UPDATE_USER_SUCCESS;
            emit statusMessage(success ? "User updated successfully." : "User update failed.");
            logger->log(success ? "User updated successfully." : "User update failed.");
            return success;
        }
        emit statusMessage("Server response timed out.");
        logger->log("Server response timed out.");
        return false;
    }

    bool NetworkManager::addUser(const QString& userName){
        return true;
    }

    QStringList NetworkManager::requestFileList(const QString &userName){
		QDataStream out(tcpSocket);
        out << QString(COMMAND_GET_FILES) << userName;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			QStringList fileList;

			in >> response;
			if (response == RESPONSE_FILES_LIST) {
				in >> fileList;
				return fileList;
			}
		}
		return QStringList();
	}

    QString NetworkManager::getUserInfo(const QString &userName){
        QDataStream out(tcpSocket);
        out << QString(COMMAND_GET_USER_INFO) << userName;
        tcpSocket->flush();

        if (tcpSocket->waitForReadyRead(3000)) {
            QDataStream in(tcpSocket);
            QString response;
            QString userInfo;

            in >> response;
            if (response == RESPONSE_USER_INFO) {
                in >> userInfo;
                return userInfo;
            }
        }
        return QString();
    }

    QString NetworkManager::getFileInfo(const QString &fileName){
        QDataStream out(tcpSocket);
        out << QString(COMMAND_GET_FILE_INFO) << fileName;
        tcpSocket->flush();

        if (tcpSocket->waitForReadyRead(3000)) {
            QDataStream in(tcpSocket);
            QString response;
            QString groupFile;

            in >> response;
            if (response == RESPONSE_FILE_INFO) {
                in >> groupFile;
                return groupFile;
            }
        }
        return QString();
    }

    QStringList NetworkManager::requestUserList(){
        QDataStream out(tcpSocket);
        out << QString(COMMAND_GET_USERS);
        tcpSocket->flush();

        if (tcpSocket->waitForReadyRead(3000)) {
            QDataStream in(tcpSocket);
            QString response;
            QStringList userList;

            in >> response;
            if (response == RESPONSE_USERS_LIST) {
                in >> userList;
                return userList;
            }
        }
        return QStringList();
    }
    std::string NetworkManager::sendLoginRequest(const QString& login, const QString& password) {
		QDataStream out(tcpSocket);
		out << QString(COMMAND_LOGIN) << login << password;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

            if (response == RESPONSE_LOGIN_USER_SUCCESS) {
                return "user";
            }else if(response == RESPONSE_LOGIN_ADMIN_SUCCESS){
                return "admin";
            }else {
				QMessageBox::warning(nullptr, "Login error", "Incorrect username or password.");
			}
		} else {
			QMessageBox::warning(nullptr, "Network error", "Failed to connect to the server.");
		}
        return "fail";
	}

	bool NetworkManager::sendRegistrationRequest(const QString& login, const QString& password, const QString& confirmPassword) {
		QDataStream out(tcpSocket);
		out << QString(COMMAND_REGISTER) << login << password;
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			QDataStream in(tcpSocket);
			QString response;
			in >> response;

			if (response == RESPONSE_REGISTER_SUCCESS) {
				QMessageBox::information(nullptr, "Registration", "Registration was successful.");
				return true;
			} else if (response == RESPONSE_REGISTER_USER_EXISTS) {
				QMessageBox::warning(nullptr, "Registration error", "A user with this username already exists.");
			} else {
				QMessageBox::warning(nullptr, "Registration error", "Such a user already exists or an error has occurred.");
			}
		} else {
			QMessageBox::warning(nullptr, "Network error", "Failed to connect to the server.");
		}
		return false;
	}

	void NetworkManager::setHostAndPort(const QString& host, quint16 port) {
		this->host = host;
		this->port = port;
	}

	bool NetworkManager::uploadFile(const QString& filePath, const QString& owner) {
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)) {
			emit statusMessage("Cannot open file for reading.");
			logger->log("Cannot open file for reading.");
			return false;
		}

		QString fileName = QFileInfo(filePath).fileName();
		qint64 fileSize = file.size();
		emit statusMessage("Uploading file: " + fileName);
		logger->log("Uploading file:" + fileName + "Owner:" + owner + "Size:" + QString::number(fileSize));

		QDataStream out(tcpSocket);
		out << QString(COMMAND_UPLOAD) << fileName << owner << fileSize;
		tcpSocket->flush();

		if (!tcpSocket->waitForReadyRead(3000)) {
			emit statusMessage("Server did not respond in time for upload.");
			logger->log("Server did not respond in time for upload.");
			return false;
		}

		QDataStream in(tcpSocket);
		QString response;
		in >> response;

		if (response != RESPONSE_READY_FOR_DATA) {
			emit statusMessage("Server is not ready for data.");
			logger->log("Server is not ready for data.");
			return false;
		}

		const qint64 chunkSize = 1024;
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

			if (!tcpSocket->waitForReadyRead(3000)) {
				emit statusMessage("No response from server during upload.");
				logger->log("No response from server after sending chunk.");
				return false;
			}

			in >> response;
			if (response != RESPONSE_CHUNK_RECEIVED) {
				emit statusMessage("Server did not acknowledge chunk.");
				logger->log("Server did not acknowledge chunk.");
				return false;
			}
		}

		out << QByteArray();
		tcpSocket->flush();

		if (tcpSocket->waitForReadyRead(3000)) {
			in >> response;
			bool success = response == RESPONSE_UPLOAD_SUCCESS;
			emit statusMessage(success ? "File uploaded successfully." : "File upload failed.");
			logger->log(success ? "File uploaded successfully." : "File upload failed.");
			return success;
		}

		return false;
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
		connectToHost(host, port);
	}
}
