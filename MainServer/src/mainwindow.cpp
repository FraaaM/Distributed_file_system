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

		toggleButton = new QPushButton("Start Server", this);
		layout->addWidget(toggleButton);

		statusBar = new QStatusBar(this);
		setStatusBar(statusBar);
		statusBar->showMessage("Server is not running.");

		setCentralWidget(centralWidget);

		connect(toggleButton, &QPushButton::clicked, this, &MainWindow::toggleServerState);
	}

	MainWindow::~MainWindow() {
		if (serverRunning) {
			server->close();
			logger->log("Server stopped before exiting.");
		}
		delete server;
	}

	void MainWindow::toggleServerState() {
		if (serverRunning) {
			server->close();
			statusBar->showMessage("Server stopped.");
			toggleButton->setText("Start Server");
			logger->log("Server stopped.");
			serverRunning = false;
		} else {
			if (!server->listen(QHostAddress::Any, 1234)) {
				statusBar->showMessage("Failed to start server.");
				logger->log("Failed to start server.");
				return;
			}
			statusBar->showMessage("Server started on port 1234.");
			toggleButton->setText("Stop Server");
			logger->log("Server started on port 1234.");
			serverRunning = true;
		}
	}
}
