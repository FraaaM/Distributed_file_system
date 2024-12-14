#pragma once

#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QListWidget>
#include <QVBoxLayout>

#include "logger.hpp"
#include "mainserver.hpp"

namespace SHIZ{
	class MainWindow : public QMainWindow {
		Q_OBJECT

		private:
			MainServer* server;
			QPushButton* toggleButton;
			QStatusBar* statusBar;
			QLineEdit* portInput;

			QLineEdit* followerIpInput;
			QLineEdit* followerPortInput;
			QListWidget* replicaList;
			QLineEdit* replicaIpInput;
			QLineEdit* replicaPortInput;
			QPushButton* connectReplicaButton;
			QPushButton* disconnectReplicaButton;
			QPushButton* connectFollowerButton;
			QPushButton* disconnectFollowerButton;

			Logger* logger;
			bool serverRunning;

		public:
			MainWindow(Logger* logger, QWidget *parent = nullptr);
			~MainWindow();

		private slots:
			void onConnectFollower();///
			void onDisconnectFollower();////
			void onFollowerDisconnected(const QString& followerAddress); ///////
			void onConnectReplica();
			void onDisconnectReplica();
			void onReplicaDisconnected(const QString& replicaAddress);
			void onToggleServerState();
	};
}
