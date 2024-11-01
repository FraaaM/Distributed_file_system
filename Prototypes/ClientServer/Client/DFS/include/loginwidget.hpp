#pragma once

#include <QLineEdit>
#include <QPushButton>

#include "networkmanager.hpp"

namespace SHIZ{
	class LoginWidget  : public QWidget{
		Q_OBJECT

		private:
			NetworkManager* networkManager;

			QLineEdit *loginInput;
			QLineEdit *passwordInput;

			QPushButton *enterButton;
			QPushButton *registrationButton;

		public:
			LoginWidget(NetworkManager* manager, QWidget* parent = nullptr);
            void ClearData();

		signals:
			void loginSuccessful(const QString& login);
			void showRegistrationWindow();

		private slots:
			void onEnterButtonClicked();
			void onRegisterButtonClicked();
	};
}
