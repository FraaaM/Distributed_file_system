#include <QLabel>
#include <QVBoxLayout>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QSqlQuery>
#include <QCoreApplication>
#include <QVariantList>


#include "registeruser.hpp"
#include "database.hpp"


RegisterUser::RegisterUser(DatabaseManager *manager, QWidget *parent) : QWidget(parent), db(manager) {
    setupUI();
}

void RegisterUser::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel("Login:", this);
    layout->addWidget(label);

    loginInput = new QLineEdit(this);
    layout->addWidget(loginInput);

    QLabel* passwordLabel = new QLabel("Password:", this);
    layout->addWidget(passwordLabel);


    passwordInput = new QLineEdit(this);
    passwordInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordInput);


    QLabel* passwordRepeatLabel = new QLabel("Repeat password:", this);
    layout->addWidget(passwordRepeatLabel);


    passwordRepeatInput = new QLineEdit(this);
    passwordRepeatInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordRepeatInput);

    enterButton = new QPushButton("Enter", this);
    layout->addWidget(enterButton);

    loginButton = new QPushButton("Login", this);
    layout->addWidget(loginButton);


    connect(enterButton, &QPushButton::clicked, this, &RegisterUser::onEnterButtonClicked);
    connect(loginButton, &QPushButton::clicked, this, &RegisterUser::onLoginButtonClicked);
}

int RegisterUser::registerNewUser(){
    QString login = loginInput->text();
    QString password = passwordInput->text();
    int team = 1;
    const char* status = "user";
    const char* right = "r";


    QString request = "INSERT INTO " USERS_TABLE " (" LOGIN " , " PASSWORD " , " TEAM " , " STATUS " , " RIGHT ") "
                      "VALUES (?, ?, ?, ?, ?)";

    QVariantList values;
    values.append(QVariant(login));
    values.append(QVariant(password));
    values.append(QVariant(team));
    values.append(QVariant(status));
    values.append(QVariant(right));
    QSqlQuery query = db->execPreparedQuery(request, values);

    return 0;
}
void RegisterUser::onEnterButtonClicked(){
    if(passwordInput->text() != passwordRepeatInput->text()){
        QMessageBox::warning(this, "Warning" , "Passwords are not equal. Please, try to input passwords again.");
        return;
    }
    if (!loginInput->text().isEmpty()) {

        QString login = loginInput->text();
        QString password = passwordInput->text();

        qDebug() << login;
        qDebug() << password;

        if(this->registerNewUser() == 0)
            emit registrationSuccessful(loginInput->text());
        else
            qDebug() << "Registr failed";
    }else{
        QMessageBox::warning(this, "Warning" , "Login is empty.");
    }
}
void RegisterUser::onLoginButtonClicked(){
    loginInput->clear();
    passwordInput->clear();
    passwordRepeatInput->clear();

    emit showLoginWindow();
}
