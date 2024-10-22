#include <QLabel>
#include <QVBoxLayout>

#include "loginwidget.hpp"

namespace SHIZ{
	LoginWidget::LoginWidget(QTcpSocket *socket, QWidget* parent):
		tcpSocket(socket), QWidget(parent)
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		QLabel *label = new QLabel("Login:", this);
		layout->addWidget(label);

		loginInput = new QLineEdit(this);
		layout->addWidget(loginInput);


		QLabel *passwordLabel = new QLabel("Password:", this);
		layout->addWidget(passwordLabel);

		passwordInput = new QLineEdit(this);
		passwordInput->setEchoMode(QLineEdit::Password);
		layout->addWidget(passwordInput);


		enterButton = new QPushButton("Enter", this);
		layout->addWidget(enterButton);

		registrationButton = new QPushButton("Register", this);
		layout->addWidget(registrationButton);


		connect(enterButton, &QPushButton::clicked, this, &LoginWidget::onEnterButtonClicked);
		connect(registrationButton, &QPushButton::clicked, this, &LoginWidget::onRegisterButtonClicked);
	}


	void LoginWidget::onEnterButtonClicked(){
		if (!loginInput->text().isEmpty()){
			tcpSocket->connectToHost("127.0.0.1", 1234);

			if(tcpSocket->waitForConnected(3000)) {
				QString loginData = "LOGIN:" + loginInput->text() + ":" + passwordInput->text();
				tcpSocket->write(loginData.toUtf8());
				tcpSocket->flush();

				if(tcpSocket->waitForReadyRead(3000)){
					QString resonse = tcpSocket->readAll();
					if (resonse == "SUCCESS") {
						emit loginSuccessful();
					}
					else {
						qDebug() << "Login failed";
					}
				}
			}
			else {
				qDebug() << "Unable to connect to the server!";
			}
		}
	}

	void LoginWidget::onRegisterButtonClicked(){
		loginInput->clear();
		passwordInput->clear();

		emit showRegistrationWindow();
	}
}
