#include <QVBoxLayout>

#include "window.hpp"

namespace SHIZ{
	Window::Window(NetworkManager* networkManager, QWidget* parent)
		: networkManager(networkManager), QWidget(parent)
	{
		stackedWidget = new QStackedWidget(this);

		connectionWidget = new ConnectionWidget(networkManager, this);
		loginWidget = new LoginWidget(networkManager, this);
		mainWidget = new MainWidget(networkManager, this);
		registrationWidget = new RegistrationWidget(networkManager, this);

		stackedWidget->addWidget(connectionWidget);
		stackedWidget->addWidget(loginWidget);
		stackedWidget->addWidget(mainWidget);
		stackedWidget->addWidget(registrationWidget);

		stackedWidget->setCurrentWidget(connectionWidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(stackedWidget);
		setLayout(mainLayout);

		connect(connectionWidget, &ConnectionWidget::ConnectionSuccessful, this, &Window::onConnectionSuccessful);

		connect(loginWidget, &LoginWidget::loginSuccessful, this, &Window::onLoginSuccessful);
		connect(loginWidget, &LoginWidget::showRegistrationWindow, this, &Window::onSwitchToRegistrationWindow);

		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onRegistrationSuccessful);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);

		connect(mainWidget, &MainWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
	}

	void Window::onConnectionSuccessful(const QString& host){
		qDebug() << "Current connection address is " + host;
		onSwitchToLoginWindow();
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
		loginWidget->ClearData();
		stackedWidget->setCurrentWidget(loginWidget);
	}

	void Window::onSwitchToMainWindow() {
		stackedWidget->setCurrentWidget(mainWidget);
	}

	void Window::onSwitchToRegistrationWindow() {
		stackedWidget->setCurrentWidget(registrationWidget);
	}
}
