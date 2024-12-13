#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include <iostream>
#include <QDebug>

#include "adminwidget.hpp"
#include "clientmacros.hpp"

namespace SHIZ{
AdminWidget::AdminWidget(Logger* logger, NetworkManager* manager, QWidget* parent)
    : logger(logger), networkManager(manager), QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    statusLabel = new QLabel(this);
    layout->addWidget(statusLabel);
    connect(networkManager, &NetworkManager::statusMessage, this, &AdminWidget::onStatusMessageReceived);

    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText("Filter by username");
    layout->addWidget(filterLineEdit);
    connect(filterLineEdit, &QLineEdit::textChanged, this, &AdminWidget::onFilterTextChanged);

    userTableWidget = new QTableWidget(this);
    userTableWidget->setColumnCount(4);
    userTableWidget->setHorizontalHeaderLabels({"Username", "Is_admin", "Group", "Rights"});
    userTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTableWidget->setSortingEnabled(true);
    layout->addWidget(userTableWidget);

    refreshButton = new QPushButton("Refresh User List", this);
    layout->addWidget(refreshButton);

    deleteButton = new QPushButton("Delete user", this);
    layout->addWidget(deleteButton);

    logoutButton = new QPushButton("Logout", this);
    layout->addWidget(logoutButton);

    connect(refreshButton, &QPushButton::clicked, this, &AdminWidget::onRefreshButtonClicked);
    connect(deleteButton, &QPushButton::clicked, this, &AdminWidget::onDeleteButtonClicked);
    connect(logoutButton, &QPushButton::clicked, this, &AdminWidget::onLogoutButtonClicked);
    connect(userTableWidget, &QTableWidget::cellChanged, this, &AdminWidget::onCellChanged);
}


void AdminWidget::setCurrentLogin(const QString& login){
    currentLogin = login;
}


void AdminWidget::onDeleteButtonClicked() {
    int selectedRow = userTableWidget->currentRow();
    if (selectedRow >= 0) {
        QString userName = userTableWidget->item(selectedRow, 0)->text();
        QString isAdmin = userTableWidget->item(selectedRow, 1)->text();
        if(isAdmin != QString("true")){
            bool success = networkManager->deleteUser(userName);
            if (success) {
                QMessageBox::information(this, "Delete", "User deleted successfully.");
                onRefreshButtonClicked();
            } else {
                QMessageBox::warning(this, "Delete", "User deletion failed.");
            }
        }else{
            QMessageBox::warning(this, "Delete", "It is admin user.");
        }
    } else {
        QMessageBox::warning(this, "Delete", "No user selected.");
    }
}

void AdminWidget::onFilterTextChanged(const QString& text) {
    for (int i = 0; i < userTableWidget->rowCount(); ++i) {
        bool match = userTableWidget->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
        userTableWidget->setRowHidden(i, !match);
    }
}

void AdminWidget::onLogoutButtonClicked() {
    userTableWidget->setRowCount(0);

    emit showLoginWindow();
}

void AdminWidget::onRefreshButtonClicked(){
    QStringList users = networkManager->requestUserList();

    userTableWidget->setRowCount(users.size());
    userChange = false;
    for (int i = 0; i < users.size(); ++i) {
        QStringList userInfo = users[i].split("|");
        if (userInfo.size() == 4) {
            QTableWidgetItem* userNameItem = new QTableWidgetItem(userInfo[0]);
            QTableWidgetItem* ownerItem = new QTableWidgetItem(userInfo[1]);
            QTableWidgetItem* sizeItem = new QTableWidgetItem(userInfo[2]);
            QTableWidgetItem* dateItem = new QTableWidgetItem(userInfo[3]);

            userTableWidget->setItem(i, 0, userNameItem);
            userTableWidget->setItem(i, 1, ownerItem);
            userTableWidget->setItem(i, 2, sizeItem);
            userTableWidget->setItem(i, 3, dateItem);
        }
    }
    userChange = true;
}

void AdminWidget::onStatusMessageReceived(const QString& message) {
    statusLabel->setText(message);
}

void AdminWidget::onCellChanged(int row, int column) {
    QString userName = userTableWidget->item(row, 0)->text();
    QTableWidgetItem *item = userTableWidget->item(row, column);
    if (item && userChange) {
        QString value = item->text();
        bool success = false;

        if(column == 1){
            success = networkManager->updateUser(userName, QString(FIELD_USER_IS_ADMIN), value);
        }else if(column == 2){
            success = networkManager->updateUser(userName, QString(FIELD_USER_GROUP_ID), value);
        }else if(column == 3){
            success = networkManager->updateUser(userName, QString(FIELD_USER_RIGHTS), value);
        }

        if (success) {
            QMessageBox::information(this, "Update", "User update successfully.");
            onRefreshButtonClicked();
        } else {
            QMessageBox::warning(this, "Update", "User update failed.");
        }
    }
}
}
