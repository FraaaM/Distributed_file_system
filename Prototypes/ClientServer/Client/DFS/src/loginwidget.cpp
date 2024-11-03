#include <QLabel>
#include <QVBoxLayout>

#include "loginwidget.hpp"

namespace SHIZ {
	LoginWidget::LoginWidget(NetworkManager* manager, QWidget* parent)
		: networkManager(manager), QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);


		QLabel* label = new QLabel("Login:", this);
		layout->addWidget(label);

		loginInput = new QLineEdit(this);
		layout->addWidget(loginInput);

		QLabel* passwordLabel = new QLabel("Password:", this);
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
		if (!loginInput->text().isEmpty()) {
			bool success = networkManager->sendLoginRequest(loginInput->text(), passwordInput->text());

			if (success) {
				emit loginSuccessful(loginInput->text());
			} else {
				qDebug() << "Login failed";
			}
		}
	}

	void LoginWidget::onRegisterButtonClicked(){
		loginInput->clear();
		passwordInput->clear();

		emit showRegistrationWindow();
	}

    void LoginWidget::ClearData() {
        loginInput->clear();
        passwordInput->clear();
    }
}
