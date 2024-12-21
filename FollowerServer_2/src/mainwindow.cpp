#include <QApplication>

#include "macros.hpp"
#include "mainwindow.hpp"

namespace SHIZ {
	MainWindow::MainWindow(Logger* logger, QWidget *parent)
		: QMainWindow(parent),
		server(new MainServer(logger, this)),
		logger(logger),
		serverRunning(false),
		coonectedToMainServer(false)
	{
		QWidget* centralWidget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(centralWidget);

		this->setWindowIcon(QIcon(":images/FollowerServer.png"));

		mainServerSocket = new QTcpSocket(this);

		mainServerIpInput = new QLineEdit(this);
		mainServerIpInput->setPlaceholderText("Main Server IP");
		mainServerIpInput->setText("127.0.0.1");
		layout->addWidget(mainServerIpInput);

		mainServerPortInput = new QLineEdit(this);
		mainServerPortInput->setPlaceholderText("Main Server Port");
		mainServerPortInput->setText("1234");
		layout->addWidget(mainServerPortInput);

		connectToMainButton = new QPushButton("Connect to Main Server", this);
		layout->addWidget(connectToMainButton);
		connect(connectToMainButton, &QPushButton::clicked, this, &MainWindow::onConnectMainServer);

		heartbeatTimer = new QTimer(this);
		heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);
		connect(heartbeatTimer, &QTimer::timeout, this, &MainWindow::onSendHeartbeat);


		portInput = new QLineEdit(this);
		portInput->setPlaceholderText("Server Port");
		portInput->setText("1234");
		layout->addWidget(portInput);

		toggleButton = new QPushButton("Start server", this);
		layout->addWidget(toggleButton);
		connect(toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleServerState);


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
		connect(connectReplicaButton, &QPushButton::clicked, this, &MainWindow::onConnectReplica);

		disconnectReplicaButton = new QPushButton("Disconnect from Replica", this);
		layout->addWidget(disconnectReplicaButton);
		connect(disconnectReplicaButton, &QPushButton::clicked, this, &MainWindow::onDisconnectReplica);

		connect(server, &MainServer::replicaDisconnected, this, &MainWindow::onReplicaDisconnected);


		statusBar = new QStatusBar(this);
		setStatusBar(statusBar);
		statusBar->showMessage("Not connected to Main Server.");

		setInterfaceEnabled(false);

		setCentralWidget(centralWidget);
	}

	MainWindow::~MainWindow() {
		if (serverRunning) {
			server->close();
			logger->log("Server stopped before exiting.");
		}
		delete server;

		if (mainServerSocket->isOpen()) {
			mainServerSocket->disconnectFromHost();
		}
		delete mainServerSocket;
	}


	void MainWindow::setInterfaceEnabled(bool enabled) {
		toggleButton->setEnabled(enabled);
		replicaIpInput->setEnabled(enabled);
		replicaPortInput->setEnabled(enabled);
		connectReplicaButton->setEnabled(enabled);
		disconnectReplicaButton->setEnabled(enabled);
		replicaList->setEnabled(enabled);
	}

	void MainWindow::transitionToIndependentMode() {
		logger->log("Transitioning to independent mode...");

		heartbeatTimer->stop();
		logger->log("Heartbeat timer stopped.");

		mainServerIpInput->setEnabled(false);
		mainServerPortInput->setEnabled(false);
		connectToMainButton->setEnabled(false);

		onDisconnectMainServer();

		setInterfaceEnabled(true);

		logger->log("Attempting to connect to replicas from replicaList.");
		for (int i = 0; i < replicaList->count(); ++i) {
			QListWidgetItem* item = replicaList->item(i);
			QString replicaAddress = item->text();
			QStringList parts = replicaAddress.split(":");

			if (parts.size() == 2) {
				QString replicaIp = parts.at(0);
				bool ok;
				quint16 replicaPort = parts.at(1).toUShort(&ok);

				if (ok && replicaPort > 0) {
					if (server->connectToReplica(replicaIp, replicaPort)) {
						logger->log("Connected to replica: " + replicaAddress);
					} else {
						logger->log("Failed to connect to replica: " + replicaAddress);
					}
				} else {
					logger->log("Invalid replica address format: " + replicaAddress);
				}
			} else {
				logger->log("Invalid replica address: " + replicaAddress);
			}
		}

		logger->log("Starting server...");
		onToggleServerState();

		statusBar->showMessage("Main Server unresponsive. Operating as independent server.");
	}


	void MainWindow::onConnectMainServer() {
		if (coonectedToMainServer) {
			heartbeatTimer->stop();
			onDisconnectMainServer();
			coonectedToMainServer = false;
			mainServerIpInput->setEnabled(true);
			mainServerPortInput->setEnabled(true);
			connectToMainButton->setText("Connect to Main Server");
			return;
		}

		QString mainServerIp = mainServerIpInput->text();
		bool ok;
		quint16 mainServerPort = mainServerPortInput->text().toUShort(&ok);

		if (mainServerIp.isEmpty() || !ok || mainServerPort == 0) {
			statusBar->showMessage("Invalid Main Server IP or port.");
			logger->log("Invalid Main Server IP or port entered.");
			return;
		}

		mainServerSocket->connectToHost(mainServerIp, mainServerPort);

		if (mainServerSocket->waitForConnected(RESPONSE_TIMEOUT)) {
			statusBar->showMessage("Connected to Main Server at " + mainServerIp + ":" + QString::number(mainServerPort));
			logger->log("Connected to Main Server at " + mainServerIp + ":" + QString::number(mainServerPort));

			QDataStream out(mainServerSocket);
			out << QString(FOLLOWER_SERVER);
			mainServerSocket->flush();

			if (mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
				connect(mainServerSocket, &QTcpSocket::readyRead, this, &MainWindow::handleMainServerData);}
			// QDataStream in(mainServerSocket);
			// QString initialMessage;
			// in >> initialMessage;

			// if (initialMessage == FOLLOWER_SERVER) {
			// 	connect(mainServerSocket, &QTcpSocket::readyRead, this, &MainWindow::handleMainServerData);
			// 	//connect(mainServerSocket, &QTcpSocket::disconnected, this, &MainWindow::handleFollowerDisconnected);
			// 	logger->log("New follower connection established.");

			heartbeatTimer->start();
			coonectedToMainServer = true;
			mainServerIpInput->setEnabled(false);
			mainServerPortInput->setEnabled(false);
			connectToMainButton->setText("Disconnect from Main Server");
		} else {
			statusBar->showMessage("Failed to connect to Main Server.");
			logger->log("Failed to connect to Main Server at " + mainServerIp + ":" + QString::number(mainServerPort));
		}
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

	void MainWindow::onDisconnectMainServer() {
		logger->log("Disconnecting from Main Server...");
		mainServerSocket->disconnectFromHost();

		if (mainServerSocket->state() != QTcpSocket::UnconnectedState) {
			mainServerSocket->waitForDisconnected(RESPONSE_TIMEOUT);
		}

		statusBar->showMessage("Disconnected from Main Server.");
		logger->log("Disconnected from Main Server.");
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
		statusBar->showMessage("Replica disconnected: " + host + ":" + QString::number(port));
	}

	void MainWindow::onReplicaDisconnected(const QString& replicaAddress) {
		QList<QListWidgetItem*> items = replicaList->findItems(replicaAddress, Qt::MatchExactly);
		for (QListWidgetItem* item : items) {
			delete replicaList->takeItem(replicaList->row(item));
		}
		statusBar->showMessage("Replica disconnected: " + replicaAddress);
		logger->log("Replica removed from list: " + replicaAddress);
	}


	void MainWindow::processGetReplicaList() {
		if (!coonectedToMainServer) {
			logger->log("Cannot send heartbeat. Not connected to Main Server.");
			statusBar->showMessage("Heartbeat failed: Not connected to Main Server.");
			return;
		}

		QDataStream out(mainServerSocket);
		out << QString(COMMAND_GET_REPLICA_LIST);
		mainServerSocket->flush();

		if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			logger->log("No response from Main Server after receiving command to load replica`s list.");
			statusBar->showMessage("Heartbeat failed: No response after command to load replica`s list.");
			return;
		}

		QDataStream in(mainServerSocket);
		QVector<QPair<QString, quint16>> replicaListData;
		in >> replicaListData;

		replicaList->clear();
		for (const auto& pair : replicaListData) {
			replicaList->addItem(pair.first + ":" + QString::number(pair.second));
		}

		out << QString(RESPONSE_REPLICA_LIST_RECEIVED);
		mainServerSocket->flush();

		if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			logger->log("No response from Main Server after receiving replicas.");
			statusBar->showMessage("Heartbeat failed: No response after receiving replicas.");
			return;
		}

	}

	void MainWindow::processGetDataBase() {
		//onSendHeartbeatQ();
		if (!coonectedToMainServer) {
			logger->log("Cannot send heartbeat. Not connected to Main Server.");
			statusBar->showMessage("Heartbeat failed: Not connected to Main Server.");
			return;
		}

		QDataStream out(mainServerSocket);
		out << QString(COMMAND_GET_DATABASE);
		mainServerSocket->flush();

		if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			logger->log("No response from Main Server after receiving command to load database.");
			statusBar->showMessage("Heartbeat failed: No response after receiving command to load database.");
			return;
		}

		QDataStream in(mainServerSocket);
		QString command;
		qint64 fileSize;
		in >> command >> fileSize;

		if (command != COMMAND_FILE_TRANSFER) {
			logger->log("Unexpected command: U" + command + "U");
			return;
		}

		logger->log("Receiving database file: " + QString(DATABASE_NAME) + " Size: " + QString::number(fileSize));

		out << QString(RESPONSE_READY_FOR_DATA);
		mainServerSocket->flush();

		QString dbFilePath = QApplication::applicationDirPath() + "/" + DATABASE_NAME;
		QFile dbFile(dbFilePath);
		if (!dbFile.open(QIODevice::WriteOnly)) {
			logger->log("Cannot open file for writing: " + dbFilePath);
			return;
		}

		const qint64 chunkSize = CHUNK_SIZE;
		qint64 totalReceived = 0;

		while (totalReceived < fileSize) {
			if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
				logger->log("No data received from Main Server during file transfer.");
				dbFile.close();
				return;
			}

			QByteArray buffer;
			in >> buffer;
			if (buffer.isEmpty()) {
				logger->log("Received empty chunk.");
				break;
			}

			dbFile.write(buffer);
			totalReceived += buffer.size();

			out << QString(RESPONSE_CHUNK_RECEIVED);
			mainServerSocket->flush();
		}

		dbFile.close();

		if (totalReceived == fileSize) {
			logger->log("Database file received and saved successfully: " + dbFilePath);
		} else {
			logger->log("Database file transfer incomplete.");
		}

		logger->log("Heartbeat succeed.");
		statusBar->showMessage("Heartbeat succeed.");

	}

	void MainWindow::onSendHeartbeat() {
		if (!coonectedToMainServer) {
			logger->log("Cannot send heartbeat. Not connected to Main Server.");
			statusBar->showMessage("Heartbeat failed: Not connected to Main Server.");
			return;
		}

		QDataStream out(mainServerSocket);
		out << QString(COMMAND_MAIN_HEARTBEAT);
		mainServerSocket->flush();

		// if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
		// 	logger->log("No response from Main Server for heartbeat.");
		// 	statusBar->showMessage("Heartbeat failed: No response.");
		// 	transitionToIndependentMode();
		// 	return;
		// }

		// QDataStream in(mainServerSocket);
		// QString command;
		// in >> command;

		// if (command != COMMAND_MAIN_HEARTBEAT) {
		// 	logger->log("Unexpected command: U" + command + "U");
		// 	return;
		// }
	}

	void MainWindow::onSendHeartbeatQ() {
		if (!coonectedToMainServer) {
			logger->log("Cannot send heartbeat. Not connected to Main Server.");
			statusBar->showMessage("Heartbeat failed: Not connected to Main Server.");
			return;
		}

		QDataStream out(mainServerSocket);
		out << QString(COMMAND_FOLLOWER_SYNC);
		mainServerSocket->flush();

		if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			logger->log("No response from Main Server for heartbeat11111111111.");
			statusBar->showMessage("Heartbeat failed: No response.");
			transitionToIndependentMode();
			return;
		}

		QDataStream in(mainServerSocket);

		QVector<QPair<QString, quint16>> replicaListData;
		in >> replicaListData;

		replicaList->clear();
		for (const auto& pair : replicaListData) {
			replicaList->addItem(pair.first + ":" + QString::number(pair.second));
		}

		out << QString(RESPONSE_REPLICA_LIST_RECEIVED);
		mainServerSocket->flush();

		if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
			logger->log("No response from Main Server after receiving replicas.");
			statusBar->showMessage("Heartbeat failed: No response after receiving replicas.");
			return;
		}

		QString command;
		qint64 fileSize;
		in >> command >> fileSize;

		if (command != COMMAND_FILE_TRANSFER) {
			logger->log("Unexpected command: U" + command + "U");
			return;
		}

		logger->log("Receiving database file: " + QString(DATABASE_NAME) + " Size: " + QString::number(fileSize));

		out << QString(RESPONSE_READY_FOR_DATA);
		mainServerSocket->flush();

		QString dbFilePath = QApplication::applicationDirPath() + "/" + DATABASE_NAME;
		QFile dbFile(dbFilePath);
		if (!dbFile.open(QIODevice::WriteOnly)) {
			logger->log("Cannot open file for writing: " + dbFilePath);
			return;
		}

		const qint64 chunkSize = CHUNK_SIZE;
		qint64 totalReceived = 0;

		while (totalReceived < fileSize) {
			if (!mainServerSocket->waitForReadyRead(RESPONSE_TIMEOUT)) {
				logger->log("No data received from Main Server during file transfer.");
				dbFile.close();
				return;
			}

			QByteArray buffer;
			in >> buffer;
			if (buffer.isEmpty()) {
				logger->log("Received empty chunk.");
				break;
			}

			dbFile.write(buffer);
			totalReceived += buffer.size();

			out << QString(RESPONSE_CHUNK_RECEIVED);
			mainServerSocket->flush();
		}

		dbFile.close();

		if (totalReceived == fileSize) {
			logger->log("Database file received and saved successfully: " + dbFilePath);
		} else {
			logger->log("Database file transfer incomplete.");
		}

		logger->log("Heartbeat succeed.");
		statusBar->showMessage("Heartbeat succeed.");
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

	void MainWindow::handleMainServerData(){
		// QTcpSocket* mainServerSocket = qobject_cast<QTcpSocket*>(sender());
		if (!mainServerSocket)
			logger->log("No mainServerSocket .");
			return;

		if (!mainServerSocket->bytesAvailable() == 0) {
			logger->log("No response from Main Server for heartbeat.");
			statusBar->showMessage("Heartbeat failed: No response.");
			transitionToIndependentMode();
			return;
		}

		QDataStream in(mainServerSocket);
		QString command;
		in >> command;

		logger->log("Received command from MainServer: " + command);

		// if (command != COMMAND_MAIN_HEARTBEAT) {
		// 	logger->log("Unexpected command: U" + command + "U");
		// 	return;
		// }

		if (command == COMMAND_MAIN_HEARTBEAT) {
			//processFollowerReceiveHeartbeatRequest(mainServerSocket);
			logger->log("command heartbeat to Follower");
		}

		else if (command == COMMAND_SEND_REPLICA_LIST) {
			processGetReplicaList();
		}

		else if (command == COMMAND_SEND_DATABASE) {
			processGetDataBase();
		}

		else {
			logger->log("Unknown command from follower: " + command);
		}
	}


}
