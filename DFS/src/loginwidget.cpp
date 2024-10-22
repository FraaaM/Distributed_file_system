#include <QLabel>
#include <QVBoxLayout>

#include "loginwidget.hpp"

namespace SHIZ{
	LoginWidget::LoginWidget(QWidget* parent): QWidget(parent){
		QVBoxLayout *layout = new QVBoxLayout(this);

		QLabel *label = new QLabel("Login:", this);
		layout->addWidget(label);

		loginInput = new QLineEdit(this);
		layout->addWidget(loginInput);


		QLabel *passwordLabel = new QLabel("Password:", this);
		layout->addWidget(passwordLabel);

		passwordInput = new QLineEdit(this);
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
			emit loginSuccessful();
		}
	}

	void LoginWidget::onRegisterButtonClicked(){
		loginInput->clear();
		passwordInput->clear();

		emit showRegistrationWindow();
	}
}
