#pragma once

#include <QSqlDatabase>
#include <QTcpServer>
#include <QTcpSocket>
#include <QObject>

namespace SHIZ{
	class MainServer : public QTcpServer{
			Q_OBJECT

		private:
			QSqlDatabase dataBase;

		public:
			MainServer(QObject *parent = nullptr);

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private:
			void processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processFileListRequest(QTcpSocket* clientSocket);
			void processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, const QByteArray& fileData, qint64 fileSize);

		private slots:
			void handleClientData();
			void handleClientDisconnected();
	};
}
