#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

namespace SHIZ {
	class NetworkManager : public QObject {
		Q_OBJECT

	private:
		QTcpSocket* tcpSocket;
		QTimer* reconnectTimer;

	public:
		NetworkManager(QObject* parent = nullptr);
		~NetworkManager();

		bool downloadFile(const QString& fileName);
		QStringList requestFileList();
		bool sendLoginRequest(const QString& login, const QString& password);
		bool sendRegistrationRequest(const QString& login, const QString& password);
		bool uploadFile(const QString& filePath, const QString& owner);

	private slots:
		void onConnected();
		void onDisconnected();
		void onReconnectToServer();
	};
}
