#pragma once

#include <QTcpServer>
#include <QTcpSocket>

#include "logger.hpp"

namespace SHIZ{
	class ReplicaServer : public QTcpServer {
		Q_OBJECT

		private:
			QTcpSocket* mainServer;
			Logger* logger;

		public:
			ReplicaServer(Logger* logger, QObject* parent = nullptr);
			~ReplicaServer();

			void closeServer();

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private slots:
			void handleMainServerData();
			void handleMainServerDisconnected();
	};
}
