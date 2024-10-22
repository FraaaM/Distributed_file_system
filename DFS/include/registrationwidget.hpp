#pragma once

#include <QLineEdit>
#include <QWidget>
#include <QPushButton>

namespace SHIZ{
	class RegistrationWidget : public QWidget{
		Q_OBJECT

		private:
			QLineEdit *loginInput;
			QLineEdit *passwordInput;
			QLineEdit *confirmPasswordInput;

			QPushButton *enterButton;
			QPushButton *loginButton;

		public:
			RegistrationWidget(QWidget* parent = nullptr);

		signals:
			void registrationSuccessful();
			void showLoginWindow();

		private slots:
			void onEnterButtonClicked();
			void onLoginButtonClicked();
	};
}
