#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QSqlDatabase>


#include "databasemanager.hpp"

class RegisterUser  : public QWidget{
    Q_OBJECT

private:
    DatabaseManager* db;
    QLineEdit *loginInput;
    QLineEdit *passwordInput;
    QLineEdit *passwordRepeatInput;
    QPushButton *enterButton;
    QPushButton *loginButton;


    void setupUI();
    int registerNewUser();
public:
    RegisterUser(DatabaseManager *manager, QWidget *parent = nullptr);

signals:
    void registrationSuccessful(const QString& login);
    void showLoginWindow();

private slots:
    void onEnterButtonClicked();
    void onLoginButtonClicked();
};
