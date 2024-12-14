#include <QVBoxLayout>
#include <QMessageBox>

#include "window.hpp"

namespace SHIZ{
	Window::Window(Logger* logger, NetworkManager* networkManager, QWidget* parent)
		: logger(logger), networkManager(networkManager), QWidget(parent)
	{
		stackedWidget = new QStackedWidget(this);

		connectionWidget = new ConnectionWidget(logger, networkManager, this);
		loginWidget = new LoginWidget(logger, networkManager, this);
		mainWidget = new MainWidget(logger, networkManager, this);
        adminWidget = new AdminWidget(logger, networkManager, this);
		registrationWidget = new RegistrationWidget(logger, networkManager, this);

		stackedWidget->addWidget(adminWidget);
		stackedWidget->addWidget(connectionWidget);
		stackedWidget->addWidget(loginWidget);
		stackedWidget->addWidget(mainWidget);
		stackedWidget->addWidget(registrationWidget);

		stackedWidget->setCurrentWidget(connectionWidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(stackedWidget);
		setLayout(mainLayout);

		connect(connectionWidget, &ConnectionWidget::ConnectionSuccessful, this, &Window::onConnectionSuccessful);

		connect(loginWidget, &LoginWidget::loginAdminSuccessful, this, &Window::onAdminLoginSuccessful);
		connect(loginWidget, &LoginWidget::loginUserSuccessful, this, &Window::onUserLoginSuccessful);
		connect(loginWidget, &LoginWidget::showRegistrationWindow, this, &Window::onSwitchToRegistrationWindow);
		connect(loginWidget, &LoginWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);

		connect(adminWidget, &AdminWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);

		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onRegistrationSuccessful);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
		connect(registrationWidget, &RegistrationWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);

		connect(mainWidget, &MainWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
        connect(mainWidget, &MainWidget::userBanned, this, &Window::onSwitchToLoginWindowWithBanned);
	}


	void Window::onAdminLoginSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToAdminWindow();
	}

	void Window::onConnectionSuccessful(const QString& host, quint16 port){
		logger->log("Current connection IP: " + host + " and port: " + port);
		onSwitchToLoginWindow();
	}

	void Window::onRegistrationSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
	}

	void Window::onSwitchToAdminWindow() {
		stackedWidget->setCurrentWidget(adminWidget);
	}

	void Window::onSwitchToConnectionWindow() {
		stackedWidget->setCurrentWidget(connectionWidget);
	}

	void Window::onSwitchToLoginWindow() {
		stackedWidget->setCurrentWidget(loginWidget);
	}

    void Window::onSwitchToLoginWindowWithBanned() {
        QMessageBox::warning(this, "Account", "Your account was deleted.");
        stackedWidget->setCurrentWidget(loginWidget);
	}

	void Window::onSwitchToMainWindow() {
		stackedWidget->setCurrentWidget(mainWidget);
	}

	void Window::onSwitchToRegistrationWindow() {
		stackedWidget->setCurrentWidget(registrationWidget);
	}

	void Window::onUserLoginSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
	}
}
