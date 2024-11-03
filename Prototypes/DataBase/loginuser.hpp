#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQueryModel>


#include "databasemanager.hpp"

class LoginUser : public QWidget {
    Q_OBJECT

private:
    DatabaseManager* db;
    QLineEdit *loginInput;
    QLineEdit *passwordInput;
    QPushButton *enterButton;
    QPushButton *registrationButton;
    QSqlQueryModel *model;


    void setupUI();
public:
    LoginUser(DatabaseManager* manager, QWidget* parent = nullptr);

signals:
    void loginSuccessful(const QString& login, std::string status);
    void showRegistrationWindow();

private slots:
    void onEnterButtonClicked();
    void onRegisterButtonClicked();
};
