#pragma once

#include <QStackedWidget>

#include "connectionwidget.hpp"
#include "loginwidget.hpp"
#include "mainwidget.hpp"
#include "networkmanager.hpp"
#include "registrationwidget.hpp"

namespace SHIZ{
	class Window: public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QStackedWidget *stackedWidget;

			ConnectionWidget *connectionWidget;
			LoginWidget *loginWidget;
			MainWidget *mainWidget;
			RegistrationWidget *registrationWidget;

		public:
			Window(NetworkManager* networkManager, QWidget* parent = nullptr);

		private slots:
			void onConnectionSuccessful (const QString &host);
			void onLoginSuccessful(const QString& login);
			void onRegistrationSuccessful(const QString& login);
			void onSwitchToLoginWindow();
			void onSwitchToMainWindow();
			void onSwitchToRegistrationWindow();
	};
}
