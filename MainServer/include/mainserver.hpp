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
			QList<QTcpSocket*> activeClients;
			QVector<QTcpSocket*> replicaSockets;
			Logger* logger;

		public:
			MainServer(Logger* logger, QObject *parent = nullptr);
			~MainServer();

			void closeServer();
			bool connectToHost(const QString& host, quint16 port);
			void disconnectFromHost(const QString& host, quint16 port);

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private:
			bool distributeFileToReplicas(const QString& fileName, const QByteArray& fileData, const QString& uploadDate);
			void processDeleteFileRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processFileListRequest(QTcpSocket* clientSocket);
			void processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processReplicaConnection(QTcpSocket* replicaSocket);
			void processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize);
			bool tryDownloadFromReplica(QTcpSocket* clientSocket, const QString& fileName, const QString& address, quint16 port);

		signals:
			void replicaDisconnected(const QString& replicaAddress);
			void statusMessage(const QString& message);

		private slots:
			void handleClientData();
			void handleClientDisconnected();
			void onReplicaConnected();
			void onReplicaDisconnected();
	};
}
