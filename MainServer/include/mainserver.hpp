#pragma once

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

namespace SHIZ{
	class MainServer : public QTcpServer{
			Q_OBJECT

		public:
			MainServer(QObject *parent = nullptr);

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private:
			void processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts);

		private slots:
			void handleClientData();
			void handleClientDisconnected();
	};
}
