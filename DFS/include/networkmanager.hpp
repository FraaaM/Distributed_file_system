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

		bool connectToHost(const QString& host, quint16 port);
		bool deleteFile(const QString& fileName);
		bool deleteUser(const QString& userName);
		bool downloadFile(const QString& filePath);
		void disconnectFromHost();
		QString getFileInfo(const QString &fileName);
		QString getUserInfo(const QString &userName);
		QStringList requestFileList(const QString &userName);
		QStringList requestUserList();
		QString sendLoginRequest(const QString& login, const QString& password);
		bool sendRegistrationRequest(const QString& login, const QString& password, const QString& confirmPassword);
		void setHostAndPort(const QString& host, quint16 port);
        bool updateUser(const QString& userName, const QString& key, const QString& value);
		bool uploadFile(const QString& filePath, const QString& owner);

	signals:
		void statusMessage(const QString& message);

	private slots:
		void onConnected();
		void onDisconnected();
		void onReconnectToServer();
	};
}
