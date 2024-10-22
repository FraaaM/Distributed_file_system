#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QTcpSocket>
#include <QWidget>

namespace SHIZ{
	class LoginWidget  : public QWidget{
		Q_OBJECT

		private:
			QTcpSocket *tcpSocket;

			QLineEdit *loginInput;
			QLineEdit *passwordInput;

			QPushButton *enterButton;
			QPushButton *registrationButton;

		public:
			LoginWidget(QTcpSocket *socket, QWidget* parent = nullptr);

		signals:
			void loginSuccessful();
			void showRegistrationWindow();

		private slots:
			void onEnterButtonClicked();
			void onRegisterButtonClicked();
	};
}
