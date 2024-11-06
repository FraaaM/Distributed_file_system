#pragma once

#include <QStackedWidget>
#include <QWidget>

#include "loginuser.hpp"
#include "filemanager.hpp"
#include "registeruser.hpp"
#include "admininterface.hpp"
#include "database.hpp"


class MainWindow: public QWidget{
    Q_OBJECT

private:
    QStackedWidget *stackedWidget;
    LoginUser *loginWidget;
    FileManager *mainWidget;
    RegisterUser *registrationWidget;
    AdminInterface *adminInterfaceWidget;

public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void onLoginSuccessful(const QString& login, std::string status);
    void onRegistrationSuccessful(const QString& login);
    void onSwitchToLoginWindow();
    void onSwitchToMainWindow();
    void onSwitchToAdminWindow();
    void onSwitchToRegistrationWindow();
};

