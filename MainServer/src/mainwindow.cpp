#include "mainwindow.hpp"

namespace SHIZ{
	MainWindow::MainWindow(Logger* logger, QWidget *parent)
		: QMainWindow(parent),
		server(new MainServer(logger, this)),
		logger(logger),
		serverRunning(false)
	{
		QWidget* centralWidget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(centralWidget);

		portInput = new QLineEdit(this);
		portInput->setPlaceholderText("Main Server Port");
		portInput->setText("1234");
		layout->addWidget(portInput);

		toggleButton = new QPushButton("Start server", this);
		layout->addWidget(toggleButton);

		followerIpInput = new QLineEdit(this);
		followerIpInput->setPlaceholderText("Follower IP");
		followerIpInput->setText("127.0.0.1");
		layout->addWidget(followerIpInput);

		followerPortInput = new QLineEdit(this);
		followerPortInput->setPlaceholderText("Follower Port");
		followerPortInput->setText("123456");
		layout->addWidget(followerPortInput);

		connectFollowerButton = new QPushButton("Connect to Follower", this);
		layout->addWidget(connectFollowerButton);

		disconnectFollowerButton = new QPushButton("Disconnect from Follower", this);
		layout->addWidget(disconnectFollowerButton);


		replicaList = new QListWidget(this);
		layout->addWidget(replicaList);

		replicaIpInput = new QLineEdit(this);
		replicaIpInput->setPlaceholderText("Replica IP");
		replicaIpInput->setText("127.0.0.1");
		layout->addWidget(replicaIpInput);

		replicaPortInput = new QLineEdit(this);
		replicaPortInput->setPlaceholderText("Replica Port");
		replicaPortInput->setText("12345");
		layout->addWidget(replicaPortInput);

		connectReplicaButton = new QPushButton("Connect to Replica", this);
		layout->addWidget(connectReplicaButton);

		disconnectReplicaButton = new QPushButton("Disconnect from Replica", this);
		layout->addWidget(disconnectReplicaButton);

		statusBar = new QStatusBar(this);
		setStatusBar(statusBar);
		statusBar->showMessage("Server is not running.");

		setCentralWidget(centralWidget);

		connect(connectReplicaButton, &QPushButton::clicked, this, &MainWindow::onConnectReplica);
		connect(disconnectReplicaButton, &QPushButton::clicked, this, &MainWindow::onDisconnectReplica);
		connect(server, &MainServer::replicaDisconnected, this, &MainWindow::onReplicaDisconnected);
		connect(toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleServerState);

		connect(connectFollowerButton, &QPushButton::clicked, this, &MainWindow::onConnectFollower);
		connect(disconnectFollowerButton, &QPushButton::clicked, this, MainWindow::onDisconnectFollower);
		connect(server, &MainServer::followerDisconnected, this, &MainWindow::onFollowerDisconnected);
	}

	MainWindow::~MainWindow() {
		if (serverRunning) {
			server->close();
			logger->log("Server stopped before exiting.");
		}
		delete server;
	}


	void MainWindow::onConnectFollower() {
		QString followerIp = followerIpInput->text();
		bool ok;
		quint16 followerPort = followerPortInput->text().toUShort(&ok);

		if (followerIp.isEmpty() || !ok || followerPort == 0) {
			statusBar->showMessage("Invalid follower IP or port.");
			logger->log("Invalid follower IP or port entered.");
			return;
		}

		if (server->connectToFollower(followerIp, followerPort)) {
			statusBar->showMessage("Follower connected: " + followerIp + ":" + QString::number(followerPort));
			logger->log("Connected to follower: " + followerIp + ":" + QString::number(followerPort));
		} else {
			statusBar->showMessage("Failed to connect to follower.");
			logger->log("Failed to connect to follower: " + followerIp + ":" + QString::number(followerPort));
		}
	}

	void MainWindow::onDisconnectFollower() {
		QTcpSocket* followerSocket = qobject_cast<QTcpSocket*>(sender());
		if (!followerSocket) {
			statusBar->showMessage("Invalid follower socket.");
			logger->log("Failed to disconnect: invalid follower socket.");
			return;
		}

		QString followerIp = followerSocket->peerAddress().toString();
		quint16 followerPort = followerSocket->peerPort();

		if (!server->disconnectFromFollower(followerIp, followerPort)) {
			statusBar->showMessage("Failed to disconnect from follower.");
			logger->log("Failed to disconnect from follower: " + followerIp + ":" + QString::number(followerPort));
		}
	}

	void MainWindow::onFollowerDisconnected(const QString& followerAddress) {
		statusBar->showMessage("Follower disconnected: " + followerAddress );
		logger->log("Follower disconnected automatically: " + followerAddress);
	}

	void MainWindow::onConnectReplica() {
		QString replicaIp = replicaIpInput->text();
		bool ok;
		quint16 replicaPort = replicaPortInput->text().toUShort(&ok);

		if (replicaIp.isEmpty() || !ok || replicaPort == 0) {
			statusBar->showMessage("Invalid replica IP or port.");
			logger->log("Invalid replica IP or port entered.");
			return;
		}

		if (server->connectToReplica(replicaIp, replicaPort)) {
			replicaList->addItem(replicaIp + ":" + QString::number(replicaPort));
			statusBar->showMessage("Replica connected: " + replicaIp + ":" + QString::number(replicaPort));
		} else {
			statusBar->showMessage("Failed to connect to replica.");
		}
	}

	void MainWindow::onDisconnectReplica() {
		QListWidgetItem* selectedItem = replicaList->currentItem();
		if (!selectedItem) {
			statusBar->showMessage("No replica selected for disconnection.");
			logger->log("No replica selected for disconnection.");
			return;
		}

		QString replicaAddress = selectedItem->text();
		QStringList parts = replicaAddress.split(":");

		if (parts.size() != 2) {
			statusBar->showMessage("Invalid replica address format.");
			logger->log("Invalid replica address format.");
			return;
		}

		QString host = parts.at(0);
		bool ok;
		quint16 port = parts.at(1).toUShort(&ok);

		if (!ok || port == 0) {
			statusBar->showMessage("Invalid port in replica address.");
			logger->log("Invalid port in replica address.");
			return;
		}

		server->disconnectFromReplica(host, port);
		delete selectedItem;
		//statusBar->showMessage("Replica disconnected: " + host + ":" + QString::number(port));
	}

	void MainWindow::onReplicaDisconnected(const QString& replicaAddress) {
		QList<QListWidgetItem*> items = replicaList->findItems(replicaAddress, Qt::MatchExactly);
		for (QListWidgetItem* item : items) {
			delete replicaList->takeItem(replicaList->row(item));
		}
		statusBar->showMessage("Replica disconnected: " + replicaAddress);
		logger->log("Replica removed from list: " + replicaAddress);
	}

	void MainWindow::onToggleServerState() {
		if (serverRunning) {
			server->closeServer();
			statusBar->showMessage("Server stopped.");
			toggleButton->setText("Start server");
			portInput->setEnabled(true);
			serverRunning = false;
			logger->log("Server stopped.");
		} else {
			bool ok;
			quint16 port = portInput->text().toUShort(&ok);

			if (!ok || port == 0) {
				statusBar->showMessage("Invalid port.");
				logger->log("Invalid port number entered.");
				return;
			}

			if (!server->listen(QHostAddress::Any, port)) {
				statusBar->showMessage("Failed to start server.");
				logger->log("Failed to start server.");
				return;
			}

			statusBar->showMessage(QString("Server started on port %1.").arg(port));
			toggleButton->setText("Stop server");
			logger->log(QString("Server started on port %1.").arg(port));
			portInput->setEnabled(false);
			serverRunning = true;
		}
	}
}
