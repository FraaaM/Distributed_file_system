#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
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

			Logger* logger;
			bool serverRunning;

		public:
			MainWindow(Logger* logger, QWidget *parent = nullptr);
			~MainWindow();

		private slots:
			void toggleServerState();
	};
}
