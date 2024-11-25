#pragma once

#include <QTcpServer>
#include <QTcpSocket>

#include "logger.hpp"

namespace SHIZ{
	class ReplicaServer : public QTcpServer {
		Q_OBJECT

		private:
			Logger* logger;

		public:
			ReplicaServer(Logger* logger, QObject* parent = nullptr);

		protected:
			void incomingConnection(qintptr socketDescriptor) override;

		private slots:
			void handleMainServerData();
			void handleMainServerDisconnected();
	};
}
