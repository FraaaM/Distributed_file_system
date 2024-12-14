#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

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

		registrationButton = new QPushButton("Register", this);
		layout->addWidget(registrationButton);

		disconnectButton = new QPushButton("Disconnect", this);
		layout->addWidget(disconnectButton);


		connect(enterButton, &QPushButton::clicked, this, &LoginWidget::onEnterButtonClicked);
		connect(registrationButton, &QPushButton::clicked, this, &LoginWidget::onRegisterButtonClicked);
		connect(disconnectButton, &QPushButton::clicked, this, &LoginWidget::onDisconnectButtonClicked);
	}


	void LoginWidget::onDisconnectButtonClicked() {
		networkManager->disconnectFromHost();
		loginInput->clear();
		passwordInput->clear();

		emit showConnectionWindow();
	}

	void LoginWidget::onEnterButtonClicked(){
		QString login = loginInput->text();
		QString password = passwordInput->text();

		if (login.isEmpty() || password.isEmpty()) {
			QMessageBox::warning(this, "Login error", "Login and password fields cannot be empty.");
			return;
		}

        std::string success = networkManager->sendLoginRequest(login, password);
        if (success == "user") {
			loginInput->clear();
			passwordInput->clear();
            emit loginUserSuccessful(login);
            logger->log("Login user successful: " + login);
        }else if(success == "admin"){
            loginInput->clear();
            passwordInput->clear();
            emit loginAdminSuccessful(login);
            logger->log("Login admin successful: " + login);
        }
	}

	void LoginWidget::onRegisterButtonClicked(){
		loginInput->clear();
		passwordInput->clear();

		emit showRegistrationWindow();
	}
}
