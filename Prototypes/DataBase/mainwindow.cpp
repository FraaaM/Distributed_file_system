#include <QVBoxLayout>


#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QWidget{parent}
{
    stackedWidget = new QStackedWidget(this);

    DatabaseManager* usersDatabase = new DatabaseManager(this);
    loginWidget = new LoginUser(usersDatabase, this);
    mainWidget = new FileManager(usersDatabase, this);
    registrationWidget = new RegisterUser(usersDatabase, this);
    adminInterfaceWidget = new AdminInterface(usersDatabase, this);


    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(mainWidget);
    stackedWidget->addWidget(registrationWidget);
    stackedWidget->addWidget(adminInterfaceWidget);

    stackedWidget->setCurrentWidget(loginWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

    connect(loginWidget, &LoginUser::loginSuccessful, this, &MainWindow::onLoginSuccessful);
    connect(loginWidget, &LoginUser::showRegistrationWindow, this, &MainWindow::onSwitchToRegistrationWindow);

    connect(registrationWidget, &RegisterUser::registrationSuccessful, this, &MainWindow::onRegistrationSuccessful);
    connect(registrationWidget, &RegisterUser::showLoginWindow, this, &MainWindow::onSwitchToLoginWindow);

}

void MainWindow::onLoginSuccessful(const QString& login, std::string status) {
    // mainWidget->setCurrentLogin(login);
    if(status == "user")
        onSwitchToMainWindow();
    else if(status == "admin"){
        onSwitchToAdminWindow();
    }
}

void MainWindow::onRegistrationSuccessful(const QString& login) {
    // mainWidget->setCurrentLogin(login);
    onSwitchToMainWindow();
}

void MainWindow::onSwitchToLoginWindow() {
    stackedWidget->setCurrentWidget(loginWidget);
}

void MainWindow::onSwitchToMainWindow() {
    stackedWidget->setCurrentWidget(mainWidget);
    resize(600, 600);
}
void MainWindow::onSwitchToAdminWindow() {
    stackedWidget->setCurrentWidget(adminInterfaceWidget);
    resize(600, 600);
}

void MainWindow::onSwitchToRegistrationWindow() {
    stackedWidget->setCurrentWidget(registrationWidget);
}
