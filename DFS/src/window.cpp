#include <QVBoxLayout>

#include "window.hpp"

namespace SHIZ{
	Window::Window(QWidget* parent): QWidget(parent){
		stackedWidget = new QStackedWidget(this);

		loginWidget = new LoginWidget(this);
		registrationWidget = new RegistrationWidget(this);
		chatWidget = new MainWidget(this);

		stackedWidget->addWidget(loginWidget);
		stackedWidget->addWidget(registrationWidget);
		stackedWidget->addWidget(chatWidget);

		stackedWidget->setCurrentWidget(loginWidget);

		QVBoxLayout *mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(stackedWidget);
		setLayout(mainLayout);


		connect(loginWidget, &LoginWidget::loginSuccessful, this, &Window::onSwitchToChatWindow);
		connect(loginWidget, &LoginWidget::showRegistrationWindow, this, &Window::onSwitchToRegistrationWindow);

		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onSwitchToChatWindow);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
	}


	void Window::onSwitchToChatWindow(){
		stackedWidget->setCurrentWidget(chatWidget);
	}

	void Window::onSwitchToLoginWindow(){
		stackedWidget->setCurrentWidget(loginWidget);
	}

	void Window::onSwitchToRegistrationWindow(){
		stackedWidget->setCurrentWidget(registrationWidget);
	}
}
