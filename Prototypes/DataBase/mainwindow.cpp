#include <QVBoxLayout>


#include "mainwindow.hpp"
#include "usersdatabase.hpp"
#include "filesdatabase.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QWidget{parent}
{
    stackedWidget = new QStackedWidget(this);

    DatabaseManager* usersDatabase = new DatabaseManager(this);
    loginWidget = new LoginUser(usersDatabase, this);
    mainWidget = new FileManager(usersDatabase, this);
    registrationWidget = new RegisterUser(usersDatabase, this);
    AdminInterfaceWidget = new AdminInterface(usersDatabase, this);


    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(mainWidget);
    stackedWidget->addWidget(registrationWidget);
    stackedWidget->addWidget(AdminInterfaceWidget);

    stackedWidget->setCurrentWidget(loginWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

    connect(loginWidget, &LoginUser::loginSuccessful, this, &MainWindow::onLoginSuccessful);
    connect(loginWidget, &LoginUser::showRegistrationWindow, this, &MainWindow::onSwitchToRegistrationWindow);

    connect(registrationWidget, &RegisterUser::registrationSuccessful, this, &MainWindow::onRegistrationSuccessful);
    connect(registrationWidget, &RegisterUser::showLoginWindow, this, &MainWindow::onSwitchToLoginWindow);


}

void MainWindow::onLoginSuccessful(const QString& login) {
    // mainWidget->setCurrentLogin(login);
    onSwitchToMainWindow();
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
}

void MainWindow::onSwitchToRegistrationWindow() {
    stackedWidget->setCurrentWidget(registrationWidget);
}
