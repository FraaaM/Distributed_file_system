#pragma once

#include <QSqlDatabase>
#include <QTcpServer>
#include <QTcpSocket>
#include <QPair>

#include "logger.hpp"

namespace SHIZ{
	class MainServer : public QTcpServer{
			Q_OBJECT

		private:
			QSqlDatabase dataBase;
			QList<QTcpSocket*> activeClients;
			QVector<QTcpSocket*> replicaSockets;
			QTcpSocket* ptrFollowerSocket;
			Logger* logger;

		public:
			MainServer(Logger* logger, QObject *parent = nullptr);
			~MainServer();

			bool isFollowerConnected() const;
			QPair<QString, quint16> getFollowerIpPort();
			void closeServer();
			bool connectToFollower(const QString& host, quint16 port); ///////////////////////////////
			bool disconnectFromFollower(const QString& host, quint16 port); //!!!!!!!!!!!!!!

			bool connectToReplica(const QString& host, quint16 port);
			void disconnectFromReplica(const QString& host, quint16 port);

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private:
			void receiveReplicaSockets();

			bool distributeFileToReplicas(const QString& fileName, const QByteArray& fileData, const QString& uploadDate);
            void processDeleteFileRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processDeleteUserRequest(QTcpSocket* clientSocket, const QString& userName);
			void processDownloadRequest(QTcpSocket* clientSocket, const QString& fileName);
            void processFileListRequest(QTcpSocket* clientSocket, const QString& userName);
			void processGetFileInfoRequest(QTcpSocket* clientSocket, const QString& fileName);
			void processGetUserInfoRequest(QTcpSocket* clientSocket, const QString& userName);
			void processLoginRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processRegistrationRequest(QTcpSocket* clientSocket, const QStringList& parts);
			void processReplicaConnection(QTcpSocket* replicaSocket);
			void processUpdateUserRequest(QTcpSocket* clientSocket, const QString& userName, const QString& key, const QString& value);
			void processUploadRequest(QTcpSocket* clientSocket, const QString& fileName, const QString& owner, qint64 fileSize);
			void processUserListRequest(QTcpSocket* clientSocket);
			bool tryDownloadFromReplica(QTcpSocket* clientSocket, const QString& fileName, const QString& address, quint16 port);

		signals:
			void followerDisconnected(const QString& followerAddress); //////////////////&
			void replicaDisconnected(const QString& replicaAddress);
			void statusMessage(const QString& message);

		private slots:
			void handleClientData();
			void handleClientDisconnected();
			void onReplicaConnected();
			void onReplicaDisconnected();
			void onFollowerConnected();      /////////////////////////&&
			void onFollowerDisconnected(); //////////////////////////!!!!!!!!!!
	};
}
