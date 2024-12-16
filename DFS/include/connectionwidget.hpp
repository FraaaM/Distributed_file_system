#pragma once

#include <QPushButton>
#include <QLineEdit>

#include "networkmanager.hpp"

namespace SHIZ {
	class ConnectionWidget : public QWidget {
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QLineEdit* hostInput;
			QLineEdit* portInput;

			QPushButton* connectButton;
			QPushButton* quitButton;

			Logger* logger;

		public:
			ConnectionWidget(Logger* logger, NetworkManager* manager, QWidget* parent = nullptr);

		signals:
			void connectRequest(const QString &host, quint16 port, bool isReconnrection = false);
			void connectSuccessful(const QString &host, quint16 port);

		public slots:
			void onConnectResult(bool success);

		private slots:
			void onConnectButtonClicked();
	};
}
