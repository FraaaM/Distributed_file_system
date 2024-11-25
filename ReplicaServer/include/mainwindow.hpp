#pragma once

#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>

#include "logger.hpp"
#include "replicaserver.hpp"

namespace SHIZ{
	class MainWindow : public QMainWindow {
		Q_OBJECT

		private:
			ReplicaServer* replica;
			QPushButton* toggleButton;
			QStatusBar* statusBar;
			QLineEdit* portInput;

			Logger* logger;
			bool serverRunning;

		public:
			MainWindow(Logger* logger, QWidget *parent = nullptr);
			~MainWindow();

		private slots:
			void onToggleServerState();
	};
}
