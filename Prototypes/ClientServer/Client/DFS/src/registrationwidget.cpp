#include <QLabel>
#include <QVBoxLayout>

#include "registrationwidget.hpp"

namespace SHIZ{
	RegistrationWidget::RegistrationWidget(NetworkManager* manager, QWidget* parent)
		: networkManager(manager), QWidget(parent) {

		QVBoxLayout* layout = new QVBoxLayout(this);


		QLabel* loginLabel = new QLabel("Login:", this);
		layout->addWidget(loginLabel);

		loginInput = new QLineEdit(this);
		layout->addWidget(loginInput);


		QLabel* passwordLabel = new QLabel("Password:", this);
		layout->addWidget(passwordLabel);

		passwordInput = new QLineEdit(this);
		passwordInput->setEchoMode(QLineEdit::Password);
		layout->addWidget(passwordInput);


		QLabel* confirmPasswordLabel = new QLabel("Confirm:", this);
		layout->addWidget(confirmPasswordLabel);

		confirmPasswordInput = new QLineEdit(this);
		confirmPasswordInput->setEchoMode(QLineEdit::Password);
		layout->addWidget(confirmPasswordInput);


		enterButton = new QPushButton("Enter", this);
		layout->addWidget(enterButton);

		loginButton = new QPushButton("Login", this);
		layout->addWidget(loginButton);


		connect(enterButton, &QPushButton::clicked, this, &RegistrationWidget::onEnterButtonClicked);
		connect(loginButton, &QPushButton::clicked, this, &RegistrationWidget::onLoginButtonClicked);
	}


	void RegistrationWidget::onEnterButtonClicked() {
		if (!(loginInput->text().isEmpty() || passwordInput->text().isEmpty())
			&& passwordInput->text() == confirmPasswordInput->text()) {

			bool success = networkManager->sendRegistrationRequest(loginInput->text(), passwordInput->text());

			if (success) {
				emit registrationSuccessful(loginInput->text());
			} else {
				qDebug() << "Registration failed";
			}
		}
	}

	void RegistrationWidget::onLoginButtonClicked() {
		loginInput->clear();
		passwordInput->clear();
		confirmPasswordInput->clear();

		emit showLoginWindow();
	}
}
