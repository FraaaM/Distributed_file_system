#pragma once

#include <QLineEdit>
#include <QPushButton>

#include "networkmanager.hpp"

namespace SHIZ{
	class LoginWidget  : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QLineEdit* loginInput;
			QLineEdit* passwordInput;

			QPushButton* enterButton;
			QPushButton* registrationButton;
			QPushButton* disconnectButton;

			Logger* logger;

		public:
			LoginWidget(Logger* logger, NetworkManager* manager, QWidget* parent = nullptr);

		signals:
			void loginSuccessful(const QString& login);
			void showConnectionWindow();
			void showRegistrationWindow();

		private slots:
			void onDisconnectButtonClicked();
			void onEnterButtonClicked();
			void onRegisterButtonClicked();
	};
}
