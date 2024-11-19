#pragma once

#include <QPushButton>
#include <QLineEdit>

#include "networkmanager.hpp"

namespace SHIZ{
	class ConnectionWidget : public QWidget {
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QLineEdit *hostInput;
			QPushButton *enterButton;

		public:
			ConnectionWidget(NetworkManager* manager, QWidget* parent = nullptr);

		signals:
			void ConnectionSuccessful(const QString &host);

		private slots:
		void onEnterButtonClicked();
	};

}
