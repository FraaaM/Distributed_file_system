#pragma once

#include <QSqlDatabase>
#include <QTcpServer>
#include <QTcpSocket>

#include "logger.hpp"

namespace SHIZ{
	class MainServer : public QTcpServer{
			Q_OBJECT

		private:
			QSqlDatabase dataBase;
			Logger* logger;

		public:
			MainServer(Logger* logger, QObject *parent = nullptr);

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private:
			void processDeleteFileRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processFileListRequest(QTcpSocket* clientSocket);
			void processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize);

		private slots:
			void handleClientData();
			void handleClientDisconnected();
	};
}
