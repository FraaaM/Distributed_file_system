#pragma once

#include <QStackedWidget>

#include "connectionwidget.hpp"
#include "loginwidget.hpp"
#include "mainwidget.hpp"
#include "networkmanager.hpp"
#include "registrationwidget.hpp"
#include "adminwidget.hpp"

namespace SHIZ{
	class Window: public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QStackedWidget* stackedWidget;

			ConnectionWidget* connectionWidget;
			LoginWidget* loginWidget;
			MainWidget* mainWidget;
			RegistrationWidget* registrationWidget;
            AdminWidget* adminWidget;

			Logger* logger;

		public:
			Window(Logger* logger, NetworkManager* networkManager, QWidget* parent = nullptr);

		private slots:
			void onConnectionSuccessful (const QString &host, quint16 port);
            void onUserLoginSuccessful(const QString& login);
             void onAdminLoginSuccessful(const QString& login);
			void onRegistrationSuccessful(const QString& login);
			void onSwitchToConnectionWindow();
			void onSwitchToLoginWindow();
			void onSwitchToMainWindow();
            void onSwitchToAdminWindow();
			void onSwitchToRegistrationWindow();
	};
}
