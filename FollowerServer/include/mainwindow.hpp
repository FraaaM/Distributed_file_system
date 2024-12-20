#pragma once

#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>

#include "logger.hpp"
#include "mainserver.hpp"

namespace SHIZ {
	class MainWindow : public QMainWindow {
		Q_OBJECT

		private:
			MainServer* server;
			QTcpSocket* mainServerSocket;

			QLineEdit* mainServerIpInput;
			QLineEdit* mainServerPortInput;
			QPushButton* connectToMainButton;
			QTimer* heartbeatTimer;

			QPushButton* toggleButton;
			QStatusBar* statusBar;
			QLineEdit* portInput;

			QListWidget* replicaList;
			QLineEdit* replicaIpInput;
			QLineEdit* replicaPortInput;
			QPushButton* connectReplicaButton;
			QPushButton* disconnectReplicaButton;

			bool serverRunning;
			bool coonectedToMainServer;

			Logger* logger;

		public:
			MainWindow(Logger* logger, QWidget *parent = nullptr);
			~MainWindow();

		private:
			void setInterfaceEnabled(bool enabled);
			void transitionToIndependentMode();

		private slots:
			void onConnectMainServer();
			void onConnectReplica();
			void onDisconnectMainServer();
			void onDisconnectReplica();
			void onReplicaDisconnected(const QString& replicaAddress);
			void onSendHeartbeat();
			void onToggleServerState();
	};
}
