#pragma once

#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QListWidget>
#include <QVBoxLayout>

#include "logger.hpp"
#include "followerserver.hpp"

namespace SHIZ{
class FollowerWindow : public QMainWindow {
		Q_OBJECT

		private:
			FollowerServer* server;
			QPushButton* toggleButton;
			QStatusBar* statusBar;
			QLineEdit* portInput;

			QListWidget* replicaList;
			QLineEdit* replicaIpInput;
			QLineEdit* replicaPortInput;
			QPushButton* connectReplicaButton;
			QPushButton* disconnectReplicaButton;

			Logger* logger;
			bool serverRunning;

		public:
			FollowerWindow(Logger* logger, QWidget *parent = nullptr);
			~FollowerWindow();

		private slots:
			void onConnectReplica();
			void onDisconnectReplica();
			void onReplicaDisconnected(const QString& replicaAddress);
			void onToggleServerState();
	};
}
