#include "mainwindow.hpp"

namespace SHIZ{
	MainWindow::MainWindow(Logger* logger, QWidget *parent)
		: QMainWindow(parent),
		replica(new ReplicaServer(logger, this)),
		logger(logger),
		serverRunning(false)
	{
		QWidget* centralWidget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(centralWidget);

		portInput = new QLineEdit(this);
		portInput->setPlaceholderText("Replica Port");
		portInput->setText("12345");
		layout->addWidget(portInput);

		toggleButton = new QPushButton("Start replica", this);
		layout->addWidget(toggleButton);

		statusBar = new QStatusBar(this);
		setStatusBar(statusBar);
		statusBar->showMessage("Replica is not running.");

		setCentralWidget(centralWidget);

		connect(toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleServerState);
	}

	MainWindow::~MainWindow() {
		replica->close();
		delete replica;
		logger->log("Replica stopped before exiting.");
	}

	void MainWindow::onToggleServerState() {
		if (serverRunning) {
			replica->closeServer();
			statusBar->showMessage("Replica stopped.");
			toggleButton->setText("Start replica");
			portInput->setEnabled(true);
			serverRunning = false;
			logger->log("Replica stopped.");
		} else {
			bool ok;
			quint16 port = portInput->text().toUShort(&ok);

			if (!ok || port == 0) {
				statusBar->showMessage("Invalid port.");
				logger->log("Invalid port input.");
				return;
			}

			if (!replica->listen(QHostAddress::Any, port)) {
				statusBar->showMessage("Failed to start replica.");
				logger->log("Failed to start replica.");
				return;
			}

			statusBar->showMessage("Replica started.");
			toggleButton->setText("Stop replica");
			portInput->setEnabled(false);
			serverRunning = true;
			logger->log(QString("Replica started on port %1").arg(port));
		}
	}
}
