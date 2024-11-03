#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <iostream>
#include <QSqlQueryModel>
#include <QCoreApplication>
#include <QDir>

#include "loginuser.hpp"
#include "database.hpp"


LoginUser::LoginUser(DatabaseManager* manager, QWidget *parent) : QWidget(parent), db(manager) {
    setupUI();
}


void LoginUser::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Login:", this);
    layout->addWidget(label);

    loginInput = new QLineEdit(this);
    layout->addWidget(loginInput);

    QLabel* passwordLabel = new QLabel("Password:", this);
    layout->addWidget(passwordLabel);

    model = new QSqlQueryModel(this);

    passwordInput = new QLineEdit(this);
    passwordInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordInput);

    enterButton = new QPushButton("Enter", this);
    layout->addWidget(enterButton);

    registrationButton = new QPushButton("Register", this);
    layout->addWidget(registrationButton);

    connect(enterButton, &QPushButton::clicked, this, &LoginUser::onEnterButtonClicked);
    connect(registrationButton, &QPushButton::clicked, this, &LoginUser::onRegisterButtonClicked);
}

void LoginUser::onEnterButtonClicked() {
    if (!loginInput->text().isEmpty() && !passwordInput->text().isEmpty()) {
        QString st = "SELECT " STATUS " FROM " USERS_TABLE " WHERE " LOGIN " = ? AND " PASSWORD " = ?";

        QVariantList values;
        values.append(QVariant(loginInput->text()));
        values.append(QVariant(passwordInput->text()));
        QSqlQuery query =db->execPreparedQuery(st, values);

        if (query.next()) {
            std::string status = query.value(0).toString().toStdString();
            emit loginSuccessful(loginInput->text(), status);
        } else {
            qDebug() << "Login failed";
        }
    }
}

void LoginUser::onRegisterButtonClicked() {
    loginInput->clear();
    passwordInput->clear();

    emit showRegistrationWindow();
}
