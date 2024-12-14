#pragma once

#include <QStackedWidget>

#include "adminwidget.hpp"
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

			QStackedWidget* stackedWidget;

			AdminWidget* adminWidget;
			ConnectionWidget* connectionWidget;
			LoginWidget* loginWidget;
			MainWidget* mainWidget;
			RegistrationWidget* registrationWidget;

			Logger* logger;

		public:
			Window(Logger* logger, NetworkManager* networkManager, QWidget* parent = nullptr);

		private slots:
			void onAdminLoginSuccessful(const QString& login);
			void onConnectionSuccessful (const QString &host, quint16 port);
			void onRegistrationSuccessful(const QString& login);
			void onSwitchToAdminWindow();
			void onSwitchToConnectionWindow();
			void onSwitchToLoginWindow();
			void onSwitchToLoginWindowWithBanned();
			void onSwitchToMainWindow();
			void onSwitchToRegistrationWindow();
			void onUserLoginSuccessful(const QString& login);
	};
}
