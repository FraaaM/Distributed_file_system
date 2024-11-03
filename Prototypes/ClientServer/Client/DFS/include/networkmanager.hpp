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
		QString host;

	public:
		NetworkManager(QObject* parent = nullptr);
		~NetworkManager();

		void setHost(const QString& initialhost);
		QString getHost();
		bool sendConnectionRequest (const QString& host);
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
