#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace SHIZ{
	class LoginWidget  : public QWidget{
		Q_OBJECT

		private:
			QLineEdit *loginInput;
			QLineEdit *passwordInput;

			QPushButton *enterButton;
			QPushButton *registrationButton;

		public:
			LoginWidget(QWidget* parent = nullptr);

		signals:
			void loginSuccessful();
			void showRegistrationWindow();

		private slots:
			void onEnterButtonClicked();
			void onRegisterButtonClicked();
	};
}
