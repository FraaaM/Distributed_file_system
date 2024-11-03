#pragma once

#include <QWidget>
#include <QSqlDatabase>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QSqlQueryModel>


#include "databasemanager.hpp"

class AdminInterface : public QWidget {
    Q_OBJECT

private:
    DatabaseManager* db;
    QPushButton* addGroupButton;
    QTableView* tableView;
    QSqlQueryModel* model;
    QLineEdit* findPanel;

public:
    AdminInterface(DatabaseManager *manager, QWidget *parent);

private slots:
    void addGroup();
    void findUsers();

private:
    void setupUI();
    void updateTable();
};
