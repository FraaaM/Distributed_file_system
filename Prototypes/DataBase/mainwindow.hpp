#pragma once

#include <QStackedWidget>
#include <QWidget>

#include "loginuser.hpp"
#include "filemanager.hpp"
#include "registeruser.hpp"
#include "admininterface.hpp"
#include "database.hpp"
#include "usersdatabase.hpp"
#include "filesdatabase.hpp"


class MainWindow: public QWidget{
    Q_OBJECT

private:
    QStackedWidget *stackedWidget;

    // DatabaseManager filesDatabase;
    LoginUser *loginWidget;
    FileManager *mainWidget;
    RegisterUser *registrationWidget;
    AdminInterface *AdminInterfaceWidget;

public:
    MainWindow(QWidget* parent = nullptr);
    // ~MainWindow();
private slots:
    void onLoginSuccessful(const QString& login);
    void onRegistrationSuccessful(const QString& login);
    void onSwitchToLoginWindow();
    void onSwitchToMainWindow();
    void onSwitchToRegistrationWindow();
};

