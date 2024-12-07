#include <QVBoxLayout>

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

		stackedWidget->addWidget(connectionWidget);
		stackedWidget->addWidget(loginWidget);
		stackedWidget->addWidget(mainWidget);
        stackedWidget->addWidget(adminWidget);
		stackedWidget->addWidget(registrationWidget);

		stackedWidget->setCurrentWidget(connectionWidget);

		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(stackedWidget);
		setLayout(mainLayout);

		connect(connectionWidget, &ConnectionWidget::ConnectionSuccessful, this, &Window::onConnectionSuccessful);

        connect(loginWidget, &LoginWidget::loginUserSuccessful, this, &Window::onUserLoginSuccessful);
        connect(loginWidget, &LoginWidget::loginAdminSuccessful, this, &Window::onAdminLoginSuccessful);
		connect(loginWidget, &LoginWidget::showRegistrationWindow, this, &Window::onSwitchToRegistrationWindow);
		connect(loginWidget, &LoginWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);

		connect(mainWidget, &MainWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);

        connect(adminWidget, &AdminWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);

		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onRegistrationSuccessful);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
		connect(registrationWidget, &RegistrationWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);

         connect(this, &Window::switchOnMainWindow, mainWidget, &MainWidget::setRights);
	}


	void Window::onConnectionSuccessful(const QString& host, quint16 port){
		logger->log("Current connection IP: " + host + " and port: " + port);
		onSwitchToLoginWindow();
	}

    void Window::onUserLoginSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
    }
    void Window::onAdminLoginSuccessful(const QString& login) {
        mainWidget->setCurrentLogin(login);
        onSwitchToAdminWindow();
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
        emit switchOnMainWindow();
	}
    void Window::onSwitchToAdminWindow() {
        stackedWidget->setCurrentWidget(adminWidget);
    }

	void Window::onSwitchToRegistrationWindow() {
		stackedWidget->setCurrentWidget(registrationWidget);
	}
}
