#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include "clientmacros.hpp"
#include "loginwidget.hpp"

namespace SHIZ {
	LoginWidget::LoginWidget(Logger* logger, NetworkManager* manager, QWidget* parent)
		: logger(logger), networkManager(manager), QWidget(parent)
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
		connect(enterButton, &QPushButton::clicked, this, &LoginWidget::onEnterButtonClicked);

		registrationButton = new QPushButton("Register", this);
		layout->addWidget(registrationButton);
		connect(registrationButton, &QPushButton::clicked, this, &LoginWidget::onRegisterButtonClicked);

		disconnectButton = new QPushButton("Disconnect", this);
		layout->addWidget(disconnectButton);
		connect(disconnectButton, &QPushButton::clicked, this, &LoginWidget::onDisconnectButtonClicked);

		connect(networkManager, &NetworkManager::loginResult, this, &LoginWidget::onLoginResult);
	}


	void LoginWidget::onLoginResult(const QString& success, const QString& login) {
		if (success == USER) {
			emit loginUserSuccessful(login);
			loginInput->clear();
			passwordInput->clear();
			logger->log("Login user successful: " + login);
		}else if(success == ADMIN){
			loginInput->clear();
			passwordInput->clear();
			emit loginAdminSuccessful(login);
			logger->log("Login admin successful: " + login);
		} else {
			QMessageBox::warning(nullptr, "Login error", "Incorrect username or password.");
			logger->log("Login error: " + success);
		}
	}


	void LoginWidget::onDisconnectButtonClicked() {
		emit disconnectRequest();
		loginInput->clear();
		passwordInput->clear();

		emit showConnectionWindow();
	}

	void LoginWidget::onEnterButtonClicked() {
		QString login = loginInput->text();
		QString password = passwordInput->text();

		if (login.isEmpty() || password.isEmpty()) {
			QMessageBox::warning(this, "Login error", "Login and password fields cannot be empty.");
			return;
		}

		emit loginRequest(login, password);
	}

	void LoginWidget::onRegisterButtonClicked(){
		loginInput->clear();
		passwordInput->clear();

		emit showRegistrationWindow();
	}
}
