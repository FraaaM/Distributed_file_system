#include <QVBoxLayout>

#include "window.hpp"

namespace SHIZ{
	Window::Window(NetworkManager* networkManager, QWidget* parent)
		: networkManager(networkManager), QWidget(parent)
	{
		stackedWidget = new QStackedWidget(this);

		loginWidget = new LoginWidget(networkManager, this);
		mainWidget = new MainWidget(networkManager, this);
		registrationWidget = new RegistrationWidget(networkManager, this);

		stackedWidget->addWidget(loginWidget);
		stackedWidget->addWidget(mainWidget);
		stackedWidget->addWidget(registrationWidget);

		stackedWidget->setCurrentWidget(loginWidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(stackedWidget);
		setLayout(mainLayout);

		connect(loginWidget, &LoginWidget::loginSuccessful, this, &Window::onLoginSuccessful);
		connect(loginWidget, &LoginWidget::showRegistrationWindow, this, &Window::onSwitchToRegistrationWindow);

		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onRegistrationSuccessful);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
	}

	void Window::onLoginSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
	}

	void Window::onRegistrationSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
	}

	void Window::onSwitchToLoginWindow() {
		stackedWidget->setCurrentWidget(loginWidget);
	}

	void Window::onSwitchToMainWindow() {
		stackedWidget->setCurrentWidget(mainWidget);
	}

	void Window::onSwitchToRegistrationWindow() {
		stackedWidget->setCurrentWidget(registrationWidget);
	}
}
