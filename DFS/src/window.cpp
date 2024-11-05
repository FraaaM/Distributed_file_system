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
		connect(loginWidget, &LoginWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);

		connect(mainWidget, &MainWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);

		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onRegistrationSuccessful);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
		connect(registrationWidget, &RegistrationWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);
	}


	void Window::onConnectionSuccessful(const QString& host, quint16 port){
		qDebug() << "Current connection IP: " << host << " and port: " << port;
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

	void Window::onSwitchToConnectionWindow() {
		stackedWidget->setCurrentWidget(connectionWidget);
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
