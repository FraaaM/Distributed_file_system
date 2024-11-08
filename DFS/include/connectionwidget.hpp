#pragma once

#include <QPushButton>
#include <QLineEdit>

#include "networkmanager.hpp"

namespace SHIZ{
	class ConnectionWidget : public QWidget {
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QLineEdit* hostInput;
			QLineEdit* portInput;

			QPushButton* enterButton;
			QPushButton* quitButton;

		public:
			ConnectionWidget(NetworkManager* manager, QWidget* parent = nullptr);

		signals:
			void requestConnection(const QString &host, quint16 port);
			void ConnectionSuccessful(const QString &host, quint16 port);

		private slots:
			void onEnterButtonClicked();
	};

}
