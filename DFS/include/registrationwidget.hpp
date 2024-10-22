#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QWidget>

namespace SHIZ{
	class RegistrationWidget : public QWidget{
		Q_OBJECT

		private:
			QTcpSocket *tcpSocket;

			QLineEdit *loginInput;
			QLineEdit *passwordInput;
			QLineEdit *confirmPasswordInput;

			QPushButton *enterButton;
			QPushButton *loginButton;

		public:
			RegistrationWidget(QTcpSocket *socket, QWidget* parent = nullptr);

		signals:
			void registrationSuccessful();
			void showLoginWindow();

		private slots:
			void onEnterButtonClicked();
			void onLoginButtonClicked();
	};
}
