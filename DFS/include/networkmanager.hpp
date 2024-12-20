#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

#include "logger.hpp"

namespace SHIZ {
	class NetworkManager : public QObject {
		Q_OBJECT

	private:
		QTcpSocket* tcpSocket;
		QTimer* reconnectTimer;

		QString host;
		quint16 port;

		Logger* logger;

	public:
		NetworkManager(Logger* logger, QObject* parent = nullptr);
		~NetworkManager();

		void setHostAndPort(const QString& host, quint16 port);

	signals:
		void connectResult(bool success);
		void deleteFileResult(const QString& result);
		void deleteUserResult(bool success);
		void downloadFileResult(const QString& result);
		void listFileResult(const QStringList& fileList);
		void loginResult(const QString& success, const QString& login);
		void registrationResult(bool success, const QString& message);
		void statusMessage(const QString& message);
		void updateUserResult(bool success);
		void uploadFileResult(const QString& result);
		void userListResult(const QStringList& users);

	public slots:
		void onConnectRequest(const QString& host, quint16 port, bool isReconnrection = false);
		void onDeleteFileRequest(const QString& fileName, const QString& userName);
		void onDeleteUserRequest(const QString& userName);
		void onDisconnectRequest();
		void onDownloadFileRequest(const QString& filePath, const QString& userName);
		void onListFileRequest(const QString& userName);
		void onLoginRequest(const QString& login, const QString& password);
		void onRegistrationRequest(const QString& login, const QString& password);
		void onUpdateUserRequest(const QString& userName, const QString& key, const QString& value);
		void onUploadFileRequest(const QString& filePath, const QString& owner);
		void onUserListRequest();

	private slots:
		void onConnected();
		void onDisconnected();
		void onReconnectToServer();
	};
}
