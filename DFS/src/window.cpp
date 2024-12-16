#include <QVBoxLayout>
#include <QMessageBox>

#include "window.hpp"

namespace SHIZ {
	Window::Window(Logger* logger, NetworkManager* networkManager, QWidget* parent)
		: logger(logger), networkManager(networkManager), QWidget(parent)
	{
		stackedWidget = new QStackedWidget(this);
		this->resize(300, 450);

		this->setWindowIcon(QIcon(":images/DFS.png"));

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

		connect(adminWidget, &AdminWidget::deleteUserRequest, networkManager, &NetworkManager::onDeleteUserRequest);
		connect(adminWidget, &AdminWidget::updateUserRequest, networkManager, &NetworkManager::onUpdateUserRequest);
		connect(adminWidget, &AdminWidget::userListRequest, networkManager, &NetworkManager::onUserListRequest);
		connect(adminWidget, &AdminWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);

		connect(connectionWidget, &ConnectionWidget::connectRequest, networkManager, &NetworkManager::onConnectRequest);
		connect(connectionWidget, &ConnectionWidget::connectSuccessful, this, &Window::onConnectSuccessful);

		connect(loginWidget, &LoginWidget::disconnectRequest, networkManager, &NetworkManager::onDisconnectRequest);
		connect(loginWidget, &LoginWidget::loginRequest, networkManager, &NetworkManager::onLoginRequest);
		connect(loginWidget, &LoginWidget::loginAdminSuccessful, this, &Window::onAdminLoginSuccessful);
		connect(loginWidget, &LoginWidget::loginUserSuccessful, this, &Window::onUserLoginSuccessful);
		connect(loginWidget, &LoginWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);
		connect(loginWidget, &LoginWidget::showRegistrationWindow, this, &Window::onSwitchToRegistrationWindow);

		connect(mainWidget, &MainWidget::deleteFileRequest, networkManager, &NetworkManager::onDeleteFileRequest);
		connect(mainWidget, &MainWidget::downloadFileRequest, networkManager, &NetworkManager::onDownloadFileRequest);
		connect(mainWidget, &MainWidget::listFileRequest, networkManager, &NetworkManager::onListFileRequest);
		connect(mainWidget, &MainWidget::uploadFileRequest, networkManager, &NetworkManager::onUploadFileRequest);
		connect(mainWidget, &MainWidget::userInfoRequest, networkManager, &NetworkManager::onUserInfoRequest);
		connect(mainWidget, &MainWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
		connect(mainWidget, &MainWidget::userBanned, this, &Window::onSwitchToLoginWindowWithBanned);

		connect(registrationWidget, &RegistrationWidget::disconnectRequest, networkManager, &NetworkManager::onDisconnectRequest);
		connect(registrationWidget, &RegistrationWidget::registrationRequest, networkManager, &NetworkManager::onRegistrationRequest);
		connect(registrationWidget, &RegistrationWidget::registrationSuccessful, this, &Window::onRegistrationSuccessful);
		connect(registrationWidget, &RegistrationWidget::showConnectionWindow, this, &Window::onSwitchToConnectionWindow);
		connect(registrationWidget, &RegistrationWidget::showLoginWindow, this, &Window::onSwitchToLoginWindow);
	}


	void Window::onAdminLoginSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToAdminWindow();
	}

	void Window::onConnectSuccessful(const QString& host, quint16 port){
		logger->log("Current connection IP: " + host + " and port: " + port);
		onSwitchToLoginWindow();
	}

	void Window::onRegistrationSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
	}

	void Window::onSwitchToAdminWindow() {
		this->setWindowTitle("Admin");
		this->resize(600, 450);
		stackedWidget->setCurrentWidget(adminWidget);
		adminWidget->onRefreshButtonClicked();
	}

	void Window::onSwitchToConnectionWindow() {
		this->setWindowTitle("Connecting to server");
		this->resize(300, 450);
		stackedWidget->setCurrentWidget(connectionWidget);
	}

	void Window::onSwitchToLoginWindow() {
		this->setWindowTitle("Login");
		this->resize(300, 450);
		stackedWidget->setCurrentWidget(loginWidget);
	}

    void Window::onSwitchToLoginWindowWithBanned() {
        QMessageBox::warning(this, "Account", "Your account was deleted.");
        stackedWidget->setCurrentWidget(loginWidget);
	}

	void Window::onSwitchToMainWindow() {
		this->setWindowTitle("DFS");
		this->resize(600, 450);
		stackedWidget->setCurrentWidget(mainWidget);
		mainWidget->onRefreshButtonClicked();
	}

	void Window::onSwitchToRegistrationWindow() {
		this->setWindowTitle("Registration");
		this->resize(300, 450);
		stackedWidget->setCurrentWidget(registrationWidget);
	}

	void Window::onUserLoginSuccessful(const QString& login) {
		mainWidget->setCurrentLogin(login);
		onSwitchToMainWindow();
	}
}
