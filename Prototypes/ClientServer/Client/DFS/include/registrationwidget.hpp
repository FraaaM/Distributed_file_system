#pragma once

#include <QLineEdit>
#include <QPushButton>

#include "networkmanager.hpp"

namespace SHIZ{
	class RegistrationWidget : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QLineEdit *loginInput;
			QLineEdit *passwordInput;
			QLineEdit *confirmPasswordInput;

			QPushButton *enterButton;
			QPushButton *loginButton;

		public:
			RegistrationWidget(NetworkManager* manager, QWidget* parent = nullptr);

		signals:
			void registrationSuccessful(const QString& login);
			void showLoginWindow();

		private slots:
			void onEnterButtonClicked();
			void onLoginButtonClicked();
	};
}
